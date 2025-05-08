using System.Runtime.InteropServices;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace InfoIcDump
{
    internal class Infoic
    {
        //System API
        [DllImport("kernel32", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr LoadLibrary(string lpFileName);
        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern void FreeLibrary(IntPtr module);
        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr GetProcAddress(IntPtr module, string proc);

        // Function pointers delegates
        private delegate void DGetMfcStruT76(uint manufacturer, ref MfcStruct mfstruct);
        private delegate void DGetIcStruT76(uint manufacturer, uint device_index, ref InfoIc2Struct devstruct);
        private delegate uint DGetIcMFCT76(string search, uint[] manufacturer_array, uint type, uint mask);
        private delegate uint DGetIcListT76(string search, uint[] ic_array, uint manufacturer, uint type, uint mask);
        private delegate uint DGetDllInfoT76(ref uint dll_version, ref uint num_mfcs, uint version);

        private delegate void DGetMfcStru2(uint manufacturer, ref MfcStruct mfstruct);
        private delegate void DGetIcStru2(uint manufacturer, uint device_index, ref InfoIc2Struct devstruct);
        private delegate uint DGetIcMFC2(string search, uint[] manufacturer_array, uint type, uint mask);
        private delegate uint DGetIcList2(string search, uint[] ic_array, uint manufacturer, uint type, uint mask);
        private delegate uint DGetDllInfo2(ref uint dll_version, ref uint num_mfcs, uint version);

        private delegate void DGetMfcStru(uint manufacturer, ref MfcStruct mfstruct);
        private delegate void DGetIcStru(uint manufacturer, uint device_index, ref InfoIcStruct devstruct);
        private delegate uint DGetIcMFC(string search, uint[] manufacturer_array, uint type);
        private delegate uint DGetIcList(string search, uint[] ic_array, uint Manuf, uint type);
        private delegate uint DGetDllInfo(ref uint dll_version, ref uint num_mfcs);

        private readonly DGetMfcStru? GetMfcStruct;
        private readonly DGetIcStru? GetIcStruct;
        private readonly DGetIcMFC? GetIcMFC;
        private readonly DGetIcList? GetIcList;
        private readonly DGetDllInfo? GetDllInfo;

        private readonly DGetMfcStru2? GetMfcStruct2;
        private readonly DGetIcStru2? GetIcStruct2;
        private readonly DGetIcMFC2? GetIcMFC2;
        private readonly DGetIcList2? GetIcList2;
        private readonly DGetDllInfo2? GetDllInfo2;

        private readonly DGetMfcStruT76? GetMfcStructT76;
        private readonly DGetIcStruT76? GetIcStructT76;
        private readonly DGetIcMFCT76? GetIcMFCT76;
        private readonly DGetIcListT76? GetIcListT76;
        private readonly DGetDllInfoT76? GetDllInfoT76;


        public struct DevStruct
        {
            public uint ProtocolId;
            public uint Opts8;
            public uint Category;
            public string Name;
            public uint Variant;
            public uint CodeMemorySize;
            public uint DataMemorySize;
            public uint DataMemory2Size;
            public uint Opts7;
            public uint ReadBufferSize;
            public uint WriteBufferSize;
            public uint Opts1;
            public uint Opts2;
            public uint Opts3;
            public ulong ChipId;
            public uint Opts5;
            public uint ChipIdBytesCount;
            public uint Opts6;
            public uint PackageDetails;
            public uint Opts4;
            public string config;
        }

        // InfoIc device structure
        [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
        private struct InfoIcStruct
        {
            public uint ProtocolId;
            public uint Opts8;
            public uint Category;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 40)]
            public string Name;
            public uint Variant;
            public uint CodeMemorySize;
            public uint DataMemorySize;
            public uint DataMemory2Size;
            public ushort Opts7;
            public ushort ReadBufferSize;
            public ushort WriteBufferSize;
            public ushort Opts1;
            public uint Opts2;
            public uint Opts3;
            public uint ChipId;
            public uint Opts5;
            public uint ChipIdBytesCount;
            public uint Opts6;
            public uint PackageDetails;
            public uint Opts4;
        }//108 bytes

        // InfoIc2Plus device structure
        [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
        private struct InfoIc2Struct
        {
            public uint ProtocolId;
            public uint Opts8;
            public uint Category;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 40)]
            public string Name;
            public uint Variant;
            public uint CodeMemorySize;
            public uint DataMemorySize;
            public uint DataMemory2Size;
            public uint Opts7;
            public ushort ReadBufferSize;
            public ushort WriteBufferSize;
            public uint Opts5;
            public uint Opts1;
            public uint Opts2;
            public uint Opts3;
            public ulong ChipId;
            public uint ChipIdBytesCount;
            public uint Opts6;
            public uint PackageDetails;
            public uint Opts4;
        }//116 bytes

        // Manufacturer structure 
        [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
        private struct MfcStruct
        {
            public uint Manufacturer;
            public uint Icon;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 20)]
            public string ManufacturerName;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 40)]
            public string ManufacturerDescription;
            public IntPtr Devices;
            public uint Count;
        }

        public static readonly uint T56_FLAG = 0x10000000;
        public static readonly uint TL866II_FLAG = 0x20000000;
        public static readonly uint T48_FLAG = 0x40000000;
        public static readonly uint T76_FLAG = 0x80000000;
        public static readonly uint PROG_MASK = T56_FLAG | TL866II_FLAG | T48_FLAG | T76_FLAG;


        public static readonly uint SMT_FLAG = 0x80000000;
        public static readonly uint PLCC_FLAG = 0x40000000;
        public static readonly uint ERASE_FLAG = 0x20;

        public enum CHIP_TYPE : uint { MEMORY = 1, MPU, PLD, SRAM, LOGIC, NAND, EMMC, VGA }
        public enum DB_TYPE { INFOIC, INFOIC2, INFOICT76 }


        private readonly IntPtr HInfoIc = IntPtr.Zero;
        private readonly IntPtr HInfoIc2 = IntPtr.Zero;
        private readonly IntPtr HInfoIcT76 = IntPtr.Zero;
        private readonly uint InfoicDeviceCount;
        private readonly uint Infoic2DeviceCount;
        private readonly uint InfoicT76DeviceCount;
        private readonly uint InfoicManufCount;
        private readonly uint Infoic2ManufCount;
        private readonly uint InfoicT76ManufCount;

        public bool InfoIcLoaded { get => HInfoIc != IntPtr.Zero; }
        public bool InfoIc2Loaded { get => HInfoIc2 != IntPtr.Zero; }
        public bool InfoIcT76Loaded { get => HInfoIcT76 != IntPtr.Zero; }
        public uint InfoIcNumDevices { get => InfoicDeviceCount; }
        public uint InfoIc2NumDevices { get => Infoic2DeviceCount; }
        public uint InfoIcT76NumDevices { get => InfoicT76DeviceCount; }
        public uint InfoIcManufacturers { get => InfoicManufCount; }
        public uint InfoIc2Manufacturers { get => Infoic2ManufCount; }
        public uint InfoIcT76Manufacturers { get => InfoicT76ManufCount; }


        private static IntPtr GetProc(ref IntPtr handle, string name)
        {
            IntPtr ProcAddress = GetProcAddress(handle, name);
            if (ProcAddress == IntPtr.Zero)
            {
                FreeLibrary(handle);
                handle = IntPtr.Zero;
            }
            return ProcAddress;
        }

        // Class constructor
        public Infoic(string InfoicPath, string Infoic2Path, string InfoicT76Path)
        {
            // Try to load modules
            if (InfoicPath != string.Empty) { HInfoIc = LoadLibrary(InfoicPath); }
            if (Infoic2Path != string.Empty) { HInfoIc2 = LoadLibrary(Infoic2Path); }
            if (InfoicT76Path != string.Empty) { HInfoIcT76 = LoadLibrary(InfoicT76Path); }

            IntPtr ProcAddress;

            if (HInfoIc != IntPtr.Zero)
            {
                ProcAddress = GetProc(ref HInfoIc, "GetMfcStru");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetMfcStruct = (DGetMfcStru)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetMfcStru));

                ProcAddress = GetProc(ref HInfoIc, "GetIcStru");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetIcStruct = (DGetIcStru)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetIcStru));

                ProcAddress = GetProc(ref HInfoIc, "GetIcMFC");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetIcMFC = (DGetIcMFC)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetIcMFC));

                ProcAddress = GetProc(ref HInfoIc, "GetIcList");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetIcList = (DGetIcList)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetIcList));

                ProcAddress = GetProc(ref HInfoIc, "GetDllInfo");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetDllInfo = (DGetDllInfo)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetDllInfo));

                InfoicDeviceCount = GetDeviceCount(ref InfoicManufCount, DB_TYPE.INFOIC);
                if (InfoicDeviceCount == 0)
                {
                    FreeLibrary(HInfoIc);
                    HInfoIc = IntPtr.Zero;
                }
            }

            if (HInfoIc2 != IntPtr.Zero)
            {
                ProcAddress = GetProc(ref HInfoIc2, "GetMfcStru");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetMfcStruct2 = (DGetMfcStru2)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetMfcStru2));

                ProcAddress = GetProc(ref HInfoIc2, "GetIcStru");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetIcStruct2 = (DGetIcStru2)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetIcStru2));

                ProcAddress = GetProc(ref HInfoIc2, "GetIcMFC");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetIcMFC2 = (DGetIcMFC2)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetIcMFC2));

                ProcAddress = GetProc(ref HInfoIc2, "GetIcList");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetIcList2 = (DGetIcList2)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetIcList2));

                ProcAddress = GetProc(ref HInfoIc2, "GetDllInfo");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetDllInfo2 = (DGetDllInfo2)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetDllInfo2));

                Infoic2DeviceCount = GetDeviceCount(ref Infoic2ManufCount, DB_TYPE.INFOIC2);
                if (Infoic2DeviceCount == 0)
                {
                    FreeLibrary(HInfoIc2);
                    HInfoIc2 = IntPtr.Zero;
                }
            }

            if (HInfoIcT76 != IntPtr.Zero)
            {
                ProcAddress = GetProc(ref HInfoIcT76, "GetMfcStru");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetMfcStructT76 = (DGetMfcStruT76)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetMfcStruT76));

                ProcAddress = GetProc(ref HInfoIcT76, "GetIcStru");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetIcStructT76 = (DGetIcStruT76)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetIcStruT76));

                ProcAddress = GetProc(ref HInfoIcT76, "GetIcMFC");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetIcMFCT76 = (DGetIcMFCT76)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetIcMFCT76));

                ProcAddress = GetProc(ref HInfoIcT76, "GetIcList");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetIcListT76 = (DGetIcListT76)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetIcListT76));

                ProcAddress = GetProc(ref HInfoIcT76, "GetDllInfo");
                if (ProcAddress == IntPtr.Zero) { return; }
                GetDllInfoT76 = (DGetDllInfoT76)Marshal.GetDelegateForFunctionPointer(ProcAddress, typeof(DGetDllInfoT76));

                InfoicT76DeviceCount = GetDeviceCount(ref InfoicT76ManufCount, DB_TYPE.INFOICT76);
                if (InfoicT76DeviceCount == 0)
                {
                    FreeLibrary(HInfoIcT76);
                    HInfoIcT76 = IntPtr.Zero;
                }
            }
        }

        private void GetMfc(uint manufacturer, ref MfcStruct mfcstruct, DB_TYPE type)
        {
            if (type == DB_TYPE.INFOIC)
            {
                if (GetMfcStruct is not null)
                {
                    GetMfcStruct(manufacturer, ref mfcstruct);
                }
            }
            else if (type == DB_TYPE.INFOIC2)
            {
                if (GetMfcStruct2 is not null)
                {
                    GetMfcStruct2(manufacturer, ref mfcstruct);
                }
            }
            else
            {
                if (GetMfcStructT76 is not null)
                {
                    GetMfcStructT76(manufacturer, ref mfcstruct);
                }

            }
        }

        public uint GetMfcDevices(uint manufacturer, DB_TYPE type)
        {
            MfcStruct mfcstruct = new();
            GetMfc(manufacturer, ref mfcstruct, type);
            return mfcstruct.Count;
        }

        public string GetManufName(uint manufacturer, DB_TYPE type)
        {
            MfcStruct mfcstruct = new();
            GetMfc(manufacturer, ref mfcstruct, type);
            return mfcstruct.ManufacturerName;
        }

        public DevStruct GetDevice(uint manufacturer, uint index, DB_TYPE type)
        {
            DevStruct devstruct = new();
            if (type == DB_TYPE.INFOIC)
            {
                if (GetIcStruct is null) { return devstruct; }
                InfoIcStruct device = new();
                device.Name = string.Empty;
                GetIcStruct(manufacturer, index, ref device);
                devstruct.ProtocolId = device.ProtocolId;
                devstruct.Opts8 = device.Opts8;
                devstruct.Category = device.Category;
                devstruct.Name = device.Name;
                devstruct.Variant = device.Variant;
                devstruct.CodeMemorySize = device.CodeMemorySize;
                devstruct.DataMemorySize = device.DataMemorySize;
                devstruct.DataMemory2Size = device.DataMemory2Size;
                devstruct.Opts7 = device.Opts7;
                devstruct.ReadBufferSize = device.ReadBufferSize;
                devstruct.WriteBufferSize = device.WriteBufferSize;
                devstruct.Opts1 = device.Opts1;
                devstruct.Opts2 = device.Opts2;
                devstruct.Opts3 = device.Opts3;
                devstruct.ChipId = device.ChipId;
                devstruct.Opts5 = device.Opts5;
                devstruct.ChipIdBytesCount = device.ChipIdBytesCount;
                devstruct.Opts6 = device.Opts6;
                devstruct.PackageDetails = device.PackageDetails;
                devstruct.Opts4 = device.Opts4;
            }
            else if (type == DB_TYPE.INFOIC2)
            {
                if (GetIcStruct2 is null) { return devstruct; }
                InfoIc2Struct device = new();
                device.Name = string.Empty;
                GetIcStruct2(manufacturer, index, ref device);
                devstruct.ProtocolId = device.ProtocolId;
                devstruct.Opts8 = device.Opts8;
                devstruct.Category = device.Category;
                devstruct.Name = device.Name;
                devstruct.Variant = device.Variant;
                devstruct.CodeMemorySize = device.CodeMemorySize;
                devstruct.DataMemorySize = device.DataMemorySize;
                devstruct.DataMemory2Size = device.DataMemory2Size;
                devstruct.Opts7 = device.Opts7;
                devstruct.ReadBufferSize = device.ReadBufferSize;
                devstruct.WriteBufferSize = device.WriteBufferSize;
                devstruct.Opts1 = device.Opts1;
                devstruct.Opts2 = device.Opts2;
                devstruct.Opts3 = device.Opts3;
                devstruct.ChipId = device.ChipId;
                devstruct.Opts5 = device.Opts5;
                devstruct.ChipIdBytesCount = device.ChipIdBytesCount;
                devstruct.Opts6 = device.Opts6;
                devstruct.PackageDetails = device.PackageDetails;
                devstruct.Opts4 = device.Opts4;
            }
            else
            {
                if (GetIcStructT76 is null) { return devstruct; }
                InfoIc2Struct device = new();
                device.Name = string.Empty;
                GetIcStructT76(manufacturer, index, ref device);
                devstruct.ProtocolId = device.ProtocolId;
                devstruct.Opts8 = device.Opts8;
                devstruct.Category = device.Category;
                devstruct.Name = device.Name;
                devstruct.Variant = device.Variant;
                devstruct.CodeMemorySize = device.CodeMemorySize;
                devstruct.DataMemorySize = device.DataMemorySize;
                devstruct.DataMemory2Size = device.DataMemory2Size;
                devstruct.Opts7 = device.Opts7;
                devstruct.ReadBufferSize = device.ReadBufferSize;
                devstruct.WriteBufferSize = device.WriteBufferSize;
                devstruct.Opts1 = device.Opts1;
                devstruct.Opts2 = device.Opts2;
                devstruct.Opts3 = device.Opts3;
                devstruct.ChipId = device.ChipId;
                devstruct.Opts5 = device.Opts5;
                devstruct.ChipIdBytesCount = device.ChipIdBytesCount;
                devstruct.Opts6 = device.Opts6;
                devstruct.PackageDetails = device.PackageDetails;
                devstruct.Opts4 = device.Opts4;
            }
            // Fix bad endiannes in database
            devstruct.ChipId = ToLittleEndian(devstruct.ChipId, devstruct.ChipIdBytesCount);

            // Patch opts8 and package_details
            if (type != DB_TYPE.INFOIC)
            {
                PatchDevice(ref devstruct);
            }
            return devstruct;
        }

        private uint GetDeviceCount(ref uint manuf_count, DB_TYPE type)
        {
            uint count = 0;
            uint dll_version = 0;

            // Get dll info
            if (type == DB_TYPE.INFOIC)
            {
                if (GetDllInfo is null) { return 0; }
                GetDllInfo(ref dll_version, ref manuf_count);
                if (manuf_count > 136) { return 0; }
            }
            else if (type == DB_TYPE.INFOIC2)
            {
                if (GetDllInfo2 is null) { return 0; }
                GetDllInfo2(ref dll_version, ref manuf_count, 0);
                if (manuf_count < 139) { return 0; }
            }
            else
            {
                if (GetDllInfoT76 is null) { return 0; }
                GetDllInfoT76(ref dll_version, ref manuf_count, 0);
                if (manuf_count < 170) { return 0; }
            }

            //Iterate over the entire manufacturers and count all devices
            for (uint i = 0; i < manuf_count; i++)
            {

                MfcStruct mfcstruct = new();
                if (type == DB_TYPE.INFOIC)
                {
                    if (GetMfcStruct is null) { return 0; }
                    GetMfcStruct(i, ref mfcstruct);
                }
                else if (type == DB_TYPE.INFOIC2)
                {
                    if (GetMfcStruct2 is null) { return 0; }
                    GetMfcStruct2(i, ref mfcstruct);
                }
                else if (type == DB_TYPE.INFOICT76)
                {
                    if (GetMfcStructT76 is null) { return 0; }
                    GetMfcStructT76(i, ref mfcstruct);
                }
                count += mfcstruct.Count;
            }
            return count;
        }

        private ulong ToLittleEndian(ulong value, uint size)
        {
            if (value == 0 || size == 0) return 0;// This is a database bug. Size is zero and id garbage bytes

            return ((0xFF) & (value >> 56) | (0xFF00) & (value >> 40) | (0xFF0000) & (value >> 24) | (0xFF000000) & (value >> 8) |
                (0xFF00000000) & (value << 8) | (0xFF0000000000) & (value << 24) | (0xFF000000000000) & (value << 40) |
                (0xFF00000000000000) & (value << 56)) >> (int)(8 * (8 - size));
        }

        private void PatchDevice(ref DevStruct Device)
        {
            // Patch package details
            PatchPackage(ref Device);

            // Patch Opts5
            if (Device.Opts7 == 0x06 && Device.Opts5 >= 0xF0)
            {
                Device.Opts5 = 0xF0;
            }
            Device.Opts5 = (Device.Opts5 & 0xFFFF00FF) | (Device.Opts1 << 8);

            // Patch Opts8
            uint lo8 = PatchLo8(ref Device);
            uint hi8 = PatchHi8(ref Device);
            Device.Opts8 = (Device.Opts8 & 0xFFFF0000) | (hi8 << 8) | lo8;

            // Patch variant high byte
            if((Device.Variant & 0xFF00) == 0)
            {
                uint algo = GetAlgoNumber(Device);
                Device.Variant = (Device.Variant & 0x00FF) | (algo << 8);
            }
        }

        private static byte GetAlgoNumber(DevStruct device)
        {
            byte variantLoByte = (byte)(device.Variant & 0xFF);
            byte voltagesHiByte = (byte)((device.Opts5 >> 8) & 0xFF);

            switch (device.ProtocolId)
            {
                case 0x01:
                    if ((variantLoByte & 3) == 1)
                    {
                        if (device.CodeMemorySize < 0x8000)
                            return 0x1A;
                        return 0x11;
                    }
                    if ((variantLoByte & 3) == 0)
                        return 0x13;
                    if ((variantLoByte & 3) == 2)
                        return 0x14;
                    return (byte)(variantLoByte & 3);
                case 0x02:
                    if (variantLoByte >= 0)
                    {
                        if ((voltagesHiByte & 0xF) != 2)
                            return 0x21;
                        return 0x2A;
                    }
                    else if ((variantLoByte & 0x20) != 0)
                    {
                        if ((voltagesHiByte & 0xF) == 1)
                        {
                            return 0x69;
                        }
                        else if ((voltagesHiByte & 0xF) == 2)
                        {
                            return 0x68;
                        }
                        else
                        {
                            return 0x67;
                        }
                    }
                    else if ((voltagesHiByte & 0xF) == 1)
                    {
                        return 0x2B;
                    }
                    else
                    {
                        if ((voltagesHiByte & 0xF) != 2)
                            return 0x11;
                        return 0x1A;
                    }
                case 0x03:
                case 0x0F:
                    if ((variantLoByte & 0xF0) == 32)
                    {
                        if ((variantLoByte & 3) == 3)
                        {
                            return 0x20;
                        }
                        else if ((variantLoByte & 3) == 2)
                        {
                            return 0x21;
                        }
                    }
                    else
                    {
                        switch (variantLoByte & 3)
                        {
                            case 3:
                                return 0x10;
                            case 2:
                                return 0x11;
                            case 1:
                                return 0x12;
                            default:
                                if ((variantLoByte & 3) == 0)
                                    return 0x13;
                                break;
                        }
                    }
                    return (byte)(variantLoByte & 0xF0);
                case 0x05:
                    if (device.PackageDetails == 5)
                        return 0x76;
                    else
                        return 0x75;
                case 0x06:
                    if ((variantLoByte & 0x80) != 0)
                    {
                        if (device.PackageDetails == 5)
                            return 0x73;
                        return 0x71;
                    }
                    if (device.PackageDetails == 5)
                    {
                        return 0x72;
                    }
                    else
                    {
                        if (device.CodeMemorySize != 0x80000)
                            return 0x71;
                        return 0x70;
                    }
                case 0x07:
                    if ((variantLoByte & 0x10) != 0)
                    {
                        if (device.CodeMemorySize == 0x10000)
                        {
                            return 0x31;
                        }
                        else if (device.CodeMemorySize == 0x8000)
                        {
                            return 0x32;
                        }
                        else
                        {
                            return 0x33;
                        }
                    }
                    else
                    {
                        return 0x41;
                    }
                case 0x08:
                    if (device.PackageDetails == 5)
                    {
                        if (variantLoByte == 4)
                        {
                            return 0x22;
                        }
                        else if (variantLoByte == 3)
                        {
                            return 0x23;
                        }
                        else
                        {
                            if (variantLoByte != 2)
                                return 0x21;
                            return 0x24;
                        }
                    }
                    else
                    {
                        if (variantLoByte == 4)
                            return 0x12;
                        if (variantLoByte == 3)
                            return 0x13;
                        if (variantLoByte != 2)
                            return 0x11;
                        return 0x14;
                    }
                case 0x09:
                    if (device.PackageDetails == 0x28000000)
                    {
                        if (device.CodeMemorySize != 0x80000)
                            return 0x1A;
                        return 0x2A;
                    }
                    if (device.PackageDetails == 0xFD000000)
                    {
                        if (device.CodeMemorySize == 0x80000)
                            return 0x2B;
                        return 0x1B;
                    }
                    else if (device.PackageDetails == 4)
                    {
                        if (device.CodeMemorySize == 0x80000)
                            return 0x2C;
                        else
                            return 0x1C;
                    }
                    return (byte)device.PackageDetails;
                case 0x0A:
                    if ((variantLoByte & 0x80) != 0)
                    {
                        return 0x42;
                    }
                    else if (device.CodeMemorySize == 0x10000)
                    {
                        return 0x34;
                    }
                    else if (device.CodeMemorySize == 0x8000)
                    {
                        return 0x35;
                    }
                    else
                    {
                        return 0x36;
                    }
                case 0x0B:
                    if ((variantLoByte & 0x10) != 0)
                    {
                        return 0x43;
                    }
                    else if (device.CodeMemorySize == 0x800)
                    {
                        return 0x3B;
                    }
                    else
                    {
                        return 0x3A;
                    }
                case 0x0D:
                    if (device.PackageDetails == 5)
                        return 0x45;
                    else
                        return 0x44;
                case 0x0E:
                    return 0x50;
                case 0x10:
                    if (variantLoByte == 16 || variantLoByte == 17)
                    {
                        if (device.PackageDetails == 5)
                            return 0x7E;
                        else
                            return 0x7B;
                    }
                    else if (variantLoByte == 18)
                    {
                        if (device.PackageDetails == 5)
                            return 0x7F;
                        else
                            return 0x7C;
                    }
                    else if (device.PackageDetails == 5)
                    {
                        return 0x7D;
                    }
                    else
                    {
                        return 0x7A;
                    }
                case 0x11:
                    if (device.PackageDetails == 5)
                    {
                        if ((variantLoByte & 0xF) == 1)
                            return 0x92;
                        else
                            return 0x94;
                    }
                    else if (device.PackageDetails == 3)
                    {
                        if ((variantLoByte & 0xF) == 1)
                            return 0x95;
                        else
                            return 0x96;
                    }
                    else if ((variantLoByte & 0xF) == 1)
                    {
                        return 0x91;
                    }
                    else
                    {
                        return 0x93;
                    }
                default:
                    return 0x00;
            }
        }

        // Patch PackageDetails and Opts4
        private static void PatchPackage(ref DevStruct Device)
        {
            if (Device.ProtocolId == 0x2D) return;
            if (Device.ProtocolId == 3)
            {
                Device.Opts4 |= 0x100048;
                if ((Device.PackageDetails & 0xFF00) == 0x500) return;
                Device.PackageDetails |= 0x900;
                return;
            }
            else
            {
                if (Device.ProtocolId == 2)
                {
                    if ((Device.Variant & 0x20) != 0) return;
                    Device.Opts4 |= 0x100000;
                    Device.PackageDetails |= 0xA00;
                    return;
                }
                else
                {
                    if (Device.ProtocolId != 1 || (byte)Device.Opts1 != 0) return;
                    Device.Opts4 |= 0x100000;
                    Device.PackageDetails |= 0xB00;
                }
            }
        }

        // Patch Opts8 hibyte
        private static byte PatchHi8(ref DevStruct Device)
        {
            byte Opts8Hibyte = (byte)(Device.Opts8 >> 8);
            byte Opts5Hibyte = (byte)(Device.Opts5 >> 8);

            // Opts8HiByte != 0
            if (Opts8Hibyte != 0x00)
            {
                if (Opts8Hibyte == 0xFF)
                {
                    return 0x00;
                }
                return Opts8Hibyte;
            }

            // Opts8HiByte == 0
            switch (Device.ProtocolId)
            {
                case 0x01:
                    return 0x80;
                case 0x02:
                    switch (Opts5Hibyte & 0x0F)
                    {
                        case 0x01:
                            return 0x7F;
                        case 0x02:
                            return 0x7E;
                        default:
                            return 0x7D;
                    }
                case 0x03:
                case 0x0F:
                    return (byte)((Device.Variant & 0xF0) == 0x20 ? 0x15 : 0x03);
                case 0x04:
                    return 0x03;
                case 0x05:
                    switch (Device.PackageDetails)
                    {
                        case 0x20000000:
                        case 0xA0000000:
                        case 0xFF000000:
                            switch (Device.CodeMemorySize)
                            {
                                case 0x10000:
                                    return 0x17;
                                case 0x20000:
                                    return 0x18;
                                case 0x40000:
                                    return 0x19;
                                case 0x80000:
                                    return 0x1A;
                                default:
                                    return 0x16;
                            }
                        default:
                            switch (Device.CodeMemorySize)
                            {
                                case 0x10000:
                                    return 0x1C;
                                case 0x20000:
                                    return 0x1D;
                                case 0x40000:
                                    return 0x1E;
                                case 0x80000:
                                    return 0x1F;
                                default:
                                    return 0x1B;
                            }
                    }
                case 0x06:
                    if ((Device.Variant & 0x80) == 0x00)
                    {
                        if (Device.PackageDetails == 0x20000000 ||
                            Device.PackageDetails == 0xA0000000 ||
                            Device.PackageDetails == 0xFF000000)
                        {
                            switch (Device.CodeMemorySize)
                            {
                                case 0x10000:
                                    return 0x59;
                                case 0x20000:
                                    return 0x5A;
                                case 0x40000:
                                    return 0x5B;
                                case 0x80000:
                                    return 0x5C;
                            }
                        }
                        else
                        {
                            switch (Device.CodeMemorySize)
                            {
                                case 0x10000:
                                    return 0x62;
                                case 0x20000:
                                    return 0x63;
                                case 0x40000:
                                    return 0x64;
                                case 0x80000:
                                    return 0x65;
                            }
                        }
                    }
                    else
                    {
                        if (Device.PackageDetails == 0x20000000 ||
                            Device.PackageDetails == 0xA0000000 ||
                            Device.PackageDetails == 0xFF000000)
                        {
                            switch (Device.CodeMemorySize)
                            {
                                case 0x10000:
                                    return 0x5E;
                                case 0x20000:
                                    return 0x5F;
                                case 0x40000:
                                    return 0x60;
                                default:
                                    return 0x5D;
                            }
                        }
                        else
                        {
                            switch (Device.CodeMemorySize)
                            {
                                case 0x10000:
                                    return 0x67;
                                case 0x20000:
                                    return 0x68;
                                case 0x40000:
                                    return 0x69;
                                default:
                                    return 0x66;
                            }
                        }
                    }
                    break;
                case 0x07:
                    if ((Device.Variant & 0x20) == 0x00)
                    {
                        if (Device.CodeMemorySize == 0x2000)
                        {
                            return 0x6F;
                        }
                        return 0x0C;
                    }
                    else
                    {
                        if (Device.CodeMemorySize == 0x800)
                        {
                            return 0x71;
                        }
                        if (Device.CodeMemorySize == 0x2000)
                        {
                            return 0x70;
                        }
                        return 0x0C;
                    }
                case 0x08:
                    switch (Device.Variant)
                    {
                        case 0x04:
                            switch (Device.PackageDetails)
                            {
                                case 0x05:
                                    switch (Device.CodeMemorySize)
                                    {
                                        case 0x10000:
                                            return 0x62;
                                        case 0x20000:
                                            return 0x63;
                                        case 0x80000:
                                            return 0x65;
                                        default:
                                            return 0x64;
                                    }
                                default:
                                    switch (Device.CodeMemorySize)
                                    {
                                        case 0x10000:
                                            return 0x59;
                                        case 0x20000:
                                            return 0x5A;
                                        case 0x80000:
                                            return 0x5C;
                                        default:
                                            return 0x5B;
                                    }
                            }
                        default:
                            switch (Device.PackageDetails)
                            {
                                case 0x05:
                                    switch (Device.CodeMemorySize)
                                    {
                                        case 0x10000:
                                            return 0x67;
                                        case 0x20000:
                                            return 0x68;
                                        default:
                                            return 0x69;
                                    }
                                default:
                                    switch (Device.CodeMemorySize)
                                    {
                                        case 0x10000:
                                            return 0x5E;
                                        case 0x20000:
                                            return 0x5F;
                                        default:
                                            return 0x60;
                                    }
                            }
                    }
                case 0x09:
                    switch (Device.PackageDetails)
                    {
                        case 4:
                            return (byte)(Device.CodeMemorySize == 0x20000 ? 0x79 : 0x0F);
                        case 0xFD000000:
                            return (byte)(Device.CodeMemorySize == 0x20000 ? 0x7A : 0x7B);
                        case 0x2A000000:
                            return (byte)(Device.CodeMemorySize == 0x100000 ? 0x7C : 0x10);
                        case 2:
                            return 0x7C;
                        default:
                            return (byte)(Device.CodeMemorySize == 0x20000 ? 0x78 : 0x0F);
                    }
                case 0x0A:
                    return (byte)((Device.Variant & 0x80) != 0x00 ? 0x75 : (Device.Variant != 0 ? 0x77 : 0x76));
                case 0x0B:
                case 0x2B:
                case 0x2C:
                    return 0x0B;
                case 0x0D:
                    switch (Device.CodeMemorySize)
                    {
                        case 0x20000:
                            return (byte)(Device.PackageDetails == 5 ? 0x63 : 0x5A);
                        case 0x40000:
                            return (byte)(Device.PackageDetails == 5 ? 0x64 : 0x5B);
                        default:
                            return 0x0D;
                    }
                case 0x10:
                    if (Device.Variant == 0x10 || Device.Variant == 0x11 || Device.Variant == 0x12)
                    {
                        return 0x0D;
                    }
                    else if (Device.PackageDetails == 0x20000000 || Device.PackageDetails == 0xA0000000 || Device.PackageDetails == 0xFF000000)
                    {
                        switch (Device.CodeMemorySize)
                        {
                            case 0x10000:
                                return 0x5E;
                            case 0x20000:
                                return 0x5F;
                            case 0x40000:
                                return 0x60;
                            default:
                                return 0x5D;
                        }
                    }
                    else
                    {
                        switch (Device.CodeMemorySize)
                        {
                            case 0x10000:
                                return 0x67;
                            case 0x20000:
                                return 0x68;
                            case 0x40000:
                                return 0x69;
                            default:
                                return 0x66;
                        }
                    }
                case 0x11:
                    if (Device.PackageDetails == 0x20000000 ||
                         Device.PackageDetails == 0xA0000000 ||
                         Device.PackageDetails == 0xFF000000)
                    {
                        if (Opts5Hibyte == 0x01)
                        {
                            return 0x6E;
                        }
                        else if ((Device.Variant & 0x80) != 0x00)
                        {
                            return 0x6A;
                        }
                        else
                        {
                            return 0x6B;
                        }
                    }
                    else
                    {
                        if (Opts5Hibyte == 0x01)
                        {
                            return 0x6E;
                        }
                        else if ((Device.Variant & 0x80) != 0x00)
                        {
                            return 0x6C;
                        }
                        else
                        {
                            return 0x6D;
                        }
                    }
                case 0x12:
                    switch ((byte)Device.PackageDetails)
                    {
                        case 0x02:
                            switch (Device.CodeMemorySize)
                            {
                                case 0x40000:
                                    return 0x2B;
                                case 0x80000:
                                    return 0x2C;
                                case 0x100000:
                                    return 0x2D;
                                case 0x200000:
                                    return 0x2E;
                                default:
                                    return 0x2A;
                            }
                        case 0x01:
                            switch (Device.CodeMemorySize)
                            {
                                case 0x100000:
                                    return 0x26;
                                case 0x200000:
                                    return 0x27;
                                case 0x400000:
                                    return 0x28;
                                case 0x800000:
                                    return 0x29;
                                default:
                                    if (Device.CodeMemorySize == 0x40000)
                                    {
                                        return 0x24;
                                    }
                                    if (Device.CodeMemorySize == 0x80000)
                                    {
                                        return 0x25;
                                    }
                                    return 0x23;
                            }
                        case 0x0B:
                            return 0x14;
                        default:
                            if ((Device.PackageDetails & 0xFF000000) == 0xE1000000)
                            {
                                switch (Device.CodeMemorySize)
                                {
                                    case 0x100000:
                                        return 0x32;
                                    case 0x200000:
                                        return 0x33;
                                    case 0x400000:
                                        return 0x34;
                                    case 0x800000:
                                        return 0x35;
                                    case 0x20000:
                                        return 0x2F;
                                    case 0x40000:
                                        return 0x30;
                                    case 0x80000:
                                        return 0x31;
                                }
                            }
                            return 0x12;
                    }
                case 0x13:
                    byte vr2 = (byte)(Device.Variant & 0xF0);

                    if (Opts5Hibyte == 0x00)
                    {
                        if (vr2 == 0x20 || vr2 == 0x40)
                        {
                            Dictionary<uint, byte> map = new() { { 0x40000, 0x54 }, { 0x80000, 0x54 },
                                { 0x100000, 0x55 }, { 0x200000, 0x56 }, { 0x400000, 0x57 } };
                            return (byte)(map.ContainsKey(Device.CodeMemorySize) ? map[Device.CodeMemorySize] : 0x00);
                        }
                        else
                        {
                            Dictionary<uint, byte> map = new() { { 0x100000, 0x51 }, { 0x200000, 0x52 }, { 0x400000, 0x53 } };

                            return (byte)(map.ContainsKey(Device.CodeMemorySize) ? map[Device.CodeMemorySize] : 0x50);
                        }
                    }
                    else
                    {
                        if (vr2 == 0x10)
                        {
                            Dictionary<uint, byte> map = new() { { 0x40000, 0x46 }, { 0x80000, 0x47 }, { 0x100000, 0x48 },
                                { 0x200000, 0x49 }, { 0x400000, 0x4A } };

                            return (byte)(map.ContainsKey(Device.CodeMemorySize) ? map[Device.CodeMemorySize] : 0x00);
                        }
                        else if (vr2 == 0x30)
                        {
                            Dictionary<uint, byte> map = new(){{ 0x40000, 0x4B },{ 0x80000, 0x4C }, { 0x100000, 0x4D },
                                { 0x200000, 0x4E },{ 0x400000, 0x4F }};
                            return (byte)(map.ContainsKey(Device.CodeMemorySize) ? map[Device.CodeMemorySize] : 0x00);
                        }
                        else
                        {
                            Dictionary<uint, byte> map = new() { { 0x80000, 0x42 }, { 0x100000, 0x43 }, { 0x200000, 0x44 }, { 0x400000, 0x45 } };
                            return (byte)(map.ContainsKey(Device.CodeMemorySize) ? map[Device.CodeMemorySize] : 0x41);
                        }
                    }
                case 0x14:
                    return 0x3F;
                case 0x15:
                    return 0x3E;
                case 0x18:
                case 0x19:
                case 0x1A:
                case 0x1B:

                    Dictionary<uint, uint> table = new() { { 0xE000000, 0x06 }, { 0x8000000, 0x03 }, { 0x12000000, 0x08 },
                        { 0x14000000, 0x09 }, { 0x1c000000, 0x37 }, { 0x94000000, 0x09 }, { 0x8E000000, 0x06 }, { 0x28000000, 0x39 }, { 0x88000000, 0x03 },
                        { 0x92000000, 0x08 }, { 0xFA000000, 0x3B }, { 0x9C000000, 0x37 }, { 0xF9000000, 0x3A }, { 0xFB000000, 0x38 } };
                    return (byte)(table.ContainsKey(Device.PackageDetails & 0xFF000000) ? table[Device.PackageDetails & 0xFF000000] : 0);

                case 0x1C:
                    return 0x74;
                case 0x1D:
                    if ((byte)Device.PackageDetails == 0x07)
                    {
                        return 0x14;
                    }
                    else
                    {
                        switch (Device.Variant & 0xE0)
                        {
                            case 0x20:
                            case 0x40:
                                return 0x0F;
                            case 0x60:
                                return 0x09;
                            default:
                                return 0x03;
                        }
                    }
                case 0x1E:
                    return (byte)((Device.Variant & 0x02) == 0x00 ? 0x03 : 0x06);
                case 0x1F:
                case 0x20:
                case 0x2A:
                    return 0x09;
                case 0x21:
                case 0x22:
                case 0x23:
                case 0x24:
                case 0x25:
                case 0x26:
                    return 0x11;
                case 0x2D:
                    return 0x81;
            }
            return Opts8Hibyte;
        }

        // Patch opts8_lobyte
        private static byte PatchLo8(ref DevStruct Device)
        {
            byte Opts8Lobyte = (byte)Device.Opts8;
            byte Opts5Hibyte = (byte)(Device.Opts5 >> 0x08);

            // Opts8HiByte != 0
            if (Opts8Lobyte != 0)
            {
                if (Opts8Lobyte == 0x4)
                {
                    return 0x00;
                }
                return Opts8Lobyte;
            }

            // Opts8LoByte == 0
            switch (Device.ProtocolId)
            {
                case 0x01:
                    return 0x6B;
                case 0x02:
                    if ((Opts5Hibyte & 0x0F) == 0x01)
                    {
                        return 0x6E;
                    }
                    else
                    {
                        return (byte)(((Opts5Hibyte & 0xF) == 0x02) ? 0x6D : 0x6C);
                    }
                case 0x04:
                    return 0x71;
                case 0x03:
                case 0x0F:
                    switch (Device.Variant & 0xF0)
                    {
                        case 0x20:
                            return 0x03;
                        case 0x10:
                            return (byte)(Opts8Lobyte + 0x02);
                        default:
                            return (byte)(Opts8Lobyte + 0x01);
                    }
                case 0x06:
                    switch (Device.CodeMemorySize)
                    {
                        case 0x10000:
                            return 0x7;
                        case 0x20000:
                            return (byte)((Device.Variant & 0x80) == 0x0 ? 0x09 : 0x0A);
                        case 0x40000:
                            return (byte)((Device.Variant & 0x80) == 0x0 ? 0x0B : 0x0C);
                        case 0x80000:
                            return 0x0D;
                    }
                    break;
                case 0x07:
                    if ((Device.Variant & 0x20) != 0x00)
                    {
                        switch (Device.CodeMemorySize)
                        {
                            case 0x8000:
                                return 0x14;
                            case 0x2000:
                                return 0x13;
                            default:
                                return 0x12;
                        }
                    }
                    else
                    {
                        return (byte)(Device.CodeMemorySize == 0x2000 ? 0x15 : 0x16);
                    }
                case 0x08:
                    switch (Device.Variant)
                    {
                        case 0x00:
                            return 0x0A;
                        case 0x04:
                            switch (Device.CodeMemorySize)
                            {
                                case 0x10000:
                                    return 0x07;
                                case 0x20000:
                                    return 0x09;
                                default:
                                    return 0x0D;
                            }
                        default:
                            return 0x0C;
                    }
                case 0x09:
                    return (byte)((Device.Variant == 0x00) ? 0x18 : 0x19);
                case 0x0A:
                    if ((Device.Variant & 0x80) == 0)
                    {
                        return (byte)(Device.Variant == 0x00 ? 0x1E : 0x1F);
                    }
                    else
                    {
                        return 0x1A;
                    }
                case 0x0B:
                    return 0x17;
                case 0x0D:
                    switch (Device.CodeMemorySize)
                    {
                        case 0x20000:
                            return 0x09;
                        case 0x40000:
                            return 0x0B;
                        case 0x80000:
                            return 0x0D;
                        default:
                            return 0x09;
                    }
                case 0x10:
                    switch (Device.CodeMemorySize)
                    {
                        case 0x8000:
                            return (byte)(Device.Variant != 0x00 ? 0x06 : 0x05);
                        case 0x10000:
                            return (byte)(Device.Variant != 0x00 ? 0x08 : 0x07);
                        case 0x20000:
                            return (byte)(Device.Variant != 0x00 ? 0x0A : 0x09);
                        case 0x40000:
                            return (byte)(Device.Variant != 0x00 ? 0x0C : 0x0B);
                        case 0x80000:
                            return 0x0D;
                    }
                    break;
                case 0x11:
                    if (Opts5Hibyte == 0x01)
                    {
                        return (byte)((Device.Variant & 0x80) != 0x00 ? 0x4B : 0x4C);
                    }
                    else
                    {
                        return (byte)((Device.Variant >> 0x07) | 0x0E);
                    }
                case 0x12:
                    Dictionary<uint, byte> map;
                    if ((byte)Device.PackageDetails == 0x02)
                    {
                        map = new() { { 0x200000, 0x55 }, { 0x100000, 0x54 }, { 0x80000, 0x53 } };
                        if (map.TryGetValue(Device.CodeMemorySize, out byte ret))
                        {
                            return ret;
                        }
                        else
                        {
                            return (byte)(Device.CodeMemorySize == 0x40000 ? 0x52 : 0x51);
                        }
                    }
                    else if (Opts5Hibyte == 0xFF || Opts5Hibyte == 0x01)
                    {
                        if (Device.CodeMemorySize == 0x800000)
                        {
                            return 0x4E;
                        }
                        else
                        {
                            map = new() { { 0x400000, 0x61 }, { 0x200000, 0x60 }, { 0x100000, 0x5F }, { 0x80000, 0x5E } };
                            if (map.TryGetValue(Device.CodeMemorySize, out byte ret))
                            {
                                return ret;
                            }
                            else
                            {
                                return (byte)(Device.CodeMemorySize != 0x40000 ? 0x5C : 0x5D);
                            }
                        }
                    }
                    else
                    {
                        if (Opts5Hibyte == 0x00)
                        {
                            if (Device.CodeMemorySize == 0x800000)
                            {
                                return 0x4E;
                            }
                            else
                            {
                                map = new() { { 0x400000, 0x57 }, { 0x200000, 0x58 }, { 0x100000, 0x59 }, { 0x80000, 0x5A } };

                                if (map.TryGetValue(Device.CodeMemorySize, out byte ret))
                                {
                                    return ret;
                                }
                                else
                                {
                                    return (byte)(Device.CodeMemorySize == 0x40000 ? 0x5B : 0x4E);
                                }
                            }
                        }
                    }
                    break;
                case 0x13:
                    byte variant = (byte)(Device.Variant & 0xF0);
                    map = [];
                    if (variant != 0x00)
                    {
                        switch (variant)
                        {
                            case 0x10:
                                map = new() { { 0x400000, 0x37 }, { 0x200000, 0x38 }, { 0x100000, 0x39 }, { 0x40000, 0x3B }, { 0x80000, 0x3A } };
                                break;
                            case 0x20:
                                return (byte)(Opts5Hibyte != 0x00 ? (Device.CodeMemorySize == 0x200000 ? 0x3C : 0x3D) :
                                    (Device.CodeMemorySize == 0x200000 ? 0x44 : 0x45));
                            case 0x30:
                                map = new() { { 0x400000, 0x3E }, { 0x200000, 0x3F }, { 0x100000, 0x40 }, { 0x40000, 0x42 }, { 0x80000, 0x41 } };
                                break;
                            case 0x40:
                                map = new() { { 0x400000, 0x43 }, { 0x200000, 0x44 }, { 0x100000, 0x45 }, { 0x40000, 0x46 }, { 0x80000, 0x46 } };
                                break;
                            case 0x60:
                                map = new() { { 0x400000, 0x47 }, { 0x200000, 0x48 }, { 0x100000, 0x49 } };
                                break;
                        }
                        return (byte)(map.TryGetValue(Device.CodeMemorySize, out byte ret) ? ret : 0x00);
                    }
                    else
                    {
                        return (byte)(Device.CodeMemorySize != 0x200000 ? 0x36 : 0x35);
                    }
                case 0x14:
                    return 0x4D;
                case 0x15:
                    return 0x62;
                case 0x1F:
                case 0x20:
                    return 0x21;
                case 0x1D:
                    if ((byte)Device.PackageDetails == 0x07)
                    {
                        return 0x28;
                    }
                    else if ((Device.PackageDetails & 0xFF000000) == 0xFC000000)
                    {
                        return 0x29;
                    }
                    else if ((Device.Variant & 0xE0) != 0x00)
                    {
                        Dictionary<byte, byte> vlut = new() { { 0x20, 0x24 }, { 0x40, 0x25 }, { 0x60, 0x26 } };

                        if (vlut.ContainsKey((byte)(Device.Variant & 0xE0)))
                        {
                            return vlut[(byte)(Device.Variant & 0xE0)];
                        }
                        else
                        {
                            return (byte)((Device.Variant & 0xE0) == 0x80 ? 0x27 : 0x22);
                        }
                    }
                    else
                    {
                        return 0x20;
                    }
                case 0x1E:
                    return (byte)((Device.Variant & 2 | 0x44) >> 1);
                case 0x21:
                case 0x22:
                case 0x23:
                case 0x24:
                case 0x25:
                case 0x26:
                    return 0x20;
                case 0x18:
                case 0x1A:
                case 0x1B:
                    if ((Device.Variant & 0x0F) == 0x07)
                    {
                        return 0x2C;
                    }
                    else
                    {
                        switch (Device.Variant & 0x0F)
                        {
                            case 0x07:
                                return 0x2C;
                            case 0x06:
                                return 0x2D;
                            case 0x05:
                                return 0x2E;
                            case 0x04:
                                return 0x2F;
                            case 0x03:
                                return 0x30;
                            case 0x01:
                                return 0x2A;
                            case 0x02:
                                return 0x2B;
                            default:
                                return 0x00;
                        }
                    }
                case 0x19:
                    switch (Device.Variant & 0x0F)
                    {
                        case 0x01:
                            return 0x2A;
                        case 0x02:
                            return 0x2B;
                        case 0x03:
                            return 0x2F;
                        case 0x04:
                            return 0x2C;
                        default:
                            return 0x00;
                    }
                case 0x1C:
                    return (byte)((Device.Variant & 0xF) == 0x00 ? 0x31 : 0x32);
                case 0x2A:
                    return 0x33;
                case 0x2B:
                case 0x2C:
                    return 0x34;
                case 0x2D:
                    return (byte)(Device.PackageDetails == 0x08 ? 0x6F : 0x70);
                case 0x30:
                    return 0x72;
            }
            return Opts8Lobyte;
        }
    }
}
