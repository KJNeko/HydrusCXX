//
// Created by kj16609 on 3/29/22.
//

#ifndef HYDRUSCXX_HYDRUSCXX_HPP
#define HYDRUSCXX_HYDRUSCXX_HPP


#include <bitset>
#include <filesystem>
#include <sqlite_modern_cpp.h>

class Client {
	sqlite::database db;

public:
	Client(std::filesystem::path path) : db(path) {
	}
};

#define MD5LENGTH 128
#define SHA1LENGTH 160
#define SHA512LENGTH 512

class Master {
	sqlite::database db;

public:
	Master(std::filesystem::path path) : db(path) {
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
	uint getHashIdFromHash(std::bitset<256> hash);

	std::bitset<256> getHash(uint id);

	// labels
	// NOT IMPLEMENTED

	// local_hashes
	uint getIDFromMD5(std::bitset<MD5LENGTH> hash);

	uint getIDFromSHA1(std::bitset<SHA1LENGTH> hash);

	uint getIDFromSHA512(std::bitset<SHA512LENGTH> hash);

	std::bitset<MD5LENGTH> getMD5(uint id);

	std::bitset<SHA1LENGTH> getSHA1(uint id);

	std::bitset<SHA512LENGTH> getSHA512(uint id);

	// namespaces
	uint getNamespaceIDFromString(std::string);

	std::string getNamespace(uint id);

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
	uint getSubtagIDFromString(std::string);

	std::string getSubtag(uint id);

	// tags
	uint getTagIDFromPair(uint, uint);

	uint getTagIdFromStringPair(std::string, std::string);

	std::pair<uint, uint> getTagPair(uint);

	std::pair<std::string, std::string> getTagPairString(uint);

	// texts
	// NOT IMPLEMENTED

	// url_domains
	// NOT IMPLEMENTED

	// urls
	// NOT IMPLEMENTED
};


class Mappings {
	sqlite::database db;

public:
	Mappings(std::filesystem::path path) : db(path) {
	}
};

class HydrusCXX {
public:
	// Contains all the basic DB operations/info
	Mappings mappings;
	Master master;
	Client client;

	HydrusCXX() = delete;

	HydrusCXX(const std::filesystem::path &path);
};


#endif // HYDRUSCXX_HYDRUSCXX_HPP
