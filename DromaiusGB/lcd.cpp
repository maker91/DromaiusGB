#include "lcd.hpp"
#include "bus.hpp"
#include "interrupts.hpp"

#include <iostream>


namespace dromaiusgb
{

	LCD::LCD(Bus &bus, InterruptController &ic) : Addressable(bus), interrupt_controller(ic)
	{
		screen_buffer = new uint32_t[160 * 144];
		screen_texture.create(160, 144);

		cycle = 0;
		ly = 0;
		scroll_y = 0;
		scroll_x = 0;
		mode = LCDMode::HBlank;
	}

	LCD::~LCD()
	{

	}

	void LCD::Set(bus_address_t addr, byte val)
	{
		switch (addr.address)
		{
		case 0xFF40: lcd_control = val; break;
		case 0xFF41: lcd_status = val; break;
		case 0xFF42: scroll_y = val; break;
		case 0xFF43: scroll_x = val; break;
		case 0xFF44: ly = val; break;
		case 0xFF45: lyc = val; break;
		case 0xFF46: LaunchDMA(val); break;
		case 0xFF47: bg_palette = val; break;
		case 0xFF48: obj_palette_0 = val; break;
		case 0xFF49: obj_palette_1 = val; break;
		case 0xFF4A: window_x = val; break;
		case 0xFF4B: window_y = val; break;
		}
	}

	byte LCD::Get(bus_address_t addr) const
	{
		switch (addr.address)
		{
		case 0xFF40: return lcd_control;
		case 0xFF41: return lcd_status;
		case 0xFF42: return scroll_y;
		case 0xFF43: return scroll_x;
		case 0xFF44: return ly;
		case 0xFF45: return lyc;
		case 0xFF46: return dma;
		case 0xFF47: return bg_palette;
		case 0xFF48: return obj_palette_0;
		case 0xFF49: return obj_palette_1;
		case 0xFF4A: return window_x;
		case 0xFF4B: return window_y;
		default: return 0xFF;
		}
	}

	std::uint32_t LCD::GetShadeColor(byte shade_index)
	{
		switch (shade_index) {
		case 0: default: return 0xFF06978A; // white
		case 1: return 0xFF025137; // light gray
		case 2: return 0xFF047461; // dark gray
		case 3: return 0xFF002F0E; // black
		}
	}

	void LCD::ClearScanLine(byte sy, std::uint32_t color)
	{
		for (unsigned int sx = 0; sx < 160; sx++)
		{
			screen_buffer[sx + sy * 160] = color;
		}
	}

	void LCD::DrawTileMapScanLine(byte sy, byte start_x, byte *bg_map, tile_data_t *tile_data)
	{
		byte scrolled_y = scroll_y + sy;
		byte tile_y = scrolled_y / 8;
		byte pixel_y = scrolled_y % 8;

		for (unsigned int sx = start_x; sx < 160; sx++)
		{
			// get which background tile coord and pixel we are on
			byte scrolled_x = scroll_x + sx;
			byte tile_x = scrolled_x / 8;
			byte pixel_x = scrolled_x % 8;

			// get the background tile to draw from the background map
			byte tile_number = bg_map[32 * tile_y + tile_x];

			// get the tile from the tile data;
			tile_data_t tile;
			if (lcd_control.bg_and_window_tile_data_select == 0)
				tile = *(tile_data + (sbyte)tile_number);
			else
				tile = *(tile_data + tile_number);

			// get the pixel palette index in the tile
			word tile_line = tile.lines[pixel_y];
			byte palette_index = ((tile_line >> (7 - pixel_x)) & 0x01) | (((tile_line >> (15 - pixel_x)) & 0x01) << 0x01);

			// convert the palette index to a color
			byte shade_index;
			switch (palette_index) {
			case 0: shade_index = bg_palette.shade0; break;
			case 1: shade_index = bg_palette.shade1; break;
			case 2: shade_index = bg_palette.shade2; break;
			case 3: shade_index = bg_palette.shade3; break;
			}

			screen_buffer[sx + sy * 160] = GetShadeColor(shade_index);
		}
	}

	void LCD::DrawScanLine(byte sy)
	{
		// clear the screen to white
		//ClearScanLine(sy, 0xFF06978A);

		if (lcd_control.lcd_display_enable == 0)
			return;

		// load the correct bg tile map for the background
		byte *bg_map;
		switch (lcd_control.bg_tile_map_display_select) {
		case 0: default:
			bg_map = bus.GetBlock(0x9800); break;
		case 1:
			bg_map = bus.GetBlock(0x9C00); break;
		}

		// load the correct bg tile map for the window
		byte *window_map;
		switch (lcd_control.window_tile_map_display_select) {
		case 0: default:
			window_map = bus.GetBlock(0x9800); break;
		case 1:
			window_map = bus.GetBlock(0x9C00); break;
		}

		// load the correct tile data
		tile_data_t *tile_data;
		switch (lcd_control.bg_and_window_tile_data_select)
		{
		case 0: default:
			tile_data = (tile_data_t *)bus.GetBlock(0x9000); break;
		case 1:
			tile_data = (tile_data_t *)bus.GetBlock(0x8000); break;
		}

		// draw the background
		if (lcd_control.bg_display) {
			DrawTileMapScanLine(sy, 0, bg_map, tile_data);
		}

		// draw window
		if (lcd_control.window_display_enable && sy >= window_y && window_x <= 167 && window_y < 144) {
			DrawTileMapScanLine(sy, std::min(0, window_x - 7), window_map, tile_data);
		}

		// draw sprites (TODO: some sprites should be under window/bg)
		if (lcd_control.obj_display_enable) {
			byte sprite_height = 8 << (lcd_control.obj_size);
			sprite_attribute_t *sprites = (sprite_attribute_t *)bus.GetBlock(0xFE00);

			for (int i = 0; i < 40; i++) {
				sprite_attribute_t sprite = sprites[i];

				if (sprite.pos_y == 0 || sprite.pos_y >= 160)
					continue;

				if (sprite.pos_x == 0 || sprite.pos_x >= 168)
					continue;

				int sprite_pos_x = sprite.pos_x - 8;
				int sprite_pos_y = sprite.pos_y - 16;

				if (sy < sprite_pos_y || sy >= sprite_height + sprite_pos_y)
					// sprite is not on this scan line
					continue;

				tile_data_t tile = tile_data[sprite.tile_num];

				// the y pixel to sample in the sprite tile
				int sample_y = sy - sprite_pos_y;
				if (sprite.flip_y)
					sample_y = sprite_height - sample_y - 1;

				for (int sx = std::max(0, sprite_pos_x); sx < sprite_pos_x + 8; sx++) {

					// the x pixel to sample in the sprite tile
					int sample_x = sx - sprite_pos_x;
					if (sprite.flip_x)
						sample_x = 8 - sample_x - 1;

					// get the pixel palette index in the tile
					word tile_line = tile.lines[sample_y];
					byte palette_index = ((tile_line >> (7 - sample_x)) & 0x01) | (((tile_line >> (15 - sample_x)) & 0x01) << 0x01);

					// get the palette for the sprite
					color_palette_t obj_palete;
					switch (sprite.palette_num) {
					case 0: obj_palete = obj_palette_0; break;
					case 1: obj_palete = obj_palette_1; break;
					}

					byte shade_index;
					switch (palette_index) {
					case 0: continue;
					case 1: shade_index = obj_palete.shade1; break;
					case 2: shade_index = obj_palete.shade2; break;
					case 3: shade_index = obj_palete.shade3; break;
					}

					screen_buffer[sx + sy * 160] = GetShadeColor(shade_index);
				}
			}
		}
	}

	void LCD::SwapBuffers()
	{
		screen_texture.update((sf::Uint8 *)screen_buffer);
	}

	const sf::Texture &LCD::GetScreenTexture() const
	{
		return screen_texture;
	}

	void LCD::LaunchDMA(byte request)
	{
		word source_addr = request << 8;
		const byte *src = bus.GetBlock(source_addr);
		byte *dst = bus.GetBlock(0xFE00);

		memcpy(dst, src, 0x9F);
	}

	void LCD::Tick(dword delta_cycle)
	{
		if (!lcd_control.lcd_display_enable)
			return;

		cycle += delta_cycle;

		// new vline
		if (cycle >= cycles_per_vline) {
			cycle -= cycles_per_vline;
			ly += 1;

			// check coincidence
			if (ly == lyc) {
				lcd_status.coincidence_flag = 1;

				if (lcd_status.coincidence_interrupt)
					interrupt_controller.RequestInterrupt(InterruptFlags::LCDStat);
			} else {
				lcd_status.coincidence_flag = 0;
			}

			if (ly < vblank_start) // start hblank
			{
				// draw the previous scanline
				DrawScanLine(ly - 1);

				mode = LCDMode::HBlank;
				lcd_status.mode_flag = (byte)mode;

				if (lcd_status.mode_0_hblank_interrupt)
					interrupt_controller.RequestInterrupt(InterruptFlags::LCDStat);
			} 
			else if (ly == vblank_start) // start vblank 
			{
				// draw the previous scanline
				DrawScanLine(ly - 1);

				mode = LCDMode::VBlank;
				lcd_status.mode_flag = (byte)mode;

				interrupt_controller.RequestInterrupt(InterruptFlags::VBlank);
				if (lcd_status.mode_1_vblank_interrupt)
					interrupt_controller.RequestInterrupt(InterruptFlags::LCDStat);

				SwapBuffers();
			} 
			else if (ly >= vlines_per_frame) // end vblank
			{
				ly -= vlines_per_frame;

				mode = LCDMode::HBlank;
				lcd_status.mode_flag = (byte)mode;

				if (lcd_status.mode_0_hblank_interrupt)
					interrupt_controller.RequestInterrupt(InterruptFlags::LCDStat);
			}
		}

		// cycle between hblank, oam_search, data_transfer (when not in vblank)
		if (mode != LCDMode::VBlank) 
		{
			if (cycle >= data_transfer_start_cycle && mode == LCDMode::OAMSearch)
			{
				mode = LCDMode::DataTransfer;
				lcd_status.mode_flag = (byte)mode;
			}
			else if (cycle >= oam_search_start_cycle && mode == LCDMode::HBlank)
			{
				mode = LCDMode::OAMSearch;
				lcd_status.mode_flag = (byte)mode;

				if (lcd_status.mode_2_oam_interrupt)
					interrupt_controller.RequestInterrupt(InterruptFlags::LCDStat);
			}
		}
	}
}