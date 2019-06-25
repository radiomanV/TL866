@ stdcall -private SetupDiGetClassDevsW(ptr ptr ptr long)
@ stdcall -private SetupDiEnumDeviceInfo(ptr long ptr)
@ stdcall -private SetupDiGetDevicePropertyW(ptr ptr ptr ptr ptr long ptr long)
@ stdcall -private SetupDiDestroyDeviceInfoList(ptr)
@ stdcall -private SetupDiCreateDeviceInfoList(ptr ptr)
@ stdcall -private SetupDiSetDevicePropertyW(ptr ptr ptr ptr ptr long long)
@ stdcall -private SetupDiCreateDeviceInfoW(ptr ptr ptr ptr ptr long ptr)
@ stdcall -private SetupDiOpenDeviceInfoW(ptr ptr ptr long ptr)
@ stdcall -private SetupDiRegisterDeviceInfo(ptr ptr long ptr ptr ptr)
@ stdcall -private SetupDiSetDeviceRegistryPropertyW(ptr ptr long ptr long)
@ stdcall -private SetupDiCreateDevRegKeyW(ptr ptr long long long ptr ptr)
@ stdcall -private SetupDiRemoveDevice(ptr ptr)


