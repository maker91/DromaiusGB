#pragma once

#include <vector>
#include <memory>
#include "addressable.hpp"


namespace dromaiusgb
{

	struct address_space_t
	{
		address_t start;
		address_t end;
		std::shared_ptr<Addressable> addressable;

		bool contains(address_t addr) const
		{
			return addr >= start && addr <= end;
		}
	};

	class Bus
	{
	private:
		std::vector<address_space_t> address_spaces;

	public:
		void Set(address_t, byte);
		byte Get(address_t) const;

		byte *GetBlock(address_t) const;

		void RegisterAddressSpace(address_t, address_t, std::shared_ptr<Addressable>);
	};
}