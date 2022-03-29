//
// Created by kj16609 on 3/29/22.
//

#define fixme throw std::runtime_error( std::string( __func__ ) + " in " + std::string( __FILE__ ) + " is incomplete." );

#include "HydrusCXX.hpp"
#include <cstring>

HydrusCXX::HydrusCXX(const std::filesystem::path &path)
		: client(path.string() + "client.db"), master(path.string() + "client.master.db"),
		  mappings(path.string() + "client.Mappings.db") {
}

/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
														 CLIENT
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
														 MASTER
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
														MAPPINGS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

uint Master::getHashIdFromHash(std::bitset<256>) {
	fixme
	return 0;
}

std::bitset<256> Master::getHash(uint id) {
	std::bitset<256> hash;

	db << "select hash from hashes where hash_id == " + std::to_string(id) >> [&](std::vector<unsigned char> blob) {
		memcpy(&hash, blob.data(), blob.size());
	};

	return hash;
}

uint Master::getIDFromMD5(std::bitset<MD5LENGTH> hash) {
	fixme
	return 0;
}

uint Master::getIDFromSHA1(std::bitset<SHA1LENGTH> hash) {
	fixme
	return 0;
}

uint Master::getIDFromSHA512(std::bitset<SHA512LENGTH> hash) {
	fixme
	return 0;
}

std::bitset<MD5LENGTH> Master::getMD5(uint id) {
	std::bitset<MD5LENGTH> hash;
	db << "select md5 from local_hashes where hash_id == " + std::to_string(id)
	   >> [&](std::vector<unsigned char> blob) {
		   memcpy(&hash, blob.data(), blob.size());
	   };


	return hash;
}

std::bitset<SHA1LENGTH> Master::getSHA1(uint id) {
	std::bitset<SHA1LENGTH> hash;

	db << "select sha1 from local_hashes where hash_id == " + std::to_string(id)
	   >> [&](std::vector<unsigned char> blob) {
		   memcpy(&hash, blob.data(), blob.size());
	   };

	return hash;
}

std::bitset<SHA512LENGTH> Master::getSHA512(uint id) {
	std::bitset<SHA512LENGTH> hash;

	db << "select sha512 from local_hashes where hash_id == " + std::to_string(id)
	   >> [&](std::vector<unsigned char> blob) {
		   memcpy(&hash, blob.data(), blob.size());
	   };

	return hash;
}

uint Master::getNamespaceIDFromString(std::string str) {
	uint temp;
	db << "select namespace_id from namespaces where namespace == \'" + str + "\'" >> [&](uint id) {
		temp = id;
	};
	return temp;
}

std::string Master::getNamespace(uint id) {
	std::string temp;
	db << "select namespace from namespaces where namespace_id == " + std::to_string(id) >> [&](std::string str) {
		temp = str;
	};
	return temp;
}

uint Master::getSubtagIDFromString(std::string str) {
	uint temp;
	db << "select subtag_id from subtags where subtag == \'" + str + "\'" >> [&](uint id) {
		temp = id;
	};
	return temp;
}

std::string Master::getSubtag(uint id) {
	std::string temp;
	db << "select subtag from subtags where subtag_id == " + std::to_string(id) >> [&](std::string subtag) {
		temp = subtag;
	};
	return temp;
}

uint Master::getTagIDFromPair(uint group_id, uint subtag_id) {
	uint temp;
	db << "select tag_id from tags where subtag_id == " + std::to_string(subtag_id) + " and namespace_id == " +
		  std::to_string(group_id) >>
	   [&](uint tag_id) {
		   temp = tag_id;
	   };
	return temp;
}

uint Master::getTagIdFromStringPair(std::string group, std::string subtag) {
	auto subtag_id = getSubtagIDFromString(subtag);
	auto namespace_id = getNamespaceIDFromString(group);

	return getTagIDFromPair(namespace_id, subtag_id);
}

std::pair<uint, uint> Master::getTagPair(uint tag_id) {
	std::pair<uint, uint> temp;
	db << "select namespace_id, subtag_id from tags where tag_id == " + std::to_string(tag_id)
	   >> [&](uint group, uint subtag) {
		   temp = std::pair(group, subtag);
	   };
	return temp;
}

std::pair<std::string, std::string> Master::getTagPairString(uint tag_id) {
	std::pair<std::string, std::string> temp;
	db << "select namespace_id, subtag_id from tags where tag_id == " + std::to_string(tag_id)
	   >> [&](uint namespace_id, uint subtag_id) {
		   temp = std::pair(getNamespace(namespace_id), getSubtag(subtag_id));
	   };
	return temp;
}
