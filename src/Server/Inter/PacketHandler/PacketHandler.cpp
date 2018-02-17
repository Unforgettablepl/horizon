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

#include "PacketHandler.hpp"

#include "Core/Logging/Logger.hpp"
#include "Libraries/BCrypt/BCrypt.hpp"
#include "Server/Common/Models/SessionData.hpp"
#include "Server/Inter/Inter.hpp"
#include "Server/Inter/InterStorage.hpp"
#include "Server/Inter/PacketHandler/Packets.hpp"
#include "Server/Inter/Session/Session.hpp"
#include "Server/Inter/SocketMgr/ClientSocketMgr.hpp"

#include <boost/bind.hpp>

Horizon::Inter::PacketHandler::PacketHandler(std::shared_ptr<Session> session)
: Horizon::Base::PacketHandler<Session>(session)
{
	// Construct
	InitializeHandlers();
}

Horizon::Inter::PacketHandler::~PacketHandler()
{
	// Destruct
}

/**
 * Handlers
 */

void Horizon::Inter::PacketHandler::InitializeHandlers()
{
#define HANDLER_FUNC(packet) addPacketHandler(Horizon::Base::inter_packets::packet, boost::bind((&PacketHandler::Handle_ ## packet), this, boost::placeholders::_1))
	HANDLER_FUNC(INTER_CONNECT_AUTH);
	HANDLER_FUNC(INTER_SESSION_DEL);
	HANDLER_FUNC(INTER_SESSION_SET);
	HANDLER_FUNC(INTER_SESSION_REQ);
	HANDLER_FUNC(INTER_GAME_ACCOUNT_DEL);
	HANDLER_FUNC(INTER_GAME_ACCOUNT_SET);
	HANDLER_FUNC(INTER_GAME_ACCOUNT_REQ);
	HANDLER_FUNC(INTER_PING);
#undef HANDLER_FUNC
}

void Horizon::Inter::PacketHandler::Handle_INTER_CONNECT_AUTH(PacketBuffer &buf)
{
	PACKET_INTER_CONNECT_AUTH pkt;
	bool success;

	buf.read(&pkt, 5);

	char pass[pkt.packet_len - 5];

	buf.read(pass, sizeof(pass));

	InterLog->info("Received request for connection of client from endpoint {}.", getSession()->getRemoteIPAddress());
	if (!(success = BCrypt::validatePassword(pass, InterServer->getNetworkConf().getInterServerPassword()))) {
		std::string password = pass;
		InterLog->info("Connection refused for endpoint {}, incorrect password {}.", getSession()->getRemoteIPAddress(), password);
		getSession()->closeSocket();
	} else {
		InterLog->info("Connection successfully authorized for endpoint {}.", getSession()->getRemoteIPAddress());
	}
	
	getSession()->setClientType(pkt.client_type);

	Send_INTER_ACK_RECEIVED(pkt.op_code, (uint8_t) success);
}

/**
 * Session
 */
void Horizon::Inter::PacketHandler::Handle_INTER_SESSION_SET(PacketBuffer &buf)
{
	PACKET_INTER_SESSION_SET pkt;
	SessionData session_data;
	buf >> pkt;
	session_data << pkt.s;
	SessionStore->add(pkt.s.auth_code, session_data);
	InterLog->info("Added session data for id {} requested by server {}.", pkt.s.auth_code, (int) getSession()->getClientType());
}

void Horizon::Inter::PacketHandler::Handle_INTER_SESSION_DEL(PacketBuffer &buf)
{
	PACKET_INTER_SESSION_DEL pkt;
	buf >> pkt;
	SessionStore->remove(pkt.auth_code);
	InterLog->info("Removed session data for id {} requested by server {}.", pkt.auth_code, (int) getSession()->getClientType());
}

void Horizon::Inter::PacketHandler::Handle_INTER_SESSION_REQ(PacketBuffer &buf)
{
	PACKET_INTER_SESSION_REQ pkt;
	PACKET_INTER_SESSION_GET send_pkt;
	std::shared_ptr<SessionData> session_data;

	buf >> pkt;

	session_data = SessionStore->get(pkt.auth_code);

	if (session_data != nullptr) {
		*session_data >> send_pkt.s;
		SendPacket(send_pkt);
		InterLog->info("Found session data for id {} requested by server {}.", pkt.auth_code, (int) getSession()->getClientType());
	} else {
		Send_INTER_ACK_RECEIVED(pkt.op_code, false);
		InterLog->info("Not found session data for id {} requested by server {}.", pkt.auth_code, (int) getSession()->getClientType());
	}
}

void Horizon::Inter::PacketHandler::Handle_INTER_PING(PacketBuffer &buf)
{
	Send_INTER_PONG();
}

/**
 * Game Account
 */
void Horizon::Inter::PacketHandler::Handle_INTER_GAME_ACCOUNT_SET(PacketBuffer &buf)
{
	PACKET_INTER_GAME_ACCOUNT_SET pkt;
	GameAccount game_account;
	buf >> pkt;
	game_account << pkt.s;
	AccountStore->add(pkt.s.id, game_account);
	InterLog->info("Added account data for id {} requested by server {}.", pkt.s.id, (int) getSession()->getClientType());
}

void Horizon::Inter::PacketHandler::Handle_INTER_GAME_ACCOUNT_DEL(PacketBuffer &buf)
{
	PACKET_INTER_GAME_ACCOUNT_DEL pkt;
	buf >> pkt;
	AccountStore->remove(pkt.account_id);
	InterLog->info("Removed account data for id {} requested by server {}.", pkt.account_id, (int) getSession()->getClientType());
}

void Horizon::Inter::PacketHandler::Handle_INTER_GAME_ACCOUNT_REQ(PacketBuffer &buf)
{
	PACKET_INTER_GAME_ACCOUNT_REQ pkt;
	PACKET_INTER_GAME_ACCOUNT_GET send_pkt;
	std::shared_ptr<GameAccount> game_account;

	buf >> pkt;

	game_account = AccountStore->get(pkt.account_id);

	if (game_account != nullptr) {
		*game_account >> send_pkt.s;
		SendPacket(send_pkt);
		InterLog->info("Found game account data for id {} requested by server {}.", pkt.account_id, (int) getSession()->getClientType());
	} else {
		Send_INTER_ACK_RECEIVED(pkt.op_code, false);
		InterLog->info("Not found game account data for id {} requested by server {}.", pkt.account_id, (int) getSession()->getClientType());
	}
}

/**
 * Senders
 */
void Horizon::Inter::PacketHandler::Send_INTER_ACK_RECEIVED(uint16_t packet_id, uint8_t response)
{
	PACKET_INTER_ACK_RECEIVED pkt;

	pkt.ack_packet_id = packet_id;
	pkt.response = response;

	SendPacket(pkt);
}

void Horizon::Inter::PacketHandler::Send_INTER_CONNECT_INIT()
{
	SendPacket(PACKET_INTER_CONNECT_INIT());
}

void Horizon::Inter::PacketHandler::Send_INTER_PONG()
{
	SendPacket(PACKET_INTER_PONG());
}
