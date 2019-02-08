#include "lcd.hpp"
#include "bus.hpp"
#include "interrupts.hpp"

#include <iostream>


namespace dromaiusgb
{

	LCD::LCD(Bus &bus) : Addressable(bus)
	{
		screen_buffer = new uint32_t[160 * 144];
		screen_texture.create(160, 140);

		cycle = 0;
		ly = 0;
		scroll_y = 0;
		scroll_x = 0;
		mode = LCDMode::HBlank;
	}

	LCD::~LCD()
	{
		if (draw_thread.joinable())
			draw_thread.join();
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
		case 0xFF46: dma = val; break;
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

	void LCD::DrawRoutine()
	{
		// clear the screen to white
		memset(screen_buffer, 0xFF, 160 * 144);

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
			for (unsigned int ly = 0; ly < 144; ly++)
				for (unsigned int lx = 0; lx < 160; lx++)
				{
					// get which background tile coord we are on
					byte scrolled_x = scroll_x + lx;
					byte scrolled_y = scroll_y + ly;
					byte tile_x = scrolled_x / 8;
					byte tile_y = scrolled_y / 8;

					// get which tile pixel we are on
					byte pixel_x = scrolled_x % 8;
					byte pixel_y = scrolled_y % 8;

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

					screen_buffer[lx + ly * 160] = GetShadeColor(shade_index);
				}
		}

		// draw window

		// draw sprites
		if (lcd_control.obj_display_enable) {
			sprite_attribute_t *sprites = (sprite_attribute_t *)bus.GetBlock(0xFE00);

			for (int i = 0; i < 40; i++) {
				sprite_attribute_t sprite = sprites[i];

				if (sprite.pos_y == 0 || sprite.pos_y >= 160)
					continue;

				if (sprite.pos_x == 0 || sprite.pos_x >= 168)
					continue;

				tile_data_t tile = tile_data[sprite.tile_num];

				for (byte pixel_x = 0; pixel_x < 8; pixel_x++)
					for (byte pixel_y = 0; pixel_y < 8; pixel_y++) {
						// get the pixel palette index in the tile
						word tile_line = tile.lines[pixel_y];
						byte palette_index = ((tile_line >> (7 - pixel_x)) & 0x01) | (((tile_line >> (7 + 7 - pixel_x)) & 0x01) << 0x01);

						byte shade_index;
						switch (palette_index) {
						case 0: shade_index = bg_palette.shade0; break;
						case 1: shade_index = bg_palette.shade1; break;
						case 2: shade_index = bg_palette.shade2; break;
						case 3: shade_index = bg_palette.shade3; break;
						}

						byte lx = sprite.pos_x + pixel_x;
						byte ly = sprite.pos_y + pixel_y;
						screen_buffer[lx + ly * 160] = GetShadeColor(shade_index);
					}
			}
		}

		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	void LCD::SwapBuffers()
	{
		screen_texture.update((sf::Uint8 *)screen_buffer);
	}

	const sf::Texture &LCD::GetScreenTexture() const
	{
		return screen_texture;
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
					request_interrupt(bus, InterruptFlags::LCDStat);
			} else {
				lcd_status.coincidence_flag = 0;
			}

			if (ly < vblank_start) // start hblank
			{
				mode = LCDMode::HBlank;
				lcd_status.mode_flag = (byte)mode;

				if (lcd_status.mode_0_hblank_interrupt)
					request_interrupt(bus, InterruptFlags::LCDStat);
			} 
			else if (ly == vblank_start) // start vblank 
			{
				mode = LCDMode::VBlank;
				lcd_status.mode_flag = (byte)mode;

				request_interrupt(bus, InterruptFlags::VBlank);
				if (lcd_status.mode_1_vblank_interrupt)
					request_interrupt(bus, InterruptFlags::LCDStat);

				//if (draw_thread.joinable())
					//draw_thread.join();

				SwapBuffers();
			} 
			else if (ly >= vlines_per_frame) // end vblank
			{
				ly -= vlines_per_frame;

				mode = LCDMode::HBlank;
				lcd_status.mode_flag = (byte)mode;

				if (lcd_status.mode_0_hblank_interrupt)
					request_interrupt(bus, InterruptFlags::LCDStat);

				//draw_thread = std::thread(&LCD::DrawRoutine, this);
				DrawRoutine();
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
					request_interrupt(bus, InterruptFlags::LCDStat);
			}
		}

		// TODO: trigger LCD STAT interrupts based on status flags
	}
}