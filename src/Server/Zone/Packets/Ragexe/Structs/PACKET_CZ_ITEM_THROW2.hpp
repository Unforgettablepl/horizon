/***************************************************
 *       _   _            _                        *
 *      | | | |          (_)                       *
 *      | |_| | ___  _ __ _ _______  _ __          *
 *      |  _  |/ _ \| '__| |_  / _ \| '_  \        *
 *      | | | | (_) | |  | |/ / (_) | | | |        *
 *      \_| |_/\___/|_|  |_/___\___/|_| |_|        *
 ***************************************************
 * This file is part of Horizon (c).
 *
 * Copyright (c) 2019 Sagun K. (sagunxp@gmail.com).
 * Copyright (c) 2019 Horizon Dev Team.
 *
 * Base Author - Sagun K. (sagunxp@gmail.com)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************/

#ifndef HORIZON_ZONE_RAGEXE_PACKET_CZ_ITEM_THROW2_HPP
#define HORIZON_ZONE_RAGEXE_PACKET_CZ_ITEM_THROW2_HPP


#include "Server/Zone/Packets/Ragexe/Packets.hpp"
#include "PACKET_CZ_ITEM_THROW.hpp"
#include "Server/Common/PacketBuffer.hpp"


namespace Horizon
{
namespace Zone
{
namespace Ragexe
{
struct PACKET_CZ_ITEM_THROW2 : public PACKET_CZ_ITEM_THROW
{
	PACKET_CZ_ITEM_THROW2(uint16_t packet_id = CZ_ITEM_THROW2) : PACKET_CZ_ITEM_THROW(packet_id) { }

	virtual void deserialize(PacketBuffer &buf)
	{
		buf >> packet_id;
		buf >> inventory_index;
		buf >> amount;
	}

	virtual PACKET_CZ_ITEM_THROW2 & operator << (PacketBuffer &right)
	{
		deserialize(right);
		return *this;
	}

	/* Size: 6 bytes */
	uint16_t inventory_index{0};
	uint16_t amount{0};
};
}
}
}

#endif /* HORIZON_ZONE_RAGEXE_PACKET_CZ_ITEM_THROW2_HPP */
