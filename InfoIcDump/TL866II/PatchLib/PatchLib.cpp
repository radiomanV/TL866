#include <Windows.h>
#include <stdint.h>


extern "C"
{
#pragma pack(1)
	typedef struct {
		uint32_t protocol_id;
		uint32_t opts8;
		uint32_t category;
		char name[40];
		uint32_t variant;
		uint32_t code_memory_size;
		uint32_t data_memory_size;
		uint32_t data_memory2_size;
		uint32_t opts7;
		uint16_t read_buffer_size;
		uint16_t write_buffer_size;
		uint32_t opts5;
		uint32_t opts1;
		uint32_t opts2;
		uint32_t opts3;
		uint64_t chip_id;
		uint32_t chip_id_bytes_count;
		uint32_t opts6;
		uint32_t package_details;
		uint32_t opts4;
	} devstruct_t;

	uint8_t patch_hi8(devstruct_t *device);
	uint8_t patch_lo8(devstruct_t *device);
	void patch_package(devstruct_t *device);


	__declspec(dllexport) void Patch_Device(devstruct_t *device)
	{
		//Patch package details and opts4
		patch_package(device);

		//Patch opts5
		if(device->opts7 == 0x06 && device->opts5 >= 0xF0)
			device->opts5 = 0xF0;
		device->opts5 = (device->opts5 & 0xFFFF00FF) | ((uint32_t)device->opts1 << 8);

		//Patch opts8
		device->opts8 = (device->opts8 & 0x00FF) | (uint32_t)(patch_hi8(device) << 8);
		device->opts8 = (device->opts8 & 0xFF00) | patch_lo8(device);
	}


	//Patch package_details and opts4
	void patch_package(devstruct_t *device)
	{
		if ( device->protocol_id == 0x2D ) return;			
		if ( device->protocol_id == 3 )
		{
			device->opts4 |= 0x100048;
			if ((device->package_details & 0xFF00) == 0x500 ) return;		
			device->package_details |= 0x900;
			return;
		}
		else
		{
			if ( device->protocol_id == 2 )
			{
				if ( device->variant & 0x20 ) return;				
				device->opts4 |= 0x100000;
				device->package_details |= 0xA00;
				return;
			}
			else
			{
				if ( device->protocol_id != 1 || (uint8_t)device->opts1 ) return;				
				device->opts4 |= 0x100000;
				device->package_details |= 0xB00;
			}
		}
	}


	//Patch opts8 hibyte
	uint8_t patch_hi8(devstruct_t *device)
	{
		uint32_t v2;
		uint8_t v3; 
		uint32_t v4; 
		uint32_t v5; 
		uint8_t v6; 
		uint8_t v7; 
		uint32_t v8; 
		uint32_t v9; 
		uint8_t v10; 
		uint32_t v11; 
		uint8_t v12; 
		uint32_t v13; 
		uint32_t v14;


		uint8_t opts8_hibyte = (uint8_t)(device->opts8 >> 8);
		uint16_t opts5_lo_word = (uint16_t) device->opts5;

		if ( opts8_hibyte )
		{
			v7 = 0;
			v3 = opts8_hibyte;
			v6 = (uint8_t)opts8_hibyte == 0xFF;
LABEL_262:
			if ( v6 )
				v3 = v7;
LABEL_264:
			opts8_hibyte = v3;
		}
		else
		{
			switch ( device->protocol_id )
			{
			case 5u:
				if ( device->package_details == 0x20000000
					|| device->package_details == 0xA0000000
					|| device->package_details == 0xFF000000 )
				{
					v4 = device->code_memory_size;
					if ( device->code_memory_size == 0x10000 )
					{
						opts8_hibyte = 0x17;
						break;
					}
					if ( device->code_memory_size == 0x20000 )
					{
						opts8_hibyte = 0x18;
						break;
					}
					if ( device->code_memory_size == 0x40000 )
					{
						opts8_hibyte = 0x19;
						break;
					}
					v4 = device->code_memory_size == 0x80000;
					v3 = 4 * v4 + 0x16;
				}
				else
				{
					v2 = device->code_memory_size;
					if ( device->code_memory_size == 0x10000 )
					{
						opts8_hibyte = 0x1C;
						break;
					}
					if ( device->code_memory_size == 0x20000 )
					{
						opts8_hibyte = 0x1D;
						break;
					}
					if ( device->code_memory_size == 0x40000 )
					{
						opts8_hibyte = 0x1E;
						break;
					}
					v2 = device->code_memory_size == 0x80000;
					v3 = 4 * v2 + 0x1B;
				}
				goto LABEL_264;
			case 6u:
				v5 = device->package_details;
				if ( device->variant & 0x80 )
					goto LABEL_22;
				if ( device->package_details == 0x20000000
					|| device->package_details == 0xA0000000
					|| device->package_details == 0xFF000000 )
				{
					v9 = device->code_memory_size;
					if ( device->code_memory_size == 0x10000 )
					{
						opts8_hibyte =  0x59;
						break;
					}
					if ( device->code_memory_size == 0x20000 )
					{
						opts8_hibyte =  0x5A;
						break;
					}
					if ( device->code_memory_size == 0x40000 )
					{
						opts8_hibyte =  0x5B;
						break;
					}
					v9 = device->code_memory_size == 0x80000;
					v3 = 4 * v9 + 88;
				}
				else
				{
					v8 = device->code_memory_size;
					if ( device->code_memory_size == 0x10000 )
					{
						opts8_hibyte =  0x62;
						break;
					}
					if ( device->code_memory_size == 0x20000 )
					{
						opts8_hibyte =  0x63;
						break;
					}
					if ( device->code_memory_size == 0x40000 )
					{
						opts8_hibyte =  0x64;
						break;
					}
					v8 = device->code_memory_size == 0x80000;
					v3 = 4 * v8 + 0x61;
				}
				goto LABEL_264;
			case 3u:
			case 0xFu:
				if ( (device->variant & 0xF0) != 0x20 )
					goto LABEL_156;
				opts8_hibyte =  0x15;
				break;
			case 0x11u:
				if ( HIBYTE(opts5_lo_word) == 1 )
				{
					opts8_hibyte =  0x6E;
				}
				else if ( device->variant & 0x80 )
				{
					if ( device->package_details == 0x20000000
						|| device->package_details == 0xA0000000
						|| device->package_details == 0xFF000000 )
					{
						opts8_hibyte =  0x6A;
					}
					else
					{
						opts8_hibyte =  0x6C;
					}
				}
				else if ( device->package_details == 0x20000000
					|| device->package_details == 0xA0000000
					|| device->package_details == 0xFF000000 )
				{
					opts8_hibyte =  0x6B;
				}
				else
				{
					opts8_hibyte =  0x6D;
				}
				break;
			case 0x10u:
				if ( device->variant == 0x10 || device->variant == 0x11 || device->variant == 0x12 )
				{
					opts8_hibyte =  0xD;
					break;
				}
				v5 = device->package_details;
LABEL_22:
				if ( v5 == 0x20000000 || v5 == 0xA0000000 || v5 == 0xFF000000 )
				{
					if ( device->code_memory_size == 0x10000 )
					{
						opts8_hibyte =  0x5E;
						break;
					}
					if ( device->code_memory_size == 0x20000 )
					{
						opts8_hibyte =  0x5F;
						break;
					}
					v3 = 0x5D;
					v6 = device->code_memory_size == 0x40000;
					v7 = 0x60;
				}
				else
				{
					if ( device->code_memory_size == 0x10000 )
						goto LABEL_26;
					if ( device->code_memory_size == 0x20000 )
					{
						opts8_hibyte =  0x68;
						break;
					}
					v3 = 0x66;
					v6 = device->code_memory_size == 0x40000;
					v7 = 0x69;
				}
				goto LABEL_262;
			case 8u:
				if ( device->variant != 4 )
				{
					if ( device->package_details == 5 )
					{
						if ( device->code_memory_size == 0x10000 )
						{
LABEL_26:
							opts8_hibyte =  0x67;
						}
						else if ( device->code_memory_size == 0x20000 )
						{
							opts8_hibyte =  0x68;
						}
						else
						{
							opts8_hibyte =  0x69;
						}
					}
					else if ( device->code_memory_size == 0x10000 )
					{
						opts8_hibyte =  0x5E;
					}
					else if ( device->code_memory_size == 0x20000 )
					{
						opts8_hibyte =  0x5F;
					}
					else
					{
						opts8_hibyte =  0x60;
					}
					break;
				}
				if ( device->package_details == 5 )
				{
					if ( device->code_memory_size == 0x10000 )
					{
						opts8_hibyte =  0x62;
						break;
					}
					if ( device->code_memory_size == 0x20000 )
					{
						opts8_hibyte =  0x63;
						break;
					}
					v3 = (device->code_memory_size != 0x40000) + 0x64;
				}
				else
				{
					if ( device->code_memory_size == 0x10000 )
					{
						opts8_hibyte =  0x59;
						break;
					}
					if ( device->code_memory_size == 0x20000 )
					{
						opts8_hibyte =  0x5A;
						break;
					}
					v3 = (device->code_memory_size != 0x40000) + 0x5B;
				}
				goto LABEL_264;
			case 7u:
				if ( !(device->variant & 0x20) )
				{
					v3 = 12;
					if ( device->code_memory_size == 0x2000 )
						v3 = 111;
					goto LABEL_264;
				}
				if ( device->code_memory_size == 2048 )
				{
					opts8_hibyte =  0x71;
					break;
				}
				if ( device->code_memory_size == 0x2000 )
				{
					opts8_hibyte =  0x70;
					break;
				}
				goto LABEL_137;
			case 0xAu:
				if ( (device->variant & 0x80u) != 0 )
				{
					opts8_hibyte =  0x75;
					break;
				}
				v3 = (device->variant != 0) + 0x76;
				goto LABEL_264;
			case 0xBu:
			case 0x2Bu:
			case 0x2Cu:
				opts8_hibyte =  0xB;
				break;
			case 9u:
				if ( device->package_details == 4 )
				{
					v3 = 0xF;
					if ( device->code_memory_size == 0x20000 )
						v3 =  0x79;
				}
				else
				{
					if ( device->package_details == 0xFD000000 )
					{
						v3 =  (device->code_memory_size != 0x20000) + 0x7A;
						goto LABEL_264;
					}
					if ( device->package_details == 0x2A000000 )
					{
						v3 =  0x10;
						if ( device->code_memory_size == 0x100000 )
							v3 =  0x7C;
					}
					else
					{
						if ( device->package_details == 2 )
						{
							opts8_hibyte =  0x7C;
							break;
						}
						v3 =  0xF;
						if ( device->code_memory_size == 0x20000 )
							v3 =  0x78;
					}
				}
				goto LABEL_264;
			case 0xDu:
				if ( device->package_details == 5 )
				{
					if ( device->code_memory_size == 0x20000 )
					{
						opts8_hibyte =  0x63;
						break;
					}
					v3 =  0xD;
					v6 = device->code_memory_size == 0x40000;
					v7 = 0x64;
				}
				else
				{
					if ( device->code_memory_size == 0x20000 )
					{
						opts8_hibyte =  0x5A;
						break;
					}
					v3 =  0xD;
					v6 = device->code_memory_size == 0x40000;
					v7 = 0x5B;
				}
				goto LABEL_262;
			case 0x21u:
			case 0x22u:
			case 0x23u:
			case 0x24u:
			case 0x25u:
			case 0x26u:
				opts8_hibyte =  0x11;
				break;
			case 0x1Eu:
				v3 =  (device->variant & 2) != 0 ? 6 : 3;
				goto LABEL_264;
			case 0x1Du:
				if ( (uint8_t)device->package_details == 7 )
				{
					opts8_hibyte =  0x14;
					break;
				}
				v10 = device->variant & 0xE0;
				if ( !(device->variant & 0xE0) )
				{
					opts8_hibyte =  0xF;
					break;
				}
				if ( v10 == 0x20 )
				{
					opts8_hibyte =  0xF;
					break;
				}
				if ( v10 == 0x40 )
				{
LABEL_137:
					opts8_hibyte =  0xC;
					break;
				}
				if ( v10 == 0x60 )
					goto LABEL_139;
				v3 =  3;
				v6 = v10 == 0x80u;
				v7 = 9;
				goto LABEL_262;
			case 0x1Fu:
			case 0x20u:
			case 0x2Au:
				goto LABEL_139;
			case 0x18u:
			case 0x19u:
			case 0x1Au:
			case 0x1Bu:
				v11 = device->package_details & 0xFF000000;
				if ( (device->package_details & 0xFF000000) <= 0x1C000000 )
				{
					if ( (device->package_details & 0xFF000000) != 0x1C000000 )
					{
						if ( v11 <= 0x12000000 )
						{
							if ( v11 != 0x12000000 )
							{
								if ( v11 != 0x8000000 )
								{
									if ( v11 == 0xE000000 )
									{
LABEL_147:
										opts8_hibyte =  6;
										break;
									}
									goto LABEL_168;
								}
LABEL_156:
								opts8_hibyte =  3;
								break;
							}
							goto LABEL_159;
						}
						if ( v11 != 0x14000000 )
						{
							opts8_hibyte =  0;
							break;
						}
LABEL_139:
						opts8_hibyte =  9;
						break;
					}
LABEL_165:
					opts8_hibyte =  0x37;
					break;
				}
				if ( v11 <= 0x94000000 )
				{
					if ( v11 == 0x94000000 )
						goto LABEL_139;
					if ( v11 <= 0x8E000000 )
					{
						if ( v11 == 0x8E000000 )
							goto LABEL_147;
						if ( v11 == 0x28000000 )
						{
							opts8_hibyte =  0x39;
							break;
						}
						if ( v11 != 0x88000000 )
							goto LABEL_168;
						goto LABEL_156;
					}
					if ( v11 == 0x92000000 )
					{
LABEL_159:
						opts8_hibyte =  8;
						break;
					}
					goto LABEL_168;
				}
				if ( v11 <= 0xFA000000 )
				{
					if ( v11 == 0xFA000000 )
					{
						opts8_hibyte =  0x3B;
						break;
					}
					if ( v11 != 0x9C000000 )
					{
						if ( v11 == 0xF9000000 )
						{
							opts8_hibyte =  0x3A;
							break;
						}
						goto LABEL_168;
					}
					goto LABEL_165;
				}
				if ( v11 != 0xFB000000 )
				{
LABEL_168:
					opts8_hibyte =  0;
					break;
				}
				opts8_hibyte =  0x38;
				break;
			case 4u:
				goto LABEL_156;
			case 0x1Cu:
				v3 = 4 * (device->protocol_id - 1) + 8;
				goto LABEL_264;
			case 0x13u:
				v12 = device->variant & 0xF0;
				if ( !HIBYTE(opts5_lo_word) )
				{
					if ( v12 == 0x20 || v12 == 0x40 )
					{
						if ( device->code_memory_size == 0x100000 )
						{
							opts8_hibyte =  0x55;
							break;
						}
						if ( device->code_memory_size == 0x200000 )
						{
							opts8_hibyte =  0x56;
							break;
						}
						v3 =  0x54;
						v6 = device->code_memory_size == 0x400000;
						v7 = 0x57;
					}
					else
					{
						if ( device->code_memory_size == 0x100000 )
						{
							opts8_hibyte =  0x51;
							break;
						}
						if ( device->code_memory_size == 0x200000 )
						{
							opts8_hibyte =  0x52;
							break;
						}
						v3 =  0x50;
						v6 = device->code_memory_size == 0x400000;
						v7 = 0x53;
					}
					goto LABEL_262;
				}
				if ( v12 == 0x10 )
				{
					v13 = device->code_memory_size;
					if ( device->code_memory_size == 0x80000 )
					{
						opts8_hibyte =  0x47;
						break;
					}
					if ( device->code_memory_size == 0x100000 )
					{
						opts8_hibyte =  0x48;
						break;
					}
					if ( device->code_memory_size == 0x200000 )
					{
						opts8_hibyte =  0x49;
						break;
					}
					v13 = device->code_memory_size == 0x400000;
					v3 = 4 * v13 + 0x46;
				}
				else
				{
					v6 = v12 == 0x30;
					v14 = device->code_memory_size;
					if ( v6 )
					{
						if ( device->code_memory_size == 0x80000 )
						{
							opts8_hibyte =  0x4C;
							break;
						}
						if ( device->code_memory_size == 0x100000 )
						{
							opts8_hibyte =  0x4D;
							break;
						}
						if ( device->code_memory_size == 0x200000 )
						{
							opts8_hibyte =  0x4E;
							break;
						}
						v14 = device->code_memory_size == 0x400000;
						v3 = 4 * v14 + 0x4B;
					}
					else
					{
						if ( device->code_memory_size == 0x80000 )
						{
							opts8_hibyte =  0x42;
							break;
						}
						if ( device->code_memory_size == 0x100000 )
						{
							opts8_hibyte =  0x43;
							break;
						}
						if ( device->code_memory_size == 0x200000 )
						{
							opts8_hibyte =  0x44;
							break;
						}
						v14 = device->code_memory_size == 0x400000;
						v3 = 4 * v14 + 0x41;
					}
				}
				goto LABEL_264;
			case 0x14u:
				opts8_hibyte =  0x3F;
				break;
			case 0x12u:
				if ( (uint8_t)device->package_details == 2 )
				{
					if ( device->code_memory_size <= 0x80000 )
					{
						if ( device->code_memory_size == 0x80000 )
						{
							opts8_hibyte =  0x2C;
							break;
						}
						if ( device->code_memory_size != 0x20000 && device->code_memory_size == 0x40000 )
						{
							opts8_hibyte =  0x2B;
							break;
						}
						goto LABEL_217;
					}
					if ( device->code_memory_size == 0x100000 )
					{
						opts8_hibyte =  0x2D;
					}
					else
					{
						if ( device->code_memory_size != 0x200000 )
						{
LABEL_217:
							opts8_hibyte =  0x2A;
							break;
						}
						opts8_hibyte =  0x2E;
					}
				}
				else
				{
					if ( (uint8_t)device->package_details == 1 )
					{
						if ( device->code_memory_size > 0x100000 )
						{
							if ( device->code_memory_size == 0x200000 )
							{
								opts8_hibyte =  0x27;
								break;
							}
							if ( device->code_memory_size == 0x400000 )
							{
								opts8_hibyte =  0x28;
								break;
							}
							if ( device->code_memory_size == 0x800000 )
							{
								opts8_hibyte =  0x29;
								break;
							}
						}
						else
						{
							if ( device->code_memory_size == 0x100000 )
							{
								opts8_hibyte =  0x26;
								break;
							}
							if ( device->code_memory_size != 0x20000 )
							{
								if ( device->code_memory_size == 0x40000 )
								{
									opts8_hibyte =  0x24;
									break;
								}
								if ( device->code_memory_size == 0x80000 )
								{
									opts8_hibyte =  0x25;
									break;
								}
							}
						}
						opts8_hibyte =  0x23;
						break;
					}
					if ( (uint8_t)device->package_details == 0xB )
					{
						opts8_hibyte =  0x14;
						break;
					}
					if ( (device->package_details & 0xFF000000) == 0xE1000000 )
					{
						if ( device->code_memory_size > 0x100000 )
						{
							if ( device->code_memory_size == 0x200000 )
							{
								opts8_hibyte =  0x33;
								break;
							}
							if ( device->code_memory_size == 0x400000 )
							{
								opts8_hibyte =  0x34;
								break;
							}
							if ( device->code_memory_size == 0x800000 )
							{
								opts8_hibyte =  0x35;
								break;
							}
						}
						else
						{
							if ( device->code_memory_size == 0x100000 )
							{
								opts8_hibyte =  0x32;
								break;
							}
							if ( device->code_memory_size != 0x20000 )
							{
								if ( device->code_memory_size == 0x40000 )
								{
									opts8_hibyte =  0x30;
									break;
								}
								if ( device->code_memory_size == 0x80000 )
								{
									opts8_hibyte =  0x31;
									break;
								}
							}
						}
						opts8_hibyte =  0x2F;
						break;
					}
					opts8_hibyte =  0x12;
				}
				break;
			case 0x15u:
				opts8_hibyte =  0x3E;
				break;
			case 1u:
				opts8_hibyte =  0x80u;
				break;
			case 2u:
				if ( (HIBYTE(opts5_lo_word) & 0xF) == 1 )
				{
					opts8_hibyte =  0x7F;
					break;
				}
				v3 =  ((HIBYTE(opts5_lo_word) & 0xF) == 2) + 0x7D;
				goto LABEL_264;
			case 0x2Du:
				opts8_hibyte =  0x81u;
				break;
			default:
				break;
			}
		}
		return opts8_hibyte;
	}


	//Patch opts8 lobyte
	uint8_t patch_lo8(devstruct_t *device)
	{

		uint8_t opts8_lobyte = (uint8_t)(device->opts8);
		uint16_t opts5_lo_word = (uint16_t) device->opts5;

		if ( opts8_lobyte)
		{

			if ( opts8_lobyte == 4 )
				opts8_lobyte = 0;
			return opts8_lobyte;
		}
		else
		{
			switch ( device->protocol_id )
			{
			case 6u:
				if ( device->code_memory_size == 0x10000 )
					goto LABEL_4;
				if ( device->code_memory_size == 0x20000 )
				{
					opts8_lobyte = 9;
					if ( device->variant & 0x80 )
						goto LABEL_7;
				}
				else
				{
					if ( device->code_memory_size != 0x40000 )
						goto LABEL_11;
					opts8_lobyte = 0xB;
					if ( device->variant & 0x80 )
						goto LABEL_10;
				}
				return opts8_lobyte;
			case 3u:
			case 0xFu:
				if ( (device->variant & 0xF0) == 0x20 )
				{
					opts8_lobyte = 3;
				}
				else
				{
					opts8_lobyte = ((device->variant & 0xF0) == 0x10) + 1;
				}
				return opts8_lobyte;
			case 0x11u:
				if ( HIBYTE(opts5_lo_word) == 1 )
				{
					opts8_lobyte = 0x4C - ((device->variant & 0x80) != 0);
				}
				else
				{
					opts8_lobyte = (device->variant >> 7) | 0xE;
				}
				return opts8_lobyte;
			case 0x10u:
				switch ( device->code_memory_size )
				{
				case 0x8000:
					opts8_lobyte = 5;
					if ( device->variant )
						opts8_lobyte = 6;
					break;
				case 0x10000:
					opts8_lobyte = 7;
					if ( device->variant )
						opts8_lobyte = 8;
					break;
				case 0x20000:
					opts8_lobyte = 9;
					if ( device->variant )
						opts8_lobyte = 0xA;
					break;
				case 0x40000:
					opts8_lobyte = 11;
					if ( device->variant )
						opts8_lobyte = 0xC;
					break;
				default:
LABEL_11:
					if ( device->code_memory_size == 0x80000 )
						opts8_lobyte = 0xD;
					break;
				}
				return opts8_lobyte;
			case 8u:
				if ( device->variant )
				{
					if ( device->variant == 4 )
					{
						if ( device->code_memory_size == 0x10000 )
						{
LABEL_4:
							opts8_lobyte = 7;
						}
						else
						{
							opts8_lobyte = 4 * (device->code_memory_size != 0x20000) + 9;
						}
					}
					else
					{
LABEL_10:
						opts8_lobyte = 0xC;
					}
				}
				else
				{
LABEL_7:
					opts8_lobyte = 0xA;
				}
				return opts8_lobyte;
			case 7u:
				if ( device->variant & 0x20 )
				{
					if ( device->code_memory_size == 0x8000 )
					{
						opts8_lobyte = 0x14;
					}
					else if ( device->code_memory_size == 0x2000 )
					{
						opts8_lobyte = 0x13;
					}
					else
					{
						opts8_lobyte = 0x12;
					}
				}
				else
				{
					opts8_lobyte = (device->code_memory_size != 0x2000) + 0x15;
				}
				return opts8_lobyte;
			case 0xBu:
				opts8_lobyte = 0x17;
				return opts8_lobyte;
			case 9u:
				opts8_lobyte = (device->variant != 0) + 0x18;
				return opts8_lobyte;
			case 0xDu:
				if ( device->code_memory_size == 0x20000 )
				{
					opts8_lobyte = 9;
				}
				else if ( device->code_memory_size == 0x40000 )
				{
					opts8_lobyte = 0xB;
				}
				else
				{
					opts8_lobyte =  4 * (device->code_memory_size == 0x80000) + 9;
				}
				return opts8_lobyte;
			case 0xAu:
				if ( (device->variant & 0x80u) == 0 )
				{
					opts8_lobyte = (device->variant != 0) + 0x1E;
				}
				else
				{
					opts8_lobyte = 26;
				}
				return opts8_lobyte;
			case 0x21u:
			case 0x22u:
			case 0x23u:
			case 0x24u:
			case 0x25u:
			case 0x26u:
				goto LABEL_52;
			case 0x1Fu:
			case 0x20u:
				opts8_lobyte = 0x21;
				return opts8_lobyte;
			case 0x1Eu:
				opts8_lobyte = (uint8_t)(device->variant & 2 | 0x44) >> 1;
				return opts8_lobyte;
			case 0x1Du:
				if ( (uint8_t)device->package_details == 7 )
				{
					opts8_lobyte = 0x28;
				}
				else
				{
					if ( (device->package_details & 0xFF000000) == 0xFC000000 )
					{
						opts8_lobyte = 0x29;
					}
					else
					{
						if ( device->variant & 0xE0 )
						{
							switch (  device->variant & 0xE0 )
							{
							case 0x20:
								opts8_lobyte = 0x24;
								break;
							case 0x40:
								opts8_lobyte = 0x25;
								break;
							case 0x60:
								opts8_lobyte = 0x26;
								break;
							default:
								if ( (device->variant & 0xE0) == 0x80u )
									opts8_lobyte = 0x27;
								else
									opts8_lobyte = 0x22;
								break;
							}
						}
						else
						{
LABEL_52:
							opts8_lobyte = 0x20;
						}
					}
				}
				return opts8_lobyte;
			case 0x19u:
				if ( (device->variant & 0xF) == 1 )
					goto LABEL_70;
				if ( (device->variant & 0xF) == 2 )
				{
					opts8_lobyte = 0x2B;
					return opts8_lobyte;
				}
				if ( (device->variant & 0xF) == 3 )
					goto LABEL_74;
				if ( (device->variant & 0xF) == 4 )
LABEL_76:
				opts8_lobyte = 0x2C;
				return opts8_lobyte;
			case 0x18u:
			case 0x1Au:
			case 0x1Bu:
				if ( (device->variant & 0xF) == 7 )
					goto LABEL_76;
				switch ( device->variant & 0xF )
				{
				case 6:
					opts8_lobyte = 0x2D;
					break;
				case 5:
					opts8_lobyte = 0x2E;
					break;
				case 4:
LABEL_74:
					opts8_lobyte = 0x2F;
					break;
				case 3:
					opts8_lobyte = 0x30;
					break;
				case 1:
LABEL_70:
					opts8_lobyte = 0x2A;
					break;
				default:
					opts8_lobyte = ( device->variant & 0xF ) != 2 ? 0 : 0x2B;
					break;
				}
				return opts8_lobyte;
			case 0x1Cu:
				opts8_lobyte = ((device->variant & 0xF) != 0) + 0x31;
				return opts8_lobyte;
			case 0x2Au:
				opts8_lobyte = 0x33;
				return opts8_lobyte;
			case 0x2Bu:
			case 0x2Cu:
				opts8_lobyte = 0x34;
				return opts8_lobyte;
			case 0x13u:
				if ( device->variant & 0xF0 )
				{
					switch ( device->variant & 0xF0)
					{
					case 0x10:
						switch ( device->code_memory_size )
						{
						case 0x400000:
							opts8_lobyte = 0x37;
							break;
						case 0x200000:
							opts8_lobyte = 0x38;
							break;
						case 0x100000:
							opts8_lobyte = 0x39;
							break;
						default:
							opts8_lobyte = (device->code_memory_size != 0x80000) + 0x3A;
							break;
						}
						break;
					case 0x20:
						if ( HIBYTE(opts5_lo_word) )
						{
							opts8_lobyte = (device->code_memory_size != 0x200000) + 0x3C;
						}
						else
						{
							opts8_lobyte = (device->code_memory_size != 0x200000) + 0x44;
						}
						break;
					case 0x30:
						switch ( device->code_memory_size )
						{
						case 0x400000:
							opts8_lobyte = 0x3E;
							break;
						case 0x200000:
							opts8_lobyte = 0x3F;
							break;
						case 0x100000:
							opts8_lobyte = 0x40;
							break;
						default:
							opts8_lobyte = (device->code_memory_size != 0x80000) + 0x41;
							break;
						}
						break;
					case 0x40:
						if ( device->code_memory_size == 0x400000 )
						{
							opts8_lobyte = 0x43;
						}
						else if ( device->code_memory_size == 0x200000 )
						{
							opts8_lobyte = 0x44;
						}
						else
						{
							opts8_lobyte = (device->code_memory_size != 0x100000) + 0x45;
						}
						break;
					case 0x60:
						if ( device->code_memory_size == 0x400000 )
						{
							opts8_lobyte = 0x47;
						}
						else if ( device->code_memory_size == 0x200000 )
						{
							opts8_lobyte = 0x48;
						}
						else
						{
							opts8_lobyte = (device->code_memory_size != 0x100000) + 0x49;
						}
						break;
					}
				}
				else
				{
					opts8_lobyte = (device->code_memory_size != 0x200000) + 0x35;
				}
				return opts8_lobyte;
			case 0x14u:
				opts8_lobyte = 0x4D;
				return opts8_lobyte;
			case 0x12u:
				if ( (uint8_t)device->package_details == 2 )
				{
					switch ( device->code_memory_size )
					{
					case 0x200000:
						opts8_lobyte = 0x55;
						break;
					case 0x100000:
						opts8_lobyte = 0x54;
						break;
					case 0x80000:
						opts8_lobyte = 0x53;
						break;
					default:
						opts8_lobyte = (device->code_memory_size == 0x40000) + 0x51;
						break;
					}
					return opts8_lobyte;
				}
				if ( HIBYTE(opts5_lo_word) == 0xFFu || HIBYTE(opts5_lo_word) == 1 )
				{
					if ( device->code_memory_size == 0x800000 )
					{
LABEL_137:
						opts8_lobyte = 0x4E;
						return opts8_lobyte;
					}
					switch ( device->code_memory_size )
					{
					case 0x400000:
						opts8_lobyte = 0x61;
						break;
					case 0x200000:
						opts8_lobyte = 0x60;
						break;
					case 0x100000:
						opts8_lobyte = 0x5F;
						break;
					case 0x80000:
						opts8_lobyte = 0x5E;
						break;
					default:
						opts8_lobyte = (device->code_memory_size == 0x40000) + 0x5C;
						break;
					}
				}
				else
				{
					if ( HIBYTE(opts5_lo_word) )
						return opts8_lobyte;
					if ( device->code_memory_size == 0x800000 )
						goto LABEL_137;
					switch ( device->code_memory_size )
					{
					case 0x400000:
						opts8_lobyte = 0x57;
						break;
					case 0x200000:
						opts8_lobyte = 0x58;
						break;
					case 0x100000:
						opts8_lobyte = 0x59;
						break;
					case 0x80000:
						opts8_lobyte = 0x5A;
						break;
					default:
						if ( device->code_memory_size == 0x40000 )
							opts8_lobyte = 0x5B;
						else
							opts8_lobyte = 0x4E;
						break;
					}
				}
				break;
			case 0x15u:
				opts8_lobyte = 0x62;
				return opts8_lobyte;
			case 1u:
				opts8_lobyte = 0x6B;
				return opts8_lobyte;
			case 2u:
				if ( (HIBYTE(opts5_lo_word) & 0xF) == 1 )
				{
					opts8_lobyte = 0x6E;
				}
				else
				{
					opts8_lobyte = (( HIBYTE(opts5_lo_word) & 0xF) == 2) + 0x6C;
				}
				return opts8_lobyte;
			case 0x2Du:
				opts8_lobyte = ((uint8_t)device->package_details != 8) + 0x6F;
				return opts8_lobyte;
			case 4u:
				opts8_lobyte = 0x71;
				return opts8_lobyte;
			case 0x30u:
				opts8_lobyte = 0x72;
				return opts8_lobyte;
			default:
				return opts8_lobyte;
			}
		}
		return opts8_lobyte;
	}




}