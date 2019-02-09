#pragma once

#include "types.hpp"

namespace dromaiusgb
{

	struct cartridge_header_t
	{
		byte entry_point[0x04];
		byte nintendo_logo[0x2F];
		byte title[0x0F];
		word licensee_code;
		byte sgb_flag;
		byte cartridge_type;
		byte rom_size;
		byte ram_size;
		byte destination_code;
		byte old_licensee_code;
		byte rom_version;
		byte header_checksum;
		word global_checksum;
	};
}