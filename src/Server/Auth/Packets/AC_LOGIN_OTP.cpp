/***************************************************
 *       _   _            _                        *
 *      | | | |          (_)                       *
 *      | |_| | ___  _ __ _ _______  _ __          *
 *      |  _  |/ _ \| '__| |_  / _ \| '_  \        *
 *      | | | | (_) | |  | |/ / (_) | | | |        *
 *      \_| |_/\___/|_|  |_/___\___/|_| |_|        *
 ***************************************************
 * This file is part of Horizon (c).
 * Copyright (c) 2020- Horizon Dev Team.
 *
 * Base Author - Sagun Khosla. (sagunxp@gmail.com)
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

#include "AC_LOGIN_OTP.hpp"
#include "Server/Auth/Socket/AuthSocket.hpp"

using namespace Horizon::Auth;
using namespace Horizon::Base;

AC_LOGIN_OTP::AC_LOGIN_OTP(std::shared_ptr<AuthSocket> sock)
 : NetworkPacket<AuthSocket>(ID_AC_LOGIN_OTP, sock) { }

AC_LOGIN_OTP::~AC_LOGIN_OTP() { }

void AC_LOGIN_OTP::deliver()
{
}
ByteBuffer &AC_LOGIN_OTP::serialize()
{
	return buf();
}
void AC_LOGIN_OTP::handle(ByteBuffer &&buf)
{
}
void AC_LOGIN_OTP::deserialize(ByteBuffer &buf)
{
}

