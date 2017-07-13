using System;

namespace TL866
{
    public static class Utils
    {
        public const string NO_DEVICE = "No device detected!\nPlease connect one and try again.";
        public const string NO_FIRMWARE = "No firmware file loaded!\nPlease load the update.dat file.";
        public const string MULTIPLE_DEVICES = "Multiple devices detected!\nPlease connect only one device.";

        public const string WARNING_REFLASH =
            "Warning! this operation will reflash the device.\nDo you want to continue?";

        public const string WARNING_BRICK = "Warning! this operation can brick your device.\nDo you want to continue?";

        public static Random Generator = new Random();
    }
}