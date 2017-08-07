using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using Microsoft.Win32.SafeHandles;

namespace TL866
{
    public  class UsbDevice
    {
        public delegate void UsbDeviceChangedEventHandler();


        [Flags]
        public enum DiGetClassFlags
        {
            DIGCF_DEFAULT = 0x1,
            DIGCF_PRESENT = 0x2,
            DIGCF_ALLCLASSES = 0x4,
            DIGCF_PROFILE = 0x8,
            DIGCF_DEVICEINTERFACE = 0x10
        }

        private const int INVALID_HANDLE_VALUE = -1;
        private const int WM_DEVICECHANGE = 0x219;
        private const string MINIPRO_GUID = "{85980D83-32B9-4ba1-8FDF-12A711B99CA2}";
        private readonly object SyncObject = new object();
        private IntPtr deviceEventHandle;

        private SafeFileHandle hDrv;


        public uint DevicesCount
        {
            get { return GetDevicesByClass(MINIPRO_GUID, null); }
        }

        public event UsbDeviceChangedEventHandler UsbDeviceChanged;


        [DllImport("setupapi.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr SetupDiGetClassDevs(ref Guid ClassGuid, int Enumerator, IntPtr hwndParent,
            uint Flags);

        [DllImport("setupapi.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern bool SetupDiEnumDeviceInterfaces(IntPtr hDevInfo, IntPtr devInfo,
            ref Guid interfaceClassGuid, uint memberIndex, ref SP_DEVICE_INTERFACE_DATA deviceInterfaceData);

        [DllImport("setupapi.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern bool SetupDiGetDeviceInterfaceDetail(IntPtr nSetupDiGetClassDevs,
            ref SP_DEVICE_INTERFACE_DATA DeviceInterfaceData, IntPtr Ptr, uint DeviceInterfaceDetailDataSize,
            ref int RequiredSize, IntPtr PtrInfo);

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        private static extern SafeFileHandle CreateFile(string lpFileName, EFileAccess dwDesiredAccess,
            EFileShare dwShareMode, IntPtr lpSecurityAttributes, ECreationDisposition dwCreationDisposition,
            EFileAttributes dwFlagsAndAttributes, IntPtr hTemplateFile);

        [DllImport("kernel32.dll", ExactSpelling = true, SetLastError = true, CharSet = CharSet.Auto)]
        private static extern bool DeviceIoControl(IntPtr hDevice, uint dwIoControlCode, IntPtr lpInBuffer,
            uint nInBufferSize, IntPtr lpOutBuffer, uint nOutBufferSize, ref uint lpBytesReturned, IntPtr lpOverlapped);

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool CloseHandle(IntPtr hObject);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern IntPtr
            RegisterDeviceNotification(IntPtr hRecipient, IntPtr NotificationFilter, int Flags);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool UnregisterDeviceNotification(IntPtr hHandle);


        public bool OpenDevice(string DevicePath)
        {
            lock (SyncObject)
            {
                hDrv = CreateFile(DevicePath, EFileAccess.GENERIC_READ | EFileAccess.GENERIC_WRITE,
                    EFileShare.FILE_SHARE_READ | EFileShare.FILE_SHARE_WRITE, IntPtr.Zero,
                    ECreationDisposition.OPEN_EXISTING, EFileAttributes.FILE_ATTRIBUTE_NORMAL, IntPtr.Zero);
            }
            return !(hDrv.IsClosed || hDrv.IsInvalid);
        }

        public void CloseDevice()
        {
            if (hDrv != null)
                lock (SyncObject)
                {
                    hDrv.Close();
                }
        }


        public bool Write(byte[] buffer)
        {
            if (hDrv == null)
                return false;
            uint btr = 0;
            byte[] obuff = new byte[4096];
            bool r;
            lock (SyncObject)
            {
                if (hDrv.IsClosed || hDrv.IsInvalid)
                    return false;
                r = DeviceIoControl(hDrv.DangerousGetHandle(), (uint) IOCTL.IOCTL_WRITE,
                    Marshal.UnsafeAddrOfPinnedArrayElement(buffer, 0), (uint) buffer.Length,
                    Marshal.UnsafeAddrOfPinnedArrayElement(obuff, 0), (uint) obuff.Length, ref btr, IntPtr.Zero);
            }
            return r;
        }

        public uint Read(byte[] buffer)
        {
            if (hDrv == null)
                return 0;
            uint btr = 0;
            byte[] o = new byte[4];
            lock (SyncObject)
            {
                if (hDrv.IsClosed || hDrv.IsInvalid)
                    return 0;
                DeviceIoControl(hDrv.DangerousGetHandle(), (uint) IOCTL.IOCTL_READ,
                    Marshal.UnsafeAddrOfPinnedArrayElement(o, 0), (uint) o.Length,
                    Marshal.UnsafeAddrOfPinnedArrayElement(buffer, 0), (uint) buffer.Length, ref btr, IntPtr.Zero);
            }
            return btr;
        }


        public List<string> Get_Devices()
        {
            List<string> DevicesPathName = new List<string>();
            GetDevicesByClass(MINIPRO_GUID, DevicesPathName);
            return DevicesPathName;
        }

        public bool RegisterForDeviceChange(bool Register, Form f)
        {
            bool Status = false;

            if (Register)
            {
                DEV_BROADCAST_DEVICEINTERFACE deviceInterface = new DEV_BROADCAST_DEVICEINTERFACE();
                int size = Marshal.SizeOf(deviceInterface);
                deviceInterface.dbcc_size = size;
                deviceInterface.dbcc_devicetype = (int) DBTDEVTYP.DBT_DEVTYP_DEVICEINTERFACE;
                IntPtr buffer = Marshal.AllocHGlobal(size);
                Marshal.StructureToPtr(deviceInterface, buffer, true);
                deviceEventHandle = RegisterDeviceNotification(f.Handle, buffer,
                    Convert.ToInt32(DEVICE_NOTIFY.DEVICE_NOTIFY_WINDOW_HANDLE |
                                    DEVICE_NOTIFY.DEVICE_NOTIFY_ALL_INTERFACE_CLASSES));
                Status = deviceEventHandle != IntPtr.Zero;
                Marshal.FreeHGlobal(buffer);
            }
            else
            {
                if (deviceEventHandle != IntPtr.Zero) Status = UnregisterDeviceNotification(deviceEventHandle);
                deviceEventHandle = IntPtr.Zero;
            }

            return Status;
        }


        public void ProcessWindowsMessage(ref Message m)
        {
            int devType;

            if (m.Msg == WM_DEVICECHANGE)
                switch (m.WParam.ToInt32())
                {
                    case (int) DBTDEVICE.DBT_DEVICEARRIVAL:
                        // New device has just arrived
                        devType = Marshal.ReadInt32(m.LParam, 4);
                        if (devType == (int) DBTDEVTYP.DBT_DEVTYP_DEVICEINTERFACE)
                            if (UsbDeviceChanged != null) UsbDeviceChanged();
                        break;
                    case (int) DBTDEVICE.DBT_DEVICEQUERYREMOVE:
                        // Device is about to be removed, any application can cancel the removal
                        devType = Marshal.ReadInt32(m.LParam, 4);
                        if (devType == (int) DBTDEVTYP.DBT_DEVTYP_DEVICEINTERFACE)
                            if (UsbDeviceChanged != null) UsbDeviceChanged();
                        break;

                    case (int) DBTDEVICE.DBT_DEVICEREMOVECOMPLETE:
                        // Device has been removed
                        devType = Marshal.ReadInt32(m.LParam, 4);
                        if (devType == (int) DBTDEVTYP.DBT_DEVTYP_DEVICEINTERFACE)
                            if (UsbDeviceChanged != null) UsbDeviceChanged();
                        break;
                }
        }


        public void UsbDeviceChange()
        {
            UsbDeviceChanged();
        }

        private uint GetDevicesByClass(string guid, List<string> devicePathName)
        {
            Guid g = new Guid(guid);
            uint count = 0;
            IntPtr handle = SetupDiGetClassDevs(ref g, 0, IntPtr.Zero,
                (int) (DiGetClassFlags.DIGCF_PRESENT | DiGetClassFlags.DIGCF_DEVICEINTERFACE));
            lock (SyncObject)
            {
                if (handle.ToInt32() != INVALID_HANDLE_VALUE)
                {
                    bool Success = true;
                    while (Success)
                    {
                        SP_DEVICE_INTERFACE_DATA dia = new SP_DEVICE_INTERFACE_DATA();
                        dia.cbSize = Marshal.SizeOf(dia);
                        Success = SetupDiEnumDeviceInterfaces(handle, IntPtr.Zero, ref g, count, ref dia);
                        if (Success)
                        {
                            int bufferSize = 0;
                            SP_DEVINFO_DATA da = new SP_DEVINFO_DATA();
                            da.cbSize = Marshal.SizeOf(da);
                            SetupDiGetDeviceInterfaceDetail(handle, ref dia, IntPtr.Zero, 0, ref bufferSize,
                                IntPtr.Zero);
                            IntPtr detailDataBuffer = Marshal.AllocHGlobal(bufferSize);
                            Marshal.WriteInt32(detailDataBuffer,
                                IntPtr.Size == 4 ? 4 + Marshal.SystemDefaultCharSize : 8);
                            int nBytes = bufferSize;
                            Success = SetupDiGetDeviceInterfaceDetail(handle, ref dia, detailDataBuffer, (uint) nBytes,
                                ref bufferSize, IntPtr.Zero);
                            if (Success)
                            {
                                IntPtr pDevicePathName = new IntPtr(detailDataBuffer.ToInt32() + 4);
                                if (devicePathName != null)
                                    devicePathName.Add(Marshal.PtrToStringAuto(pDevicePathName));
                                Marshal.FreeHGlobal(detailDataBuffer);
                            }
                        }
                        count++;
                    }
                }
            }
            return count - 1;
        }

        private enum IOCTL
        {
            IOCTL_READ = 0x222004,
            IOCTL_WRITE = 0x222000
        }


        private enum EFileAccess : uint
        {
            DELETE = 0x10000,
            READ_CONTROL = 0x20000,
            WRITE_DAC = 0x40000,
            WRITE_OWNER = 0x80000,
            SYNCHRONIZE = 0x100000,
            STANDARD_RIGHTS_REQUIRED = 0xf0000,
            STANDARD_RIGHTS_READ = READ_CONTROL,
            STANDARD_RIGHTS_WRITE = READ_CONTROL,
            STANDARD_RIGHTS_EXECUTE = READ_CONTROL,
            STANDARD_RIGHTS_ALL = 0x1f0000,
            SPECIFIC_RIGHTS_ALL = 0xffff,
            ACCESS_SYSTEM_SECURITY = 0x1000000,
            MAXIMUM_ALLOWED = 0x2000000,
            GENERIC_READ = 0x80000000,
            GENERIC_WRITE = 0x40000000,
            GENERIC_EXECUTE = 0x20000000,
            GENERIC_ALL = 0x10000000
        }

        private enum EFileShare
        {
            FILE_SHARE_NONE = 0x0,
            FILE_SHARE_READ = 0x1,
            FILE_SHARE_WRITE = 0x2,
            FILE_SHARE_DELETE = 0x4
        }

        private enum ECreationDisposition
        {
            CREATE_ALWAYS = 2,
            OPEN_EXISTING = 3,
            OPEN_ALWAYS = 4,
            TRUNCATE_EXISTING = 5
        }


        private enum EFileAttributes : uint
        {
            FILE_ATTRIBUTE_READONLY = 0x1,
            FILE_ATTRIBUTE_HIDDEN = 0x2,
            FILE_ATTRIBUTE_SYSTEM = 0x4,
            FILE_ATTRIBUTE_DIRECTORY = 0x10,
            FILE_ATTRIBUTE_ARCHIVE = 0x20,
            FILE_ATTRIBUTE_DEVICE = 0x40,
            FILE_ATTRIBUTE_NORMAL = 0x80,
            FILE_ATTRIBUTE_TEMPORARY = 0x100,
            FILE_ATTRIBUTE_SPARSE_FILE = 0x200,
            FILE_ATTRIBUTE_REPARSE_POINT = 0x400,
            FILE_ATTRIBUTE_COMPRESSED = 0x800,
            FILE_ATTRIBUTE_OFFLINE = 0x1000,
            FILE_ATTRIBUTE_NOT_CONTENT_INDEXED = 0x2000,
            FILE_ATTRIBUTE_ENCRYPTED = 0x4000,
            FILE_ATTRIBUTE_VIRTUAL = 0x10000,
            FILE_FLAG_BACKUP_SEMANTICS = 0x2000000,
            FILE_FLAG_DELETE_ON_CLOSE = 0x4000000,
            FILE_FLAG_NO_BUFFERING = 0x20000000,
            FILE_FLAG_OPEN_NO_RECALL = 0x100000,
            FILE_FLAG_OPEN_REPARSE_POINT = 0x200000,
            FILE_FLAG_OVERLAPPED = 0x40000000,
            FILE_FLAG_POSIX_SEMANTICS = 0x1000000,
            FILE_FLAG_RANDOM_ACCESS = 0x10000000,
            FILE_FLAG_SEQUENTIAL_SCAN = 0x8000000,
            FILE_FLAG_WRITE_THROUGH = 0x80000000
        }

        private enum DEVICE_NOTIFY
        {
            DEVICE_NOTIFY_WINDOW_HANDLE = 0x0,
            DEVICE_NOTIFY_SERVICE_HANDLE = 0x1,
            DEVICE_NOTIFY_ALL_INTERFACE_CLASSES = 0x4
        }

        private enum DBTDEVICE
        {
            DBT_DEVICEARRIVAL = 0x8000,
            DBT_DEVICEQUERYREMOVE = 0x8001,
            DBT_DEVICEQUERYREMOVEFAILED = 0x8002,
            DBT_DEVICEREMOVEPENDING = 0x8003,
            DBT_DEVICEREMOVECOMPLETE = 0x8004,
            DBT_DEVICETYPESPECIFIC = 0x8005,
            DBT_CUSTOMEVENT = 0x8006
        }

        private enum DBTDEVTYP
        {
            DBT_DEVTYP_OEM = 0x0,
            DBT_DEVTYP_DEVNODE = 0x1,
            DBT_DEVTYP_VOLUME = 0x2,
            DBT_DEVTYP_PORT = 0x3,
            DBT_DEVTYP_NET = 0x4,
            DBT_DEVTYP_DEVICEINTERFACE = 0x5,
            DBT_DEVTYP_HANDLE = 0x6
        }


        [StructLayout(LayoutKind.Sequential)]
        public struct SP_DEVINFO_DATA
        {
            public int cbSize;
            public Guid ClassGuid;
            public uint DevInst;
            public IntPtr Reserved;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct SP_DEVICE_INTERFACE_DATA
        {
            public int cbSize;
            public Guid InterfaceClassGuid;
            public uint Flags;
            public IntPtr Reserved;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct SP_DEVICE_INTERFACE_DETAIL_DATA
        {
            public int cbSize;
            public byte[] DevicePath;
        }

        public struct DEV_BROADCAST_DEVICEINTERFACE
        {
            public int dbcc_size;
            public int dbcc_devicetype;
            public int dbcc_reserved;

            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 16)]
            public byte[] dbcc_classguid;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)] public char[] dbcc_name;
        }
    }
}