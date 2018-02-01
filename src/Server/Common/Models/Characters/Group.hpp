//
// Created by SagunKho on 01/02/2018.
//

#ifndef HORIZON_MODELS_CHARACTERS_GROUP_HPP
#define HORIZON_MODELS_CHARACTERS_GROUP_HPP

#include "Server/Common/Horizon.hpp"
#include "Server/Common/Server.hpp"
#include "Core/Database/MySqlConnection.hpp"

namespace Horizon
{
namespace Models
{
namespace Characters
{
class Group
{
public:
	Group() {}
	~Group() {}

	/**
	 * Load all fields from the database into this instance.
	 * @param server
	 * @param char_id
	 * @return
	 */
	bool LoadFromDatabase(Server *server, uint32_t char_id)
	{
		std::string query = "SELECT * FROM character_group_data WHERE char_id = ?";
		auto sql = server->MySQLBorrow();
		bool ret = false;

		try {
			sql::PreparedStatement *pstmt = sql->sql_connection->prepareStatement(query);
			pstmt->setInt(1, char_id);
			sql::ResultSet *res = pstmt->executeQuery();

			if (res != nullptr && res->next()) {
				/**
				 * Create Game Account Data
				 */
				setCharacterId(char_id);
				setPartyId(res->getUInt("party_id"));
				setGuildId(res->getUInt("guild_id"));
				ret = true;
			}

			delete res;
			delete pstmt;
		} catch (sql::SQLException &e) {
			DBLog->error("Models::Characters::Group::LoadFromDatabase: {}", e.what());
		}

		server->MySQLUnborrow(sql);

		return ret;
	}

	/* Character ID */
	uint32_t getCharacterId() const { return character_id; }
	void setCharacterId(uint32_t character_id) { Group::character_id = character_id; }
	/* Party ID */
	uint32_t getPartyId() const { return party_id; }
	void setPartyId(uint32_t party_id) { Group::party_id = party_id; }
	/* Guild ID */
	uint32_t getGuildId() const { return guild_id; }
	void setGuildId(uint32_t guild_id) { Group::guild_id = guild_id; }

private:
	uint32_t character_id;
	uint32_t party_id;
	uint32_t guild_id;
};
}
}
}
#endif // HORIZON_MODELS_CHARACTERS_GROUP_HPP
