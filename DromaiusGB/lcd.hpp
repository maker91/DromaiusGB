#pragma once

#include <SFML/Graphics.hpp>

#include "types.hpp"
#include "addressable.hpp"
#include "interrupts.hpp"


namespace dromaiusgb
{

	union lcd_control_t
	{
		byte value;
		struct
		{
			byte bg_display : 1;
			byte obj_display_enable : 1;
			byte obj_size : 1;
			byte bg_tile_map_display_select : 1;
			byte bg_and_window_tile_data_select : 1;
			byte window_display_enable : 1;
			byte window_tile_map_display_select : 1;
			byte lcd_display_enable : 1;
		};

		lcd_control_t() : value(0) {};
		lcd_control_t(int i) : value(i) {}
		operator byte() const { return value; };
	};

	union lcd_status_t
	{
		byte value;
		struct
		{
			byte mode_flag : 2;
			byte coincidence_flag : 1;
			byte mode_0_hblank_interrupt : 1;
			byte mode_1_vblank_interrupt : 1;
			byte mode_2_oam_interrupt : 1;
			byte coincidence_interrupt : 1;
			byte : 1;
		};

		lcd_status_t() : value(0) {};
		lcd_status_t(int i) : value(i) {};
		operator byte() const { return value; };
	};

	struct sprite_attribute_t
	{
		byte pos_y;
		byte pos_x;
		byte tile_num;
		
		struct
		{
			byte unused : 4;
			byte palette_num : 1;
			byte flip_x : 1;
			byte flip_y : 1;
			byte obj_bg_priority : 1;
		};
	};

	struct tile_data_t
	{
		word lines[8];
	};

	union color_palette_t
	{
		byte value;
		struct
		{
			byte shade0 : 2;
			byte shade1 : 2;
			byte shade2 : 2;
			byte shade3 : 2;
		};

		color_palette_t() : value(0) {};
		color_palette_t(int i) : value(i) {};
		operator byte() const { return value; }
	};

	enum class LCDMode : byte
	{
		HBlank = 0,
		VBlank = 1,
		OAMSearch = 2,
		DataTransfer = 3,
	};

	class LCD : public Addressable
	{
	private:
		const dword cycles_per_frame = 70224; // framerate = 59.73
		const dword vlines_per_frame = 154;
		const dword cycles_per_vline = 456; // cycles_per_frame / vlines_per_frame;

		const dword hblank_end_cycle = 204;
		const dword oam_search_start_cycle = 204;
		const dword data_transfer_start_cycle = 284;

		const dword vblank_start = 144; // vblank starts at vline 144

	private:
		lcd_control_t lcd_control;
		lcd_status_t lcd_status;
		byte scroll_y;
		byte scroll_x;
		byte ly;
		byte lyc;
		byte dma;
		color_palette_t bg_palette;
		color_palette_t obj_palette_0;
		color_palette_t obj_palette_1;
		byte window_y;
		byte window_x;

	private:
		InterruptController &interrupt_controller;
		dword cycle;
		LCDMode mode;

		sf::Uint32 *screen_buffer;
		sf::Texture screen_texture;
	private:
		void ClearScanLine(byte, std::uint32_t);
		void DrawTileMapScanLine(byte, byte, byte *, tile_data_t *);
		void DrawScanLine(byte);
		void SwapBuffers();

		std::uint32_t GetShadeColor(byte);

	public:
		LCD(Bus &, InterruptController &);
		~LCD();

		void Set(bus_address_t, byte);
		byte Get(bus_address_t) const;

		void Tick(dword delta_cycle);
		const sf::Texture &GetScreenTexture() const;
		void LaunchDMA(byte);
	};
}