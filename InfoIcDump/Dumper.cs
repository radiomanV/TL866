/*
 * Infoic.dll and InfoIc2Plus.dll dump utility
 * radioman 2013 -2024
 */
using System.Xml;
using System.Xml.Linq;
using System.Text;
using static InfoIcDump.Infoic;


namespace InfoIcDump
{
    internal class Dumper
    {
        private struct CSV_STRUCT
        {
            public uint DeviceID;
            public string Fuses;
        };

        private struct Options
        {
            public bool DumpInfoic;
            public bool DumpInfoic2;
            public bool TL866;
            public bool T48;
            public bool T56;
            public string InfoicPath;
            public string Infoic2Path;
            public string ConfigsPath;
            public string OutPath;
            public bool Memory;
            public bool Mcu;
            public bool Pld;
            public bool Sram;
            public bool Nand;
            public bool Emmc;
            public bool Vga;
            public bool Group;
            public bool RemoveDuplicates;
            public bool SortByType;
        }


        private readonly SortedDictionary<string, CSV_STRUCT> config_csv_list;
        private readonly Infoic infoic;

        public Dumper(string InfoicPath, string Infoic2Path, string ConfigsPath)
        {
            // Open the configs.csv file
            config_csv_list = [];

            try
            {
                using StreamReader stream_reader = new(ConfigsPath);
                string? line;
                CSV_STRUCT csv = new();
                while ((line = stream_reader.ReadLine()) is not null)
                {
                    if (line.Split(';').Length == 3)
                    {
                        csv.DeviceID = Convert.ToUInt32(line.Split(';')[1], 16);
                        csv.Fuses = string.IsNullOrEmpty(line.Split(';')[2]) ? "NULL" : line.Split(';')[2];
                        config_csv_list.Add(line.Split(';')[0], csv);
                    }
                }
                Console.WriteLine("configs.csv loaded. Total entries: {0}", config_csv_list.Count.ToString());
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                Console.WriteLine("configs.csv not found.");
            }

            infoic = new(InfoicPath, Infoic2Path);

        }

        private static void PrintUsage()
        {
            Console.Write(
                "InfoIc Dumper usage:" + Environment.NewLine +
                "--infoic-path  <file>      Specify infoic.dll path" + Environment.NewLine +
                "--infoic2-path <file>      Specify infoic2plus.dll path" + Environment.NewLine +
                "--configs-path <path>      Specify configs.csv file path" + Environment.NewLine +
                "--output-dir  <directory>  Specify the dump output directory" + Environment.NewLine +
                "--keep-duplicates          Don't remove duplicates" + Environment.NewLine +
                "--no-sorting               Don't sort devices by type" + Environment.NewLine +
                "--no-group                 Don't group devices" + Environment.NewLine +
                "--no-infoic                Don't dump infoic.dll" + Environment.NewLine +
                "--no--infoic2              Don't dump infoic2plus.dll" + Environment.NewLine +
                "--tl866-only               Only dump TL866II+ entries" + Environment.NewLine +
                "--t48-only                 Only dump T48 entries" + Environment.NewLine +
                "--t56-only                 Only dump T56 entries" + Environment.NewLine +
                "--no-memory                Don't dump EPROM/EEPROM/FLASH devices" + Environment.NewLine +
                "--no-mcu                   Don't dump MCU/MPU devices" + Environment.NewLine +
                "--no-pld                   Don't dump PLD/CPLD devices" + Environment.NewLine +
                "--no-sram                  Don't dump SRAM/NVRAM devices" + Environment.NewLine +
                "--no--nand                 Don't dump NAND devices" + Environment.NewLine +
                "--no--emmc                 Don't dump EMMC devices" + Environment.NewLine +
                "--no-vga                   Don't dump VGA/HDMI devices" + Environment.NewLine +
                "--help                     Print this help info" + Environment.NewLine + Environment.NewLine +
                "Run InfoicDump without any arguments to dump everything using the default settings." + Environment.NewLine
                );
        }

        private static bool ParseArgs(string[] args, ref Options options)
        {
            for (int i = 0; i < args.Length; i++)
            {
                switch (args[i].ToLower())
                {
                    case "--default":
                        return true;
                    case "--no-infoic":
                        options.DumpInfoic = false;
                        break;

                    case "--no-infoic2":
                        options.DumpInfoic2 = false;
                        break;

                    case "--tl866-only":
                        options.TL866 = true;
                        break;

                    case "--t48-only":
                        options.T48 = true;
                        break;

                    case "--t56-only":
                        options.T56 = true;
                        break;

                    case "--no-memory":
                        options.Memory = false;
                        break;

                    case "--no-mcu":
                        options.Mcu = false;
                        break;

                    case "--no-pld":
                        options.Pld = false;
                        break;

                    case "--no-sram":
                        options.Sram = false;
                        break;

                    case "--no-nand":
                        options.Nand = false;
                        break;

                    case "--no-emmc":
                        options.Emmc = false;
                        break;

                    case "--no-vga":
                        options.Vga = false;
                        break;

                    case "--keep-duplicates":
                        options.RemoveDuplicates = false;
                        break;

                    case "--no-sorting":
                        options.SortByType = false;
                        break;

                    case "--no-group":
                        options.Group = false;
                        break;

                    case "--infoic-path":
                        if (i + 1 < args.Length)
                        {
                            i++;
                            options.InfoicPath = args[i];
                            continue;
                        }
                        return false;

                    case "--infoic2-path":
                        if (i + 1 < args.Length)
                        {
                            i++;
                            options.Infoic2Path = args[i];
                            continue;
                        }
                        return false;

                    case "--configs-path":
                        if (i + 1 < args.Length)
                        {
                            i++;
                            options.ConfigsPath = args[i];
                            continue;
                        }
                        return false;

                    case "--output-dir":
                        if (i + 1 < args.Length)
                        {
                            i++;
                            options.OutPath = args[i];
                            continue;
                        }
                        return false;
                    case "--help":
                        PrintUsage();
                        Environment.Exit(0);
                        break;
                    default:
                        throw new Exception(string.Format("Unknown argument: {0}", args[i]));
                }
            }
            return true;
        }

        static int Main(string[] args)
        {
            Console.CancelKeyPress += new ConsoleCancelEventHandler(delegate (object? sender, ConsoleCancelEventArgs args)
            {
                Console.WriteLine("{0}The dump operation has been interrupted.", Environment.NewLine);
            });

            Console.WriteLine();

            string StartupPath = Path.GetDirectoryName(Environment.GetCommandLineArgs()[0]) + Path.DirectorySeparatorChar;
            Options options = new()
            {
                DumpInfoic = true,
                DumpInfoic2 = true,
                InfoicPath = StartupPath + "InfoIC.dll",
                Infoic2Path = StartupPath + "InfoIC2Plus.dll",
                ConfigsPath = StartupPath + "configs.csv",
                OutPath = StartupPath + "output" + Path.DirectorySeparatorChar,
                TL866 = false,
                T48 = false,
                T56 = false,
                Memory = true,
                Mcu = true,
                Pld = true,
                Sram = true,
                Nand = true,
                Emmc = true,
                Vga = true,
                Group = true,
                SortByType = true,
                RemoveDuplicates = true
            };

            try
            {
                if (!ParseArgs(args, ref options))
                {
                    Console.WriteLine("Invalid syntax." + Environment.NewLine);
                    PrintUsage();
                    return -1;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message + Environment.NewLine);
                PrintUsage();
                return -1;
            }

            if (!options.DumpInfoic && !options.DumpInfoic2)
            {
                Console.WriteLine("Nothing to dump.");
                return -1;
            }

            if (!options.DumpInfoic) { options.InfoicPath = string.Empty; }
            if (!options.DumpInfoic2) { options.Infoic2Path = string.Empty; }

            Dumper dumper = new(options.InfoicPath, options.Infoic2Path, options.ConfigsPath);
            return (dumper.BeginDump(ref options));
        }

        // Start database dump
        private int BeginDump(ref Options options)
        {
            if (options.DumpInfoic)
            {
                if (infoic.InfoIcLoaded)
                {
                    Console.WriteLine("InfoIc.dll loaded. Total devices: {0}", infoic.InfoIcNumDevices);
                }
                else
                {
                    Console.WriteLine("InfoIc.dll not found.");
                }
            }

            if (options.DumpInfoic2)
            {
                if (options.DumpInfoic2 && infoic.InfoIc2Loaded)
                {
                    Console.WriteLine("InfoIc2Plus.dll loaded. Total devices: {0}", infoic.InfoIc2NumDevices);
                }
                else
                {
                    Console.WriteLine("InfoIc2Plus.dll not found.");
                }
            }

            if (!infoic.InfoIcLoaded && !infoic.InfoIc2Loaded)
            {
                Console.WriteLine("{0}No modules to dump, the program will exit now.", Environment.NewLine);
                return -1;
            }

            // create the output directory first
            if (!options.OutPath.EndsWith(Path.DirectorySeparatorChar))
            {
                options.OutPath += Path.DirectorySeparatorChar;
            }

            try
            {
                if (!Directory.Exists(options.OutPath))
                {
                    Directory.CreateDirectory(options.OutPath);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                return -1;
            }

            List<string> filter;
            SortedDictionary<uint, string> total;
            int device_count;

            // Create the root xml
            XElement xml_root = new("infoic");

            // Dump the infoic2plus.dll
            if (infoic.InfoIc2Loaded && options.DumpInfoic2)
            {
                filter = [];
                total = [];
                device_count = 0;
                XElement[] elements = DumpDatabase(ref options, DB_TYPE.INFOIC2, ref filter, ref total, ref device_count);
                if (device_count > 0)
                {
                    XElement database = new("database");
                    database.Add(new XAttribute("type", "INFOIC2PLUS"), elements);
                    xml_root.Add(database);
                }
                if (!WriteLogs(options.OutPath, filter, "filter2.txt", total, "log2.txt", device_count)) { return -1; }
            }

            // Dump the infoic.dll
            if (infoic.InfoIcLoaded && options.DumpInfoic)
            {
                filter = [];
                total = [];
                device_count = 0;
                XElement[] elements = DumpDatabase(ref options, DB_TYPE.INFOIC, ref filter, ref total, ref device_count);
                if (device_count > 0)
                {
                    XElement database = new("database");
                    database.Add(new XAttribute("type", "INFOIC"), elements);
                    xml_root.Add(database);
                }
                if (!WriteLogs(options.OutPath, filter, "filter.txt", total, "log.txt", device_count)) { return -1; }
            }


            // write infoic.xml file
            try
            {
                //write the devices.xml file
                XmlWriterSettings xws = new()
                {
                    Indent = true,
                    NewLineChars = "\n",
                    NewLineOnAttributes = true,
                    CheckCharacters = true
                };

                using XmlWriter xml_writer = XmlWriter.Create(options.OutPath + "infoic.xml", xws);
                xml_root.WriteTo(xml_writer);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                return -1;
            }
            Console.WriteLine("{0}Dump was saved in {1}", Environment.NewLine, options.OutPath);

            return 0;
        }

        // Returns the fuse name if found
        private string GetFuseName(DevStruct devstruct)
        {
            string key = devstruct.Name.Split('@')[0].Trim();
            key = key.Split('(')[0].Trim();

            return config_csv_list.TryGetValue(key, out CSV_STRUCT value) ? value.Fuses : "NULL";
        }

        //Get the device info in xml format
        private static List<XAttribute> GetIcXml(DevStruct devstruct, DB_TYPE type)
        {
            List<XAttribute> xml_chip = [];
            xml_chip.Add(new XAttribute("name", devstruct.Name));
            xml_chip.Add(new XAttribute("type", (byte)devstruct.Category));
            xml_chip.Add(new XAttribute("protocol_id", "0x" + devstruct.ProtocolId.ToString("x2")));
            xml_chip.Add(new XAttribute("variant", "0x" + devstruct.Variant.ToString("x2")));
            xml_chip.Add(new XAttribute("read_buffer_size", "0x" + devstruct.ReadBufferSize.ToString("x2")));
            xml_chip.Add(new XAttribute("write_buffer_size", "0x" + devstruct.WriteBufferSize.ToString("x2")));
            xml_chip.Add(new XAttribute("code_memory_size", "0x" + devstruct.CodeMemorySize.ToString("x2")));
            xml_chip.Add(new XAttribute("data_memory_size", "0x" + devstruct.DataMemorySize.ToString("x2")));
            xml_chip.Add(new XAttribute("data_memory2_size", "0x" + devstruct.DataMemory2Size.ToString("x2")));
            xml_chip.Add(new XAttribute("page_size", "0x" + devstruct.Opts2.ToString("x4")));
            if (type == DB_TYPE.INFOIC2)
            {
                xml_chip.Add(new XAttribute("pages_per_block", "0x" + devstruct.Opts6.ToString("x4")));
            }
            xml_chip.Add(new XAttribute("chip_id", "0x" + devstruct.ChipId.ToString("x8")));
            xml_chip.Add(new XAttribute("voltages", "0x" + (type == DB_TYPE.INFOIC ? devstruct.Opts1 :
            devstruct.Opts5).ToString("x4")));
            xml_chip.Add(new XAttribute("pulse_delay", "0x" + devstruct.Opts3.ToString("x4")));
            xml_chip.Add(new XAttribute("flags", "0x" + devstruct.Opts4.ToString("x8")));
            xml_chip.Add(new XAttribute("chip_info", "0x" + ((byte)devstruct.Opts7).ToString("x4")));
            if (type == DB_TYPE.INFOIC2)
            {
                xml_chip.Add(new XAttribute("pin_map", "0x" + devstruct.Opts8.ToString("x8")));
            }
            xml_chip.Add(new XAttribute("package_details", "0x" + devstruct.PackageDetails.ToString("x8")));
            xml_chip.Add(new XAttribute("config", devstruct.config));
            return xml_chip;
        }

        // Compare two device profiles
        private static bool CompareDevice(DevStruct d1, DevStruct d2)
        {
            return
            d1.Category == d2.Category &&
                d1.ProtocolId == d2.ProtocolId &&
                d1.Variant == d2.Variant &&
                d1.ReadBufferSize == d2.ReadBufferSize &&
                d1.WriteBufferSize == d2.WriteBufferSize &&
                d1.CodeMemorySize == d2.CodeMemorySize &&
                d1.DataMemorySize == d2.DataMemorySize &&
                d1.DataMemory2Size == d2.DataMemory2Size &&
                d1.Opts2 == d2.Opts2 &&
                d1.Opts6 == d2.Opts6 &&
                d1.ChipId == d2.ChipId &&
                d1.Opts5 == d2.Opts5 &&
                d1.Opts3 == d2.Opts3 &&
                d1.Opts4 == d2.Opts4 &&
                d1.Opts7 == d2.Opts7 &&
                d1.Opts8 == d2.Opts8 &&
                d1.PackageDetails == d2.PackageDetails &&
                d1.config == d2.config;
        }

        //Perform the actual dump
        private XElement[] DumpDatabase(ref Options options, DB_TYPE type, ref List<string> filter,
            ref SortedDictionary<uint, string> total, ref int device_count)
        {
            DevStruct Devstruct;
            List<DevStruct> DeviceList = [];
            List<DevStruct> tmp_list;

            total = [];
            filter = [];
            device_count = 0;
            string db_name;
            uint num_mfc;

            if (type == DB_TYPE.INFOIC)
            {
                db_name = "InfoIc.dll";
                num_mfc = infoic.InfoIcManufacturers;
            }
            else
            {
                db_name = "InfoIc2plus.dll";
                num_mfc = infoic.InfoIc2Manufacturers;
            }

            Console.WriteLine("{0}{1} dump started.", Environment.NewLine, db_name);

            //Iterate over the entire manufacturers and add devices to list
            for (uint i = 0; i <= num_mfc; i++)
            {
                //Iterate over the entire devices in the curent manufacturer
                for (uint k = 0; k < infoic.GetMfcDevices(i, type); k++)
                {
                    //Get the device struct
                    Devstruct = infoic.GetDevice(i, k, type);

                    // Skip logic devices defined in old infoic.dll
                    if (Devstruct.Category == (uint)CHIP_TYPE.LOGIC) { continue; }


                    // Filter devices
                    UInt32 ptype = Devstruct.Opts8 & (TL866II_FLAG | T48_FLAG | T56_FLAG);
                    if ((Devstruct.Category == (uint)CHIP_TYPE.MEMORY && !options.Memory) ||
                      (Devstruct.Category == (uint)CHIP_TYPE.MPU && !options.Mcu) ||
                      (Devstruct.Category == (uint)CHIP_TYPE.PLD && !options.Pld) ||
                      (Devstruct.Category == (uint)CHIP_TYPE.SRAM && !options.Sram) ||
                      (Devstruct.Category == (uint)CHIP_TYPE.NAND && !options.Nand) ||
                      (Devstruct.Category == (uint)CHIP_TYPE.EMMC && !options.Emmc) ||
                      (Devstruct.Category == (uint)CHIP_TYPE.VGA && !options.Vga) ||
                      (options.TL866 && ptype != TL866II_FLAG) ||
                      (options.T48 && ptype != T48_FLAG) ||
                      (options.T56 && ptype != T56_FLAG) ||
                      (ptype == 0 && (options.TL866 || options.T48 || options.T56)))
                        continue;


                    //Remove spaces and bad characters
                    Devstruct.Name = Devstruct.Name.Replace(" ", "").Replace(",", ";").
                        Replace("*","").Replace("?", "").Replace("Ł¨", "(");

                    // Insert the device manufacturer
                    Devstruct.Category |= i << 8;

                    // Patch device
                    string key = Devstruct.Name.Split('@')[0];
                    key = key.Split('(')[0];
                    if (config_csv_list.TryGetValue(key, out CSV_STRUCT value))
                    {
                        Devstruct.ChipId = value.DeviceID;
                        Devstruct.Opts4 |= ERASE_FLAG;
                    }
                    else
                    {
                        switch ((byte)Devstruct.Category)
                        {
                            case (byte)CHIP_TYPE.MPU:
                            case (byte)CHIP_TYPE.PLD:
                                filter.Add(Devstruct.Name);
                                break;
                        }
                    }

                    // Patch wrong erase flag in database
                    if (Devstruct.ChipId != 0 && Devstruct.ChipIdBytesCount != 0)
                    {
                        Devstruct.Opts4 |= ERASE_FLAG;
                    }
                    else
                    {
                        Devstruct.Opts4 &= ~ERASE_FLAG;
                    }

                    // Reset the SMT flag in package details
                    // This will allow us to group THT and SMD devices together
                    Devstruct.PackageDetails &= ~(SMT_FLAG | PLCC_FLAG);

                    //Add device to list
                    DeviceList.Add(Devstruct);
                }
            }

            // Remove duplicates
            if (options.RemoveDuplicates)
            {
                int count = DeviceList.Count;
                Console.Write("Removing duplicates.. ");
                DeviceList = DeviceList.Distinct().ToList();
                Console.WriteLine("{0} duplicates removed.", count - DeviceList.Count);
            }

            //Sort the list by Category
            if (options.SortByType)
            {
                tmp_list = [];
                Console.Write("Sorting by type.. ");
                for (uint i = 1; i < 9; i++)
                {
                    foreach (DevStruct d in DeviceList)
                    {
                        if ((byte)d.Category == i)
                            tmp_list.Add(d);
                    }
                }
                Console.WriteLine("{0} manufacturers sorted.", num_mfc);
                DeviceList = tmp_list;
            }

            // Get fuse name for each chip before comapacting and log devices
            for (int i = 0; i < DeviceList.Count; i++)
            {
                Devstruct = DeviceList[i];
                Devstruct.config = GetFuseName(Devstruct);
                DeviceList[i] = Devstruct;

                //Log the device
                if (total.ContainsKey(Devstruct.ProtocolId))
                    total[Devstruct.ProtocolId] += Devstruct.Name + Environment.NewLine;
                else
                    total.Add(Devstruct.ProtocolId, Devstruct.Name + Environment.NewLine);
            }

            // save device count for later
            device_count = DeviceList.Count;

            // Compact database
            if (options.Group)
            {
                Console.Write("Compacting database.. ");
                tmp_list = [];
                StringBuilder sb = new();
                bool flag;

                foreach (DevStruct d in DeviceList)
                {
                    flag = false;
                    for (int i = 0; i < tmp_list.Count; i++)
                    {
                        if (CompareDevice(d, tmp_list[i]))
                        {
                            Devstruct = tmp_list[i];
                            sb.Clear();
                            sb.Append(Devstruct.Name).Append(',').Append(d.Name);
                            Devstruct.Name = sb.ToString();
                            tmp_list[i] = Devstruct;
                            flag = true;
                            break;
                        }
                    }
                    if (tmp_list.Count == 0 || !flag) tmp_list.Add(d);
                }
                Console.WriteLine("{0} profiles created.", tmp_list.Count);
                DeviceList = tmp_list;
            }

            SortedDictionary<uint, XElement> category = [];
            foreach (DevStruct device in DeviceList)
            {
                XElement ic = new("ic");
                ic.Add(GetIcXml(device, type));
                if (category.ContainsKey(device.Category >> 8))
                {
                    category[device.Category >> 8].Add(ic);
                }
                else
                {
                    XElement devices = new("manufacturer");
                    XAttribute manufacturer = new("name", infoic.GetManufName(device.Category >> 8, type));
                    devices.Add(manufacturer);
                    devices.Add(ic);
                    category.Add(device.Category >> 8, devices);
                }
            }

            Console.WriteLine("{0} devices created.", device_count);
            return [.. category.Values];
        }

        // Write log files
        private static bool WriteLogs(string OutPath, List<string> filter, string filter_file,
            SortedDictionary<uint, string> total, string total_file, int device_count)
        {
            try
            {
                // write InfoIc filter file
                using (StreamWriter stream_writer = new(OutPath + filter_file))
                {
                    filter.ForEach(stream_writer.WriteLine);
                }

                //write InfoIc log file
                using (StreamWriter stream_writer = new(OutPath + total_file))
                {
                    foreach (KeyValuePair<uint, string> key in total)
                    {
                        stream_writer.WriteLine("Protocol:0x{0}{1}{2}", key.Key.ToString("X2"), Environment.NewLine, key.Value);
                    }
                    stream_writer.Write("{0}{1} devices in {2} protocols.", Environment.NewLine, device_count, total.Count);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                return false;
            }
            return true;
        }
    }
}

