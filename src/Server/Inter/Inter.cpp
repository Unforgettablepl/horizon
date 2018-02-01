/**           _   _            _
 *          | | | |          (_)
 *          | |_| | ___  _ __ _ _______  _ __
 *          |  _  |/ _ \| '__| |_  / _ \| '_ \
 *          | | | | (_) | |  | |/ / (_) | | | |
 *          \_| |_/\___/|_|  |_/___\___/|_| |_|
 *
 * This file is part of Horizon (c).
 * Copyright (c) 2018 Horizon Dev Team.
 *
 * Base Author - Sagun Khosla. (sagunxp@gmail.com)
 *
 * Under a proprietary license this file is not for use
 * or viewing without permission.
 ****************************************************/

#include "Inter.hpp"
#include "Server/Inter/SocketMgr/ClientSocketMgr.hpp"

#include <yaml-cpp/yaml.h>
#include <boost/asio.hpp>
#include <iostream>
#include <boost/make_shared.hpp>

using namespace std;
using boost::asio::ip::udp;

/**
 * Inter Main server constructor.
 */
Horizon::Inter::InterMain::InterMain() : Server("Inter", "config/", "inter-server.yaml")
{
}

/**
 * Inter Main server destructor.
 */
Horizon::Inter::InterMain::~InterMain()
{
}

/**
 * Read /config/inter-server.yaml
  * @return true on success, false on failure.
  */
bool Horizon::Inter::InterMain::ReadConfig()
{
	YAML::Node config;
	std::string filepath = getGeneralConf().getConfigFilePath() + getGeneralConf().getConfigFileName();

	try {
		config = YAML::LoadFile(filepath);
	} catch (std::exception &err) {
		InterLog->error("Unable to read {}. ({})", filepath, err.what());
		return false;
	}

	if (config["Password"])
		getNetworkConf().setInterServerPassword(config["Password"].as<std::string>());

	/**
	 * Process Configuration that is common between servers.
	 */
	if (!ProcessCommonConfiguration(config))
		return false;

	InterLog->info("Done reading {} configurations in '{}'.", config.size(), getGeneralConf().getConfigFilePath() + getGeneralConf().getConfigFileName());

	return true;
}

bool Horizon::Inter::InterMain::CLICmd_SendAuthPacket()
{
	return true;
}
/**
 * Initialize Inter-Server CLI Commands.
 */
void Horizon::Inter::InterMain::InitializeCLICommands()
{
	Server::InitializeCLICommands();
}

/**
 * Signal Handler for the Inter-Server's main thread.
 * @param error
 */
void SignalHandler(const boost::system::error_code &error, int /*signalNumber*/)
{
	if (!error) {
		InterServer->shutdown(SIGINT);
	}
}

/**
 * Main entry point of the char-server application.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char * argv[])
{
	if (argc > 1)
		InterServer->ParseExecArguments(argv, argc);

	/*
	 * Read Configuration Settings for
	 * the Interacter Server.
	 */
	if (!InterServer->ReadConfig())
		exit(SIGTERM); // Stop process if the file can't be read.

	/**
	 * Core Signal Handler
	 */
	boost::asio::signal_set signals(*InterServer->getIOService(), SIGINT, SIGTERM);
	// Set signal handler for callbacks.
	// Set signal handlers (this must be done before starting io_service threads,
	// because otherwise they would unblock and exit)
	signals.async_wait(SignalHandler);

	// Start Interacter Network
	ClientSocktMgr->Start(*InterServer->getIOService(),
            InterServer->getNetworkConf().getListenIp(),
            InterServer->getNetworkConf().getListenPort(),
            InterServer->getNetworkConf().getMaxThreads());

	/**
	 * Initialize the Common Core
	 */
	InterServer->InitializeCore();

	/*
	 * Core Cleanup
	 */
	InterLog->info("Server shutting down...");

	/* Stop Network */
	ClientSocktMgr->StopNetwork();

	signals.cancel();

	return InterServer->getGeneralConf().getShutdownSignal();
}
