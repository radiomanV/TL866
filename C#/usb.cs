using System;
using System.Collections;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;
using System.Threading;
namespace TL866
{

	public class UsbDevice
	{
		private const int DIGCF_PRESENT = 0x2;
		private const int DIGCF_DEVICEINTERFACE = 0x10;
		private const int INVALID_HANDLE_VALUE = -1;
		private const UInt64 GENERIC_READ = 0x80000000;
		private const UInt64 GENERIC_WRITE = 0x40000000;
		private const UInt64 GENERIC_EXECUTE = 0x20000000;
		private const UInt64 GENERIC_ALL = 0x10000000;
		private const uint FILE_SHARE_READ = 0x1;
		private const uint FILE_SHARE_WRITE = 0x2;
		private const uint OPEN_EXISTING = 0x3;
		private const uint FILE_FLAG_DELETE_ON_CLOSE = 0x4000000;
        public const int WM_DEVICECHANGE = 0x219;
		private const string MINIPRO_GUID = "{85980D83-32B9-4ba1-8FDF-12A711B99CA2}";
        
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

		[DllImport("Kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
		private static extern uint GetLastError();

		[DllImport("setupapi.dll", CharSet = CharSet.Auto, SetLastError = true)]
		private static extern IntPtr SetupDiGetClassDevs(ref Guid ClassGuid, int Enumerator, IntPtr hwndParent, UInt32 Flags);

		[DllImport("setupapi.dll", CharSet = CharSet.Auto, SetLastError = true)]
		private static extern Boolean SetupDiEnumDeviceInterfaces(IntPtr hDevInfo, IntPtr devInfo, ref Guid interfaceClassGuid, UInt32 memberIndex, ref SP_DEVICE_INTERFACE_DATA deviceInterfaceData);

		[DllImport("setupapi.dll", CharSet = CharSet.Auto, SetLastError = true)]
		private static extern Boolean SetupDiGetDeviceInterfaceDetail(IntPtr nSetupDiGetClassDevs, ref SP_DEVICE_INTERFACE_DATA DeviceInterfaceData, ref SP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData, uint DeviceInterfaceDetailDataSize, ref int RequiredSize, ref SP_DEVINFO_DATA DeviceInfoData);

		[DllImport("setupapi.dll", CharSet = CharSet.Auto, SetLastError = true)]
		private static extern Boolean SetupDiGetDeviceInterfaceDetail(IntPtr nSetupDiGetClassDevs, ref SP_DEVICE_INTERFACE_DATA DeviceInterfaceData, IntPtr Ptr, uint DeviceInterfaceDetailDataSize, ref int RequiredSize, IntPtr PtrInfo);

		[DllImport("setupapi.dll", CharSet = CharSet.Auto, SetLastError = true)]
		private static extern uint CM_Get_Parent(ref UInt32 pdnDevInst, uint dnDevInst, ulong ulFlags);

		[DllImport("setupapi.dll", CharSet = CharSet.Auto)]
		private static extern int CM_Get_Device_ID(UInt32 dnDevInst, IntPtr Buffer, int BufferLen, int ulFlags);

		[DllImport("setupapi.dll", CharSet = CharSet.Auto, SetLastError = true)]
		private static extern bool SetupDiDestroyDeviceInfoList(IntPtr hDevInfo);

		[DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
		private static extern Microsoft.Win32.SafeHandles.SafeFileHandle CreateFile(string lpFileName, EFileAccess dwDesiredAccess, EFileShare dwShareMode, IntPtr lpSecurityAttributes, ECreationDisposition dwCreationDisposition, EFileAttributes dwFlagsAndAttributes, IntPtr hTemplateFile);

		[DllImport("kernel32.dll", ExactSpelling = true, SetLastError = true, CharSet = CharSet.Auto)]
		private static extern bool DeviceIoControl(IntPtr hDevice, uint dwIoControlCode, IntPtr lpInBuffer, uint nInBufferSize, IntPtr lpOutBuffer, uint nOutBufferSize, ref uint lpBytesReturned, IntPtr lpOverlapped);

		[DllImport("Kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
		private static extern bool DeviceIoControl(IntPtr hDevice, uint dwIoControlCode, ref long InBuffer, int nInBufferSize, ref long OutBuffer, int nOutBufferSize, ref int pBytesReturned, [In()]ref NativeOverlapped lpOverlapped);

		[DllImport("kernel32.dll", SetLastError = true)]
		[return: MarshalAs(UnmanagedType.Bool)]
		private static extern bool CloseHandle(IntPtr hObject);

		[DllImport("kernel32.dll")]
		[return: MarshalAs(UnmanagedType.Bool)]
		private static extern bool CancelIo(IntPtr hFile);

		[DllImport("kernel32.dll", SetLastError = true)]
		private static extern bool GetOverlappedResult(IntPtr hFile, [In()]ref System.Threading.NativeOverlapped lpOverlapped, ref uint lpNumberOfBytesTransferred, bool bWait);

		[DllImport("user32.dll", SetLastError = true)]
		private static extern IntPtr RegisterDeviceNotification(IntPtr hRecipient, IntPtr NotificationFilter, Int32 Flags);

		[DllImport("user32.dll", SetLastError = true)]
		private static extern bool UnregisterDeviceNotification(IntPtr hHandle);

		public struct DEV_BROADCAST_DEVICEINTERFACE
		{
			public Int32 dbcc_size;
			public Int32 dbcc_devicetype;
			public Int32 dbcc_reserved;
			[MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 16)]
			public byte[] dbcc_classguid;
			[MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
			public char[] dbcc_name;
		}


		public event UsbDeviceChangedEventHandler UsbDeviceChanged;
		public delegate void UsbDeviceChangedEventHandler(int message);

		private SafeFileHandle hDrv;
		private IntPtr deviceEventHandle;
		private object SyncObject = new object();

		public UsbDevice()
		{
			hDrv = null;
		}


		public bool RegisterForDeviceChange(bool Register, System.Windows.Forms.Form f)
		{
			bool Status = false;
			long LastError = 0;

			if (Register) {
				DEV_BROADCAST_DEVICEINTERFACE deviceInterface = new DEV_BROADCAST_DEVICEINTERFACE();
				int size = Marshal.SizeOf(deviceInterface);
				deviceInterface.dbcc_size = size;
				deviceInterface.dbcc_devicetype = (Int32) DBTDEVTYP.DBT_DEVTYP_DEVICEINTERFACE;
				IntPtr buffer = IntPtr.Zero;
				buffer = Marshal.AllocHGlobal(size);
				Marshal.StructureToPtr(deviceInterface, buffer, true);
				deviceEventHandle = RegisterDeviceNotification(f.Handle, buffer, Convert.ToInt32(DEVICE_NOTIFY.DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY.DEVICE_NOTIFY_ALL_INTERFACE_CLASSES));
				Status = (deviceEventHandle != IntPtr.Zero);
				if (!Status) {
					LastError = Marshal.GetLastWin32Error();
				}
				Marshal.FreeHGlobal(buffer);
			} else {
				if (deviceEventHandle != IntPtr.Zero) {
					Status = UnregisterDeviceNotification(deviceEventHandle);
				}
				deviceEventHandle = IntPtr.Zero;
			}

			return Status;
		}


		public void ProcessWindowsMessage(ref Message m)
		{
			Int32 devType;

			if (m.Msg == WM_DEVICECHANGE) {
				switch (m.WParam.ToInt32()) {
					case (Int32) DBTDEVICE.DBT_DEVICEARRIVAL:
						// New device has just arrived
						devType = Marshal.ReadInt32(m.LParam, 4);
						if (devType == (Int32) DBTDEVTYP.DBT_DEVTYP_DEVICEINTERFACE) {
							USBDeviceEventArgs e = new USBDeviceEventArgs();
							if (UsbDeviceChanged != null) {
								UsbDeviceChanged(1);
							}
						}
						break;
					case (Int32)DBTDEVICE.DBT_DEVICEQUERYREMOVE:
						// Device is about to be removed, any application can cancel the removal
						devType = Marshal.ReadInt32(m.LParam, 4);
						if (devType == (Int32) DBTDEVTYP.DBT_DEVTYP_DEVICEINTERFACE) {
							USBDeviceEventArgs e = new USBDeviceEventArgs();
							if (UsbDeviceChanged != null) {
								UsbDeviceChanged(2);
							}
						}
						break;

					case (Int32)DBTDEVICE.DBT_DEVICEREMOVECOMPLETE:
						// Device has been removed
						devType = Marshal.ReadInt32(m.LParam, 4);
						if (devType == (Int32) DBTDEVTYP.DBT_DEVTYP_DEVICEINTERFACE) {
							USBDeviceEventArgs e = new USBDeviceEventArgs();
							if (UsbDeviceChanged != null) {
								UsbDeviceChanged(0);
							}
						}
						break;

				}
			}
		}



		public bool OpenDevice(string DevicePath)
		{
			hDrv = CreateFile(DevicePath, EFileAccess.GENERIC_READ | EFileAccess.GENERIC_WRITE, EFileShare.FILE_SHARE_READ | EFileShare.FILE_SHARE_WRITE, IntPtr.Zero, ECreationDisposition.OPEN_EXISTING, 0, IntPtr.Zero);
			return !(hDrv.IsClosed | hDrv.IsInvalid);
		}

		public void CloseDevice()
		{
			if (hDrv != null)
				hDrv.Close();
		}


		public bool Write(byte[] buffer)
		{
			uint btr = 0;
			byte[] obuff = new byte[4096];
			bool r = false;
			lock (SyncObject) {
				if (hDrv.IsClosed | hDrv.IsInvalid)
					return false;
				r = DeviceIoControl(hDrv.DangerousGetHandle(), (uint)IOCTL.IOCTL_WRITE, Marshal.UnsafeAddrOfPinnedArrayElement(buffer, 0), (uint)buffer.Length, Marshal.UnsafeAddrOfPinnedArrayElement(obuff, 0), (uint)obuff.Length, ref btr, IntPtr.Zero);
			}
			return r;
		}

		public uint Read(byte[] buffer)
		{
			uint btr = 0;
			byte[] o = new byte[4];
			lock (SyncObject) {
				if (hDrv.IsClosed | hDrv.IsInvalid)
					return 0;
				DeviceIoControl(hDrv.DangerousGetHandle(), (uint)IOCTL.IOCTL_READ, Marshal.UnsafeAddrOfPinnedArrayElement(o, 0), (uint)o.Length, Marshal.UnsafeAddrOfPinnedArrayElement(buffer, 0), (uint)buffer.Length, ref btr, IntPtr.Zero);
			}
			return btr;
		}


		private uint GetDevicesByClass(string guid, List<string> devicePathName)
		{
			Guid g = new Guid(guid);
			uint i = 0;
			IntPtr h = SetupDiGetClassDevs(ref g, 0, IntPtr.Zero, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
			lock (SyncObject) {
				if (h.ToInt32() != INVALID_HANDLE_VALUE) {
					bool Success = true;
					while (Success) {
						SP_DEVICE_INTERFACE_DATA dia = new SP_DEVICE_INTERFACE_DATA();
						dia.cbSize = Marshal.SizeOf(dia);
						Success = SetupDiEnumDeviceInterfaces(h, IntPtr.Zero, ref g, i, ref dia);
						if (Success) {
							IntPtr detailDataBuffer = IntPtr.Zero;
							Int32 bufferSize = 0;
							SP_DEVINFO_DATA da = new SP_DEVINFO_DATA();
							da.cbSize = Marshal.SizeOf(da);
							SP_DEVICE_INTERFACE_DETAIL_DATA didd = new SP_DEVICE_INTERFACE_DETAIL_DATA();
							didd.DevicePath = new byte[1001];
							didd.cbSize = 4 + Marshal.SystemDefaultCharSize;
							int nBytes = didd.cbSize;
							Success = SetupDiGetDeviceInterfaceDetail(h, ref dia, IntPtr.Zero, 0, ref bufferSize, IntPtr.Zero);
							detailDataBuffer = Marshal.AllocHGlobal(bufferSize);
							Marshal.WriteInt32(detailDataBuffer, (IntPtr.Size == 4) ? (4 + Marshal.SystemDefaultCharSize) : 8);
							nBytes = bufferSize;
							Success = SetupDiGetDeviceInterfaceDetail(h, ref dia, detailDataBuffer, (uint)nBytes, ref bufferSize, IntPtr.Zero);
							if (Success) {
								IntPtr pDevicePathName = new IntPtr(detailDataBuffer.ToInt32() + 4);
								devicePathName.Add(Marshal.PtrToStringAuto(pDevicePathName));
								Marshal.FreeHGlobal(detailDataBuffer);
							}
						}
						i += 1;
					}
				}
			}
			return i;
		}

		public List<string> Get_Devices()
		{
			List<string> DevicesPathName = new List<string>();
			GetDevicesByClass(MINIPRO_GUID, DevicesPathName);
			return DevicesPathName;
		}
	}
}
namespace TL866
{

	public class USBDeviceEventArgs : EventArgs
	{
		/// <summary>
		/// Get/Set the value indicating that the event should be cancelled 
		/// Only in QueryRemove handler.
		/// </summary>

		public bool Cancel;
		/// <summary>
		/// Set to true in your DeviceArrived event handler if you wish to receive the 
		/// QueryRemove event for this device. 
		/// </summary>

		public bool HookQueryRemove;
		public USBDeviceEventArgs()
		{
			Cancel = false;
			HookQueryRemove = false;
		}
	}
}
