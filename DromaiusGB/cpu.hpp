#pragma once

#include "lcd.hpp"
#include "timer.hpp"
#include "interrupts.hpp"

#include <atomic>
#include <thread>
#include <SFML/Window.hpp>


namespace dromaiusgb
{

	typedef byte opcode_t;

	union flags_t
	{
		byte value;
		struct
		{
			byte unused : 4;
			byte cy : 1;
			byte h : 1;
			byte n : 1;
			byte zf : 1;
		};

		flags_t(int i) : value(i) {}
		operator byte() const { return value; }
	};

	union register_t
	{
		word value;
		struct
		{
			union
			{
				byte lo;
				flags_t flags;
			};
			byte hi;
		};

		register_t() : value(0) {}
		register_t(int i) : value(i) {}
		operator word() const { return value; }
	};

	class CPU
	{
	private:
		register_t AF;
		register_t BC;
		register_t DE;
		register_t HL;
		word SP;
		word PC;

		byte *registers[8];
		word *wregisters[4];

	private:
		const dword clock_speed = 4194304;
		
	private:
		bool interrupt_master_enable_flag = true;
		Bus &bus;
		LCD &lcd;
		Timer &timer;
		InterruptController &interrupt_controller;

		std::thread thread;
		std::atomic<bool> running;

	private:
		void HandleInterrupts();
		dword HandleCBPrefixOpcode();
		dword Step();

	public:
		CPU(Bus &, LCD &, Timer &, InterruptController &);

		void Start();
		void Stop();
	};
}