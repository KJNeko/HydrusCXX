//
// Created by kj16609 on 4/6/22.
//

#ifndef HYDRUSCXX_MASTERDB_HPP
#define HYDRUSCXX_MASTERDB_HPP

#include <sqlite_modern_cpp.h>
#include <vector>
#include <string>
#include <spdlog/spdlog.h>
#include <filesystem>

#include "HydrusCXXUtils.hpp"

class Master
{
public:
	sqlite::database db;

	std::vector<std::pair<size_t, size_t>> tags {};
	std::vector<std::string> subtags {};
	std::vector<std::string> namespaceTags {};
	std::vector<std::vector<std::string>> urls {};

	void loadTags();

	Master( std::filesystem::path path, bool preload = false ) : db( path )
	{
		spdlog::info( path.string() + " was opened in Master" );

		if ( preload )
		{
			loadTags();
			spdlog::info( "Preload finished for Master" );
		}
	}

	std::string getSubtag( size_t subtag_id );

	std::string getNamespace( size_t namespace_id );

	std::pair<std::string, std::string> getTagString( size_t tag_id );

	size_t getSubtagID( std::string str );

	size_t getNamespaceID( std::string str );

	size_t getTagID( size_t group, size_t subtag );

	size_t getTagID( std::string group, std::string subtag );
};


#endif //HYDRUSCXX_MASTERDB_HPP
