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
#include "Auth.hpp"
#include "Server/Auth/SocketMgr/InterSocketMgr.hpp"
#include "Server/Auth/SocketMgr/ClientSocketMgr.hpp"

#include <yaml-cpp/yaml.h>
#include <boost/asio.hpp>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>
#include <Server/Common/Server.hpp>

using boost::asio::ip::udp;
using namespace std::chrono_literals;

/**
 * AuthMain Constructor.
 */
Horizon::Auth::AuthMain::AuthMain()
: Server("Auth", "config/", "auth-server.yaml")
{
	InitializeCLICommands();
}

/**
 * AuthMain Destructor.
 */
Horizon::Auth::AuthMain::~AuthMain()
{
}

/**
 * Read /config/auth-server.yaml
 * @return true on success, false on failure.
 */
bool Horizon::Auth::AuthMain::ReadConfig()
{
	YAML::Node config;
	std::string filepath = getGeneralConf().getConfigFilePath() + getGeneralConf().getConfigFileName();

	try {
		config = YAML::LoadFile(filepath);
	} catch (std::exception &err) {
		AuthLog->error("Unable to read {}. ({})", filepath, err.what());
		return false;
	}

	try {
		/**
		 * Inter Server Settings
		 * @brief Definitions of the Inter-server networking configuration.
		 */
		if (config["InterServer.IP"]) {
			getNetworkConf().setInterServerIp(config["InterServer.IP"].as<std::string>());
		} else {
			AuthLog->error("Inter-server IP configuration not set, defaulting to '127.0.0.1'.");
			getNetworkConf().setInterServerIp("127.0.0.1");
		}

		if (config["InterServer.Port"]) {
			getNetworkConf().setInterServerPort(config["InterServer.Port"].as<uint16_t>());
		} else {
			AuthLog->error("Inter-server password not set.");
		}

		if (config["InterServer.Password"]) {
			getNetworkConf().setInterServerPassword(config["InterServer.Password"].as<std::string>());
		}

		AuthLog->info("Outbound connections: Inter-Server configured to tcp://{}:{} {}",
		            getNetworkConf().getInterServerIp(), getNetworkConf().getInterServerPort(),
					(getNetworkConf().getInterServerPassword().length()) ? "using password" : "not using password");

		/**
		 * Additional Configuration
		 * @brief
		 */
		if (config["pass_hash_method"]) {
			HashingMethods hashingMethod = static_cast<HashingMethods>(config["pass_hash_method"].as<int>());
			if (hashingMethod < PASS_HASH_NONE || hashingMethod > PASS_HASH_BCRYPT) {
				AuthLog->warn("Incorrect value {} given for 'pass_hash_method', defaulting to 0.", hashingMethod);
				getAuthConfig().setPassHashMethod(PASS_HASH_NONE);
			} else {
				getAuthConfig().setPassHashMethod(hashingMethod);
			}
		}

		if (this->getAuthConfig().getPassHashMethod() == PASS_HASH_NONE)
			AuthLog->warn("Passwords are stored in plain text! This is not recommended!");

		/**
		 * Date Format for clients
		 * @brief
		 */
		if (config["client_date_format"])
			getAuthConfig().setClientDateFormat(config["client_date_format"].as<std::string>());

		if (config["allowed_client_version"])
			getAuthConfig().setAllowedClientVersion(config["allowed_client_version"].as<uint32_t>());

		/**
		 * Logging Configuration
		 */
		if (config["log"] && config["log"].as<bool>()) {
			getAuthConfig().getLogConf().enable();

			if (config["login_max_tries"])
				getAuthConfig().getLogConf().setLoginMaxTries(config[ "login_max_tries" ].as<uint32_t>());

			if (config["login_fail_ban_time"])
				getAuthConfig().getLogConf().setLoginFailBanTime(config["login_fail_ban_time"].as<time_t>());

			if (config["login_fail_check_time"])
				getAuthConfig().getLogConf().setLoginFailCheckTime(config["login_fail_check_time"].as<time_t>());
		}

		AuthLog->info("Logging is {}.", getAuthConfig().getLogConf().isEnabled() ? "enabled" : "disabled");
		AuthLog->info("Failed logins exceeding {} tries every {} seconds will be banned for {} seconds.",
		              getAuthConfig().getLogConf().getLoginFailCheckTime(), getAuthConfig().getLogConf().getLoginMaxTries(), getAuthConfig().getLogConf().getLoginFailBanTime());

		/**
		 * Character Servers.
		 * @brief configuration for the connection to character servers.
		 */
		if (config["character_servers"]) {
			YAML::Node n = config["character_servers"];
			if (n.IsMap()) {
				for (std::size_t i = 1; i <= n.size(); ++i) {
					YAML::Node nn = n[i];
					character_server_data char_serv;
					if (nn.IsMap()) {
						if (nn["name"]) {
							char_serv.name = nn["name"].as<std::string>();
						} else {
							AuthLog->warn("Name for character server at index '{}' not found, defaulting to 'Horizon_{}'.", i, i);
							char_serv.name = "Horizon_" + std::to_string(i);
						}
						if (nn["host"]) {
							char_serv.ip_address = nn["host"].as<std::string>();
						} else {
							AuthLog->warn("IP Address for character server '{}' not found, defaulting to 'localhost'.", char_serv.name);
							char_serv.ip_address = "localhost";
						}
						if (nn["port"]) {
							char_serv.port = nn["port"].as<uint16_t>();
						} else {
							AuthLog->warn("Port for character server '{}' not found, defaulting to '{}'.", char_serv.name, 6121);
							char_serv.port = 6121;
						}
						if (nn["isNew"]) {
							char_serv.is_new = (int16_t) (nn["isNew"].as<bool>() ? 1 : 0);
						}
						if (nn["type"]) {
							char_serv.server_type = static_cast<character_server_types>(nn["type"].as<uint16_t>());
							if (char_serv.server_type >= CHAR_SERVER_TYPE_MAX) {
								AuthLog->error(
									"Incorrect character server type '{}' given for server #{}, defaulting to 'normal type'.",
									i, (int) char_serv.server_type);
								char_serv.server_type = CHAR_SERVER_TYPE_NORMAL;
							}
						}
						char_serv.id = (int) i;
						addCharacterServer(char_serv); // Add the server in.
						AuthLog->info("Configured Character Server: {}@{}:{} {}", char_serv.name, char_serv.ip_address, char_serv.port, char_serv.is_new ? "(new)" : "");
					} else {
						AuthLog->warn("Invalid configuration type for sequence {} in 'character_servers', required map.", i);
					}
				}
			} else {
				AuthLog->warn("Invalid configuration type for character_servers, required nested maps.");
			}
		} else {
			AuthLog->warn("Configurations for character servers were not found!");
		}
	} catch (YAML::Exception &e) {
		AuthLog->error("YAML Parse Error: {}", e.what());
		return false;
	}

	/**
	 * Process Configuration that is common between servers.
	 */
	if (!ProcessCommonConfiguration(config))
		return false;

	AuthLog->info("Done reading {} configurations in '{}'.", config.size(), getGeneralConf().getConfigFilePath() + getGeneralConf().getConfigFileName());

	return true;
}

/**
 * CLI Command: Reload Configuration
 * @return boolean value from AuthServer->ReadConfig()
 */
bool Horizon::Auth::AuthMain::CLICmd_ReloadConfig()
{
	// Clear all character server info before reloading.
	_character_servers.clear();

	AuthLog->info("Reloading configuration from '{}'.", getGeneralConf().getConfigFileName());

	return AuthServer->ReadConfig();
}

/**
 * Initialize CLI Comamnds
 */
void Horizon::Auth::AuthMain::InitializeCLICommands()
{
	addCLIFunction("reloadconf", std::bind(&AuthMain::CLICmd_ReloadConfig, this));

	Server::InitializeCLICommands();
}

void Horizon::Auth::AuthMain::InitializeCore()
{
	/**
	 * Establish a connection to the inter-server.
	 */
	std::thread inter_conn_thread(std::bind(&AuthMain::ConnectWithInterServer, this));

	// Initialize Main Core.
	Server::InitializeCore();

	// Join connection thread on end.
	inter_conn_thread.join();
}

/**
 * Connect with Inter Server
 */
void Horizon::Auth::AuthMain::ConnectWithInterServer()
{
	if (!getGeneralConf().isTestRun()) {
		try {
			InterSocktMgr->Start(INTER_SESSION_NAME, this, getNetworkConf().getInterServerIp(), getNetworkConf().getInterServerPort(), 1);
		} catch (boost::system::system_error &e) {
		}
	}
}

/**
 * Signal Handler from the main thread.
 * @param ioServiceRef
 * @param error
 */
void SignalHandler(std::weak_ptr<boost::asio::io_service> &ioServiceRef, const boost::system::error_code &error, int /*signal*/)
{
	if (!error) {
		if (std::shared_ptr<boost::asio::io_service> io_service = ioServiceRef.lock())
			io_service->stop();
		AuthServer->shutdown(SIGINT);
	}
}

/**
 * Main Runtime Method
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char * argv[])
{
	if (argc > 1)
		AuthServer->ParseExecArguments(argv, argc);

	/*
	 * Read Configuration Settings for
	 * the Authentication Server.
	 */
	if (!AuthServer->ReadConfig())
		exit(SIGTERM); // Stop process if the file can't be read.

	/**
	 * Core Signal Handler
	 */
	boost::asio::signal_set signals(*AuthServer->getIOService(), SIGINT, SIGTERM);
	// Set signal handler for callbacks.
	// Set signal handlers (this must be done before starting io_service threads,
	// because otherwise they would unblock and exit)
	signals.async_wait(std::bind(&SignalHandler, std::weak_ptr<boost::asio::io_service>(AuthServer->getIOService()), std::placeholders::_1, std::placeholders::_2));

	// Start Auth Network
	ClientSocktMgr->Start(*AuthServer->getIOService(),
		AuthServer->getNetworkConf().getListenIp(),
		AuthServer->getNetworkConf().getListenPort(),
		AuthServer->getNetworkConf().getMaxThreads());

	/**
	 * Initialize the Common Core
	 */
	AuthServer->InitializeCore();

	/*
	 * Core Cleanup
	 */
	AuthLog->info("Server shutting down...");

	/* Stop Network */
	ClientSocktMgr->StopNetwork();
	InterSocktMgr->StopNetwork();

	// Cancel signal handling.
	signals.cancel();

	return AuthServer->getGeneralConf().getShutdownSignal();
}
