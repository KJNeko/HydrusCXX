//
// Created by kj16609 on 3/29/22.
//

#ifndef HYDRUSCXX_HYDRUSCXX_HPP
#define HYDRUSCXX_HYDRUSCXX_HPP


#include <bitset>
#include <filesystem>
#include <unordered_map>
#include <sqlite3.h>
#include <optional>
#include <vector>


class Client
{
	sqlite3* db;

public:
	Client( std::filesystem::path path )
	{
		sqlite3_open( path.c_str(), &db );
	}
};

#define MD5LENGTH 128
#define SHA1LENGTH 160
#define SHA512LENGTH 512

class Master
{
	sqlite3* db;


	std::unordered_map<uint, std::string> subtagMap;

	std::unordered_map<uint, std::string> namespaceMap;

	std::unordered_map<uint, std::pair<uint, uint>> tagMap;


	//Prepared statements
	//"select namespace_id, subtag_id from tags where tag_id == ?";

public:
	Master( std::filesystem::path path )
	{
		sqlite3_open( path.c_str(), &db );
	}

	/* [IMPLEMENTED = Y/N]TABLES
	 * [Y]hashes
	 * [N]labels
	 * [Y]local_hashes
	 * [Y]namespaces
	 * [N]notes
	 * [N]shape_perceptual_hash_map
	 * [N]shape_perceptual_hashes
	 * [N]sqlite_master
	 * [N]sqlite_stat1
	 * [Y]subtags
	 * [Y]tags
	 * [N]texts
	 * [N]url_domains
	 * [N]urls
	 */


	// hashes
	uint getHashIdFromHash( std::bitset<256> hash );

	std::bitset<256> getHash( uint id );

	// labels
	// NOT IMPLEMENTED

	// local_hashes
	uint getIDFromMD5( std::bitset<MD5LENGTH> hash );

	uint getIDFromSHA1( std::bitset<SHA1LENGTH> hash );

	uint getIDFromSHA512( std::bitset<SHA512LENGTH> hash );

	std::bitset<MD5LENGTH> getMD5( uint id );

	std::bitset<SHA1LENGTH> getSHA1( uint id );

	std::bitset<SHA512LENGTH> getSHA512( uint id );

	// namespaces
	std::optional<uint> getNamespaceIDFromString( const std::string& group );

	std::optional<std::string> getNamespace( uint namespace_id );

	// notes
	// NOT IMPLEMENTED

	// shape_perceptual_hash_map
	// NOT IMPLEMENTED

	// shape_perceptual_hahses
	// NOT IMPLEMENTED

	// sqlite_master
	// NOT IMPLEMENTED

	// sqlite_stat1
	// NOT IMPLEMENTED

	// subtags
	std::optional<uint> getSubtagIDFromString( std::string );

	std::optional<std::string> getSubtag( uint subtag_id );

	// tags
	std::optional<uint> getTagIDFromPair( uint namespace_id, uint subtag_id );

	std::optional<uint> getTagIDFromStringPair( const std::string& group, const std::string& subtag );

	std::optional<std::pair<uint, uint>> getTagPair( uint tag_id );

	std::optional<std::pair<std::string, std::string>> getTagPairString( const uint tag_id );

	// texts
	// NOT IMPLEMENTED

	// url_domains
	// NOT IMPLEMENTED

	// urls
	// NOT IMPLEMENTED
};


class Mappings
{
	sqlite3* db;

	//sqlite::database_binder preparedGetImageList = db << "select hash_id from current_mappings_8 where tag_id == ?";
	//sqlite::database_binder preparedGetTagList = db << "select tag_id from current_mappings_8 where hash_id == ?";

public:
	Mappings( std::filesystem::path path )
	{
		sqlite3_open( path.c_str(), &db );
	}

	std::vector<uint> getImageList( uint tag_id );

	std::vector<uint> getTagList( uint hash_id );
};

class HydrusCXX
{
public:
	// Contains all the basic DB operations/info
	Mappings mappings;
	Master master;
	Client client;

	HydrusCXX() = delete;

	HydrusCXX( const std::filesystem::path& path );
};


#endif // HYDRUSCXX_HYDRUSCXX_HPP
