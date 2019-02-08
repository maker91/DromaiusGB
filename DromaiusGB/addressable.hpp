#pragma once

#include "types.hpp"


namespace dromaiusgb
{

	class Bus;
	
	class Addressable
	{
	protected:
		Bus &bus;

	public:
		Addressable(Bus &bus) : bus(bus) {}

		virtual bool Enabled() const { return true; }

		virtual void Set(bus_address_t, byte) =0;
		virtual byte Get(bus_address_t) const =0;
		virtual byte *GetBlock(bus_address_t) { return nullptr; }
	};
}