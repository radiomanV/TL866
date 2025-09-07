/*
 * Infoic.dll, InfoIc2Plus.dll and infoict76.dll dump utility
 * radiomanV 2013 -2025
 */
using System.Xml;
using System.Xml.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Buffers;
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
            public bool DumpInfoic76;
            public bool TL866;
            public bool T48;
            public bool T56;
            public bool T76;
            public string InfoicPath;
            public string Infoic2Path;
            public string Infoic76Path;
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

        public Dumper(string InfoicPath, string Infoic2Path, string Infoic76Path, string ConfigsPath)
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
                Console.WriteLine("configs.csv loaded. Total entries:{0}", config_csv_list.Count.ToString());
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                Console.WriteLine("configs.csv not found.");
            }

            infoic = new(InfoicPath, Infoic2Path, Infoic76Path);

        }

        private static void PrintUsage()
        {
            Console.Write(
                "InfoIc Dumper usage:" + Environment.NewLine +
                "--infoic-path  <file>      Specify infoic.dll path" + Environment.NewLine +
                "--infoic2-path <file>      Specify infoic2plus.dll path" + Environment.NewLine +
                "--infoic76-path <file>     Specify infoic76.dll path" + Environment.NewLine +
                "--configs-path <file>      Specify configs.csv file path" + Environment.NewLine +
                "--output-dir  <directory>  Specify the dump output directory" + Environment.NewLine +
                "--keep-duplicates          Don't remove duplicates" + Environment.NewLine +
                "--no-sorting               Don't sort devices by type" + Environment.NewLine +
                "--no-group                 Don't group devices" + Environment.NewLine +
                "--no-infoic                Don't dump infoic.dll" + Environment.NewLine +
                "--no-infoic2               Don't dump infoic76.dll" + Environment.NewLine +
                "--no-infoic76              Don't dump infoic2plus.dll" + Environment.NewLine +
                "--no-tl866                 Don't dump TL866II+ entries" + Environment.NewLine +
                "--no-T48                   Don't dump T48 entries" + Environment.NewLine +
                "--no-T56                   Don't dump T56 entries" + Environment.NewLine +
                "--no-T76                   Don't dump T76 entries" + Environment.NewLine +
                "--no-memory                Don't dump EPROM/EEPROM/FLASH devices" + Environment.NewLine +
                "--no-mcu                   Don't dump MCU/MPU devices" + Environment.NewLine +
                "--no-pld                   Don't dump PLD/CPLD devices" + Environment.NewLine +
                "--no-sram                  Don't dump SRAM/NVRAM devices" + Environment.NewLine +
                "--no-nand                  Don't dump NAND devices" + Environment.NewLine +
                "--no-emmc                  Don't dump EMMC devices" + Environment.NewLine +
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

                    case "--no-infoic76":
                        options.DumpInfoic76 = false;
                        break;

                    case "--no-tl866":
                        options.TL866 = false;
                        break;

                    case "--no-t48":
                        options.T48 = false;
                        break;

                    case "--no-t56":
                        options.T56 = false;
                        break;

                    case "--no-t76":
                        options.T76 = false;
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

                    case "--infoic76-path":
                        if (i + 1 < args.Length)
                        {
                            i++;
                            options.Infoic76Path = args[i];
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
                DumpInfoic76 = true,
                InfoicPath = StartupPath + "InfoIC.dll",
                Infoic2Path = StartupPath + "InfoIC2Plus.dll",
                Infoic76Path = StartupPath + "InfoICT76.dll",
                ConfigsPath = StartupPath + "configs.csv",
                OutPath = StartupPath + "output" + Path.DirectorySeparatorChar,
                TL866 = true,
                T48 = true,
                T56 = true,
                T76 = true,
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

            if (!options.DumpInfoic && !options.DumpInfoic2 && !options.DumpInfoic76)
            {
                Console.WriteLine("Nothing to dump.");
                return -1;
            }

            if (!options.DumpInfoic) { options.InfoicPath = string.Empty; }
            if (!options.DumpInfoic2) { options.Infoic2Path = string.Empty; }
            if (!options.DumpInfoic76) { options.Infoic76Path = string.Empty; }

            Dumper dumper = new(options.InfoicPath, options.Infoic2Path, options.Infoic76Path, options.ConfigsPath);
            return (dumper.BeginDump(ref options));
        }

        // Start database dump
        private int BeginDump(ref Options options)
        {
            if (options.DumpInfoic)
            {
                if (infoic.InfoIcLoaded)
                {
                    Console.WriteLine("InfoIc.dll loaded. Total devices:{0}", infoic.InfoIcNumDevices);
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
                    Console.WriteLine("InfoIc2Plus.dll loaded. Total devices:{0}", infoic.InfoIc2NumDevices);
                }
                else
                {
                    Console.WriteLine("InfoIc2Plus.dll not found.");
                }
            }

            if (options.DumpInfoic76)
            {
                if (options.DumpInfoic76 && infoic.InfoIc76Loaded)
                {
                    Console.WriteLine("InfoIc76.dll loaded. Total devices:{0}", infoic.InfoIc76NumDevices);
                }
                else
                {
                    Console.WriteLine("InfoIc76.dll not found.");
                }
            }

            if (!infoic.InfoIcLoaded && !infoic.InfoIc2Loaded && !infoic.InfoIc76Loaded)
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

            // Dump the infoic76.dll
            if (infoic.InfoIc76Loaded && options.DumpInfoic76)
            {
                filter = [];
                total = [];
                device_count = 0;
                XElement[] elements = DumpDatabase(ref options, DB_TYPE.INFOIC76, ref filter, ref total, ref device_count);
                XElement database = new("database");
                database.Add(new XAttribute("type", "INFOICT76"), elements);
                xml_root.Add(database);
                if (!WriteLogs(options.OutPath, filter, "filter76.txt", total, "log76.txt", device_count)) { return -1; }
            }

            // Dump the infoic2plus.dll
            if (infoic.InfoIc2Loaded && options.DumpInfoic2)
            {
                filter = [];
                total = [];
                device_count = 0;
                XElement[] elements = DumpDatabase(ref options, DB_TYPE.INFOIC2, ref filter, ref total, ref device_count);
                XElement database = new("database");
                database.Add(new XAttribute("type", "INFOIC2PLUS"), elements);
                xml_root.Add(database);
                if (!WriteLogs(options.OutPath, filter, "filter2.txt", total, "log2.txt", device_count)) { return -1; }
            }

            // Dump the infoic.dll
            if (infoic.InfoIcLoaded && options.DumpInfoic)
            {
                filter = [];
                total = [];
                device_count = 0;
                XElement[] elements = DumpDatabase(ref options, DB_TYPE.INFOIC, ref filter, ref total, ref device_count);
                XElement database = new("database");
                database.Add(new XAttribute("type", "INFOIC"), elements);
                xml_root.Add(database);
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
        private List<XAttribute> GetIcXml(DevStruct devstruct, DB_TYPE type)
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
            if (type != DB_TYPE.INFOIC)
            {
                xml_chip.Add(new XAttribute("pages_per_block", "0x" + devstruct.Opts6.ToString("x4")));
            }
            xml_chip.Add(new XAttribute("chip_id", "0x" + devstruct.ChipId.ToString("x8")));
            xml_chip.Add(new XAttribute("voltages", "0x" + (type == DB_TYPE.INFOIC ? devstruct.Opts1 :
            devstruct.Opts5).ToString("x4")));
            xml_chip.Add(new XAttribute("pulse_delay", "0x" + devstruct.Opts3.ToString("x4")));
            xml_chip.Add(new XAttribute("flags", "0x" + devstruct.Opts4.ToString("x8")));
            xml_chip.Add(new XAttribute("chip_info", "0x" + ((byte)devstruct.Opts7).ToString("x4")));
            if (type != DB_TYPE.INFOIC)
            {
                xml_chip.Add(new XAttribute("pin_map", "0x" + devstruct.Opts8.ToString("x8")));
            }
            xml_chip.Add(new XAttribute("package_details", "0x" + devstruct.PackageDetails.ToString("x8")));
            xml_chip.Add(new XAttribute("config", devstruct.config));
            return xml_chip;
        }

        // Compare two device profiles
        private static bool CompareDevice(in DevStruct d1, in DevStruct d2) =>
            (d1.Category, d1.ProtocolId, d1.Variant,
             d1.ReadBufferSize, d1.WriteBufferSize,
             d1.CodeMemorySize, d1.DataMemorySize, d1.DataMemory2Size,
             d1.Opts2, d1.Opts6, d1.ChipId, d1.Opts5, d1.Opts3, d1.Opts4, d1.Opts7, d1.Opts8,
             d1.PackageDetails, d1.config)
         ==
            (d2.Category, d2.ProtocolId, d2.Variant,
             d2.ReadBufferSize, d2.WriteBufferSize,
             d2.CodeMemorySize, d2.DataMemorySize, d2.DataMemory2Size,
             d2.Opts2, d2.Opts6, d2.ChipId, d2.Opts5, d2.Opts3, d2.Opts4, d2.Opts7, d2.Opts8,
             d2.PackageDetails, d2.config);


    private static readonly Regex RxMultiWs = new(@"\s+", RegexOptions.Compiled);
    private static readonly Regex RxAtTokens = new(@"@[^,]+", RegexOptions.Compiled);
    private static readonly Regex RxLeadingGarbage = new(@"^[=\s'\""]+", RegexOptions.Compiled);
    private static readonly Regex RxMultiDash = new(@"-{2,}", RegexOptions.Compiled);
    private static readonly Regex RxLetterDashDigit = new(@"(?<=\p{L})-(?=\d)", RegexOptions.Compiled);
    private static readonly Regex RxDashBeforeParen = new(@"-(?=\()", RegexOptions.Compiled);
    private static readonly Regex RxRecommendAny =
        new(@"(?i)\s*\(?\s*(?:recommend(?:ed)?|rec\.?|reco|recom)\s*\)?\s*", RegexOptions.Compiled);
    private static readonly Regex RxCombining = new(@"\p{M}+", RegexOptions.Compiled);
    private static readonly Regex RxLeadingJunk = new(@"^[^A-Za-z0-9(]+", RegexOptions.Compiled);
    private const string VoltChoices = @"(?:1\.(?:2|8)|2\.5|3\.(?:0|3|6)|5\.0)"; 
    private static readonly Regex RxVccList =
        new($@"(?i)(?:\(|_)?\s*VCC\s*=\s*({VoltChoices})\s*v\s*(?:\)|_)?", RegexOptions.Compiled);

    private static readonly Regex RxVoltList =
        new($@"(?i)(?:\(|_)?\s*({VoltChoices})\s*v\s*(?:\)|_)?", RegexOptions.Compiled);


    // Name cleanup
    private string NormalizeName(string s)
        {
            if (string.IsNullOrWhiteSpace(s)) return string.Empty;
            s = s.Trim();
            s = s.Normalize(NormalizationForm.FormKC);
            s = s.Replace('\u00A0', ' ')
                .Replace('\u2007', ' ')
                .Replace('\u202F', ' ');

            foreach (var d in new[] { '–', '—', '−', '­', '‒' }) s = s.Replace(d, '-');

            var pairs = new (string bad, string good)[] {
                ("£¨", "("), ("£©", ")"),
                ("ï¼ˆ", "("), ("ï¼‰", ")"),
                ("【","["), ("】","]"), ("《","<"), ("》",">"),
                ("＂","\""), ("＇","'"), ("，",","), ("．","."),
                ("：",":"), ("；",";"), ("／","/"), ("＼","\\"), ("－","-"),
            };

            foreach (var (bad, good) in pairs) s = s.Replace(bad, good);
            s = s.Replace("£", "(");
            s = RxCombining.Replace(s, "");
            s = s.Replace("Â", "");
            s = RxLeadingJunk.Replace(s, "");
            s = RxLeadingGarbage.Replace(s, "").Trim();
            s = s.Replace("*", "").Replace("?", "");
            s = RxVccList.Replace(s, "($1V)");
            s = RxVoltList.Replace(s, "($1V)");
            var atTokens = RxAtTokens.Matches(s).Cast<Match>().Select(m => m.Value).ToList();
            s = RxAtTokens.Replace(s, " ");
            s = RxMultiWs.Replace(s, " ").Trim();
            s = RxRecommendAny.Replace(s, "(preferred)").Trim();
            s = s.Replace(" ", "-");
            s = RxMultiDash.Replace(s, "-").Trim('-');
            s = RxDashBeforeParen.Replace(s, "");
            s = RxLetterDashDigit.Replace(s, "");
            var pkgs = new List<string>();
            var seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            foreach (var tok in atTokens)
            {
                string pkg = tok.Substring(1);
                pkg = RxVccList.Replace(pkg, "($1V)");
                pkg = RxVoltList.Replace(pkg, "($1V)");
                pkg = RxMultiWs.Replace(pkg, " ").Trim();
                pkg = RxRecommendAny.Replace(pkg, "(preferred)").Trim();
                pkg = pkg.Replace(" ", "-");
                pkg = RxMultiDash.Replace(pkg, "-").Trim('-');
                pkg = RxDashBeforeParen.Replace(pkg, "");
                pkg = RxLetterDashDigit.Replace(pkg, "");

                if (pkg.Length == 0) continue;
                string final = "@" + pkg;
                if (seen.Add(final)) pkgs.Add(final);
            }

            string result = s + string.Concat(pkgs);
            result = result.Replace(" ", "");
            result = RxMultiDash.Replace(result, "-").Trim('-');
            result = result.Replace("-@", "@");
            result = RxDashBeforeParen.Replace(result, "");
            result = result.Replace("-GSTO", "GSTO");
            result = result.Replace("@@", "@");
            result = result.Replace(",", "+");
            result = result.Replace("ADP:Pin9-to1", "Rotated");

            if (result.IndexOf(',') >= 0)
                Console.WriteLine("[WARN] Comma remained in single name: " + result);

            return result;
        }

        static int RemoveDuplicates(List<DevStruct> list)
        {
            if (list == null || list.Count <= 1) return 0;

            var cmp  = new DevStructAllComparer();
            var seen = new HashSet<DevStruct>(list.Count, cmp);

            int write = 0;
            foreach (ref readonly var d in CollectionsMarshal.AsSpan(list))
            {
                if (seen.Add(d)) list[write++] = d;
            }

            int removed = list.Count - write;
            if (removed > 0) list.RemoveRange(write, removed);
            return removed;
        }

        static readonly SearchValues<char> StopChars = SearchValues.Create("@(");

        private string BaseKey(string name)
        {
            if (string.IsNullOrEmpty(name)) return string.Empty;
            int cut = name.AsSpan().IndexOfAny(StopChars);
            return cut <= 0 ? (cut == 0 ? string.Empty : name) : name[..cut];
        }

        static void LogByProtocolSB(SortedDictionary<uint, StringBuilder> acc, uint pid, string name)
        {
            if (!acc.TryGetValue(pid, out var sb))
                acc[pid] = sb = new StringBuilder(256);
            sb.AppendLine(name);
        }

        private (
            uint Category, uint ProtocolId, uint Variant,
            uint ReadBufferSize, uint WriteBufferSize,
            uint CodeMemorySize, uint DataMemorySize, uint DataMemory2Size,
            uint Opts2, uint Opts6, ulong ChipId, uint Opts5, uint Opts3, uint Opts4, uint Opts7, uint Opts8,
            uint PackageDetails, string config
        ) GroupKey(in DevStruct d) => (
            d.Category, d.ProtocolId, d.Variant,
            d.ReadBufferSize, d.WriteBufferSize,
            d.CodeMemorySize, d.DataMemorySize, d.DataMemory2Size,
            d.Opts2, d.Opts6, d.ChipId, d.Opts5, d.Opts3, d.Opts4, d.Opts7, d.Opts8,
            d.PackageDetails, d.config ?? string.Empty
        );

        //Perform the actual dump
        private XElement[] DumpDatabase(ref Options options, DB_TYPE type, ref List<string> filter,
            ref SortedDictionary<uint, string> total, ref int device_count)
        {
            DevStruct Devstruct;
            List<DevStruct> DeviceList = [];

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
            else if (type == DB_TYPE.INFOIC2)
            {
                db_name = "InfoIc2plus.dll";
                num_mfc = infoic.InfoIc2Manufacturers;
            }
            else
            {
                db_name = "InfoIc76.dll";
                num_mfc = infoic.InfoIc76Manufacturers;
            }

            Console.WriteLine("{0}{1} dump started.", Environment.NewLine, db_name);

            bool wantMem  = options.Memory,
                 wantMcu  = options.Mcu,
                 wantPld  = options.Pld,
                 wantSram = options.Sram,
                 wantNand = options.Nand,
                 wantEmmc = options.Emmc,
                 wantVga  = options.Vga;

            var acc = new SortedDictionary<uint, StringBuilder>();

            // Iterate over the entire manufacturers and add devices to list
            for (int i = 0; i < (int)num_mfc; i++)
            {
                int devCount = (int)infoic.GetMfcDevices((uint)i, type);

                for (int k = 0; k < devCount; k++)
                {
                    var d = infoic.GetDevice((uint)i, (uint)k, type);
                    d.Category &= 0xFFu;
                    if (d.Category == (uint)CHIP_TYPE.LOGIC) continue;
                    uint catLow = d.Category;
                    if ((catLow == (uint)CHIP_TYPE.MEMORY && !wantMem)  ||
                        (catLow == (uint)CHIP_TYPE.MPU    && !wantMcu)  ||
                        (catLow == (uint)CHIP_TYPE.PLD    && !wantPld)  ||
                        (catLow == (uint)CHIP_TYPE.SRAM   && !wantSram) ||
                        (catLow == (uint)CHIP_TYPE.NAND   && !wantNand) ||
                        (catLow == (uint)CHIP_TYPE.EMMC   && !wantEmmc) ||
                        (catLow == (uint)CHIP_TYPE.VGA    && !wantVga))
                        continue;

                    // Name cleanup
                    d.Name = NormalizeName(d.Name);
                    d.Category |= (uint)(i << 8);
                    string key = BaseKey(d.Name);
                    if (config_csv_list.TryGetValue(key, out CSV_STRUCT csv))
                    {
                        d.ChipId = csv.DeviceID;
                        d.Opts4 |= ERASE_FLAG;
                    }
                    else
                    {
                        switch ((byte)catLow)
                        {
                            case (byte)CHIP_TYPE.MPU:
                            case (byte)CHIP_TYPE.PLD:
                                filter.Add(d.Name);
                                break;
                        }
                    }

                    // Patch ERASE_FLAG corect
                    if (d.ChipId != 0 && d.ChipIdBytesCount != 0)
                        d.Opts4 |= ERASE_FLAG;
                    else
                        d.Opts4 &= ~ERASE_FLAG;

                    // TL866A/CS PLCC32 mapping fix
                    if (type == DB_TYPE.INFOIC &&
                        d.PackageDetails == 0xA0000000 &&
                        d.ProtocolId == 0x36)
                    {
                        d.PackageDetails = 0xFF000000;
                    }
                    DeviceList.Add(d);
                    LogByProtocolSB(acc, d.ProtocolId, d.Name);
                }
            }

            // Remove duplicates
            if (options.RemoveDuplicates)
            {
                Console.Write("Removing duplicates.. ");
                int removed = RemoveDuplicates(DeviceList);
                Console.WriteLine("{0} duplicates removed.", removed);
            }

            //Sort the list by Category
            if (options.SortByType)
            {
                Console.Write("Sorting by type.. ");

                int n = DeviceList.Count;
                var buckets = new List<DevStruct>[9];
                for (int t = 1; t <= 8; t++)
                    buckets[t] = new List<DevStruct>(n / 8 + 1);

                foreach (ref readonly var d in CollectionsMarshal.AsSpan(DeviceList))
                {
                    int t = (byte)d.Category;
                    if ((uint)(t - 1) < 8)
                        buckets[t]!.Add(d);
                }

                var tmp = new List<DevStruct>(n);
                for (int t = 1; t <= 8; t++)
                    if (buckets[t]!.Count != 0)
                        tmp.AddRange(buckets[t]!);

                DeviceList = tmp;

                Console.WriteLine("{0} manufacturers sorted.", num_mfc);
            }

            // Get fuse name for each chip before compacting
            for (int i = 0; i < DeviceList.Count; i++)
            {
                Devstruct = DeviceList[i];
                Devstruct.config = GetFuseName(Devstruct);
                DeviceList[i] = Devstruct;
            }

            // save device count for later
            device_count = DeviceList.Count;

            // Compact database
        if (options.Group)
        {
            Console.Write("Compacting database.. ");


        var indexByKey = new Dictionary<
            (uint,uint,uint,uint,uint,uint,uint,uint,uint,uint,ulong,uint,uint,
            uint,uint,uint,uint,string), int >(DeviceList.Count);

            var results = new List<DevStruct>(DeviceList.Count);
            var nameAcc = new List<StringBuilder>(DeviceList.Count);

            foreach (var d in DeviceList)
            {
                var key = GroupKey(d);

                if (indexByKey.TryGetValue(key, out int idx))
                {
                    var sb = nameAcc[idx];
                    if (sb.Length > 0) sb.Append(',');
                    sb.Append(d.Name);
                }
                else
                {
                    indexByKey[key] = results.Count;
                    results.Add(d);
                    nameAcc.Add(new StringBuilder(d.Name.Length + 16).Append(d.Name));
                }
            }

            var span = CollectionsMarshal.AsSpan(results);
            for (int i = 0; i < span.Length; i++)
            {
                span[i].Name = nameAcc[i].ToString();
            }

            Console.WriteLine("{0} profiles created.", results.Count);
            DeviceList = results;
        }

        SortedDictionary<uint, XElement> category = new();
        foreach (var device in DeviceList)
        {
            uint mId = device.Category >> 8;
            if (!category.TryGetValue(mId, out var man))
            {
                man = new XElement("manufacturer",
                        new XAttribute("name", infoic.GetManufName(mId, type)));
                category.Add(mId, man);
            }
            var ic = new XElement("ic");
            ic.Add(GetIcXml(device, type));
            man.Add(ic);
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

    internal class DevStructAllComparer : IEqualityComparer<DevStruct>
    {
        public bool Equals(DevStruct x, DevStruct y)
        {
            return
                x.ProtocolId       == y.ProtocolId &&
                x.Opts8            == y.Opts8 &&
                x.Category         == y.Category &&
                x.Variant          == y.Variant &&
                x.CodeMemorySize   == y.CodeMemorySize &&
                x.DataMemorySize   == y.DataMemorySize &&
                x.DataMemory2Size  == y.DataMemory2Size &&
                x.Opts7            == y.Opts7 &&
                x.ReadBufferSize   == y.ReadBufferSize &&
                x.WriteBufferSize  == y.WriteBufferSize &&
                x.Opts1            == y.Opts1 &&
                x.Opts2            == y.Opts2 &&
                x.Opts3            == y.Opts3 &&
                x.ChipId           == y.ChipId &&
                x.Opts5            == y.Opts5 &&
                x.ChipIdBytesCount == y.ChipIdBytesCount &&
                x.Opts6            == y.Opts6 &&
                x.PackageDetails   == y.PackageDetails &&
                x.Opts4            == y.Opts4 &&
                string.Equals(x.Name,   y.Name,   StringComparison.Ordinal) &&
                string.Equals(x.config, y.config, StringComparison.Ordinal);
        }

        public int GetHashCode(DevStruct d)
        {
            var h = new HashCode();
            h.Add(d.ProtocolId);
            h.Add(d.Opts8);
            h.Add(d.Category);
            h.Add(d.Variant);
            h.Add(d.CodeMemorySize);
            h.Add(d.DataMemorySize);
            h.Add(d.DataMemory2Size);
            h.Add(d.Opts7);
            h.Add(d.ReadBufferSize);
            h.Add(d.WriteBufferSize);
            h.Add(d.Opts1);
            h.Add(d.Opts2);
            h.Add(d.Opts3);
            h.Add(d.ChipId);
            h.Add(d.Opts5);
            h.Add(d.ChipIdBytesCount);
            h.Add(d.Opts6);
            h.Add(d.PackageDetails);
            h.Add(d.Opts4);
            h.Add(d.Name,   StringComparer.Ordinal);
            h.Add(d.config, StringComparer.Ordinal);
            return h.ToHashCode();
        }
    }
}
