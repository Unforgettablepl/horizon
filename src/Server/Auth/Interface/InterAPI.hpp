/***************************************************
 *       _   _            _                        *
 *      | | | |          (_)                       *
 *      | |_| | ___  _ __ _ _______  _ __          *
 *      |  _  |/ _ \| '__| |_  / _ \| '_  \        *
 *      | | | | (_) | |  | |/ / (_) | | | |        *
 *      \_| |_/\___/|_|  |_/___\___/|_| |_|        *
 ***************************************************
 * This file is part of Horizon (c).
 * Copyright (c) 2018 Horizon Dev Team.
 *
 * Base Author - Sagun Khosla. (sagunxp@gmail.com)
 *
 * Under a proprietary license this file is not for use
 * or viewing without permission.
 **************************************************/

#ifndef HORIZON_AUTH_INTERFACE_INTERAPI_HPP
#define HORIZON_AUTH_INTERFACE_INTERAPI_HPP

#include "Server/Common/Horizon.hpp"
#include "Server/Common/Interfaces/InterServerAPI.hpp"
#include "Server/Auth/PacketHandler/InterPacketHandler.hpp"
#include "Server/Auth/Session/InterSession.hpp"

namespace Horizon
{
namespace Auth
{
namespace Interface
{
class InterAPI : public Horizon::Interface::InterServerAPI<InterSession>
{
public:
	InterAPI() : InterServerAPI(InterSocktMgr->getConnectedSession(INTER_SESSION_NAME)){}
	~InterAPI() {}

	static InterAPI *getInstance()
	{
		static InterAPI inter_api;
		return &inter_api;
	}
};
}
}
}

#define AuthInterAPI Horizon::Auth::Interface::InterAPI::getInstance()

#endif // HORIZON_AUTH_INTERFACE_INTERQUERY_HPP
