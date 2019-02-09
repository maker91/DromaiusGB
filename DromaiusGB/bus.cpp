#include "bus.hpp"
#include <iostream>


namespace dromaiusgb
{

	void Bus::Set(address_t addr, byte val)
	{
		for (auto &space : address_spaces) {
			if (space.contains(addr) && space.addressable->Enabled()) {
				bus_address_t baddr{ addr, address_t(addr - space.start) };
				space.addressable->Set(baddr, val);
				return;
			}
		}
	}

	byte Bus::Get(address_t addr) const
	{
		for (auto &space : address_spaces) {
			if (space.contains(addr) && space.addressable->Enabled()) {
				bus_address_t baddr{ addr, address_t(addr - space.start) };
				return space.addressable->Get(baddr);
			}
		}

		return 0xFF;
	}

	byte *Bus::GetBlock(address_t addr) const
	{
		for (auto &space : address_spaces) {
			if (space.contains(addr) && space.addressable->Enabled()) {
				bus_address_t baddr{ addr, address_t(addr - space.start) };
				return space.addressable->GetBlock(baddr);
			}
		}

		return nullptr;
	}

	void Bus::RegisterAddressSpace(address_t start, address_t end, std::shared_ptr<Addressable> addressable)
	{
		address_spaces.push_back({ start, end, addressable });
	}
}