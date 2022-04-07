//
// Created by kj16609 on 4/6/22.
//

#ifndef HYDRUSCXX_MAINDB_HPP
#define HYDRUSCXX_MAINDB_HPP

#include <sqlite_modern_cpp.h>
#include <vector>
#include <string>
#include <spdlog/spdlog.h>
#include <filesystem>

#include "HydrusCXXUtils.hpp"

class Main
{
public:
	sqlite::database db;

	std::vector<std::pair<size_t, size_t>> siblings {};
	std::vector<std::pair<size_t, size_t>> parents {};

	Main( std::filesystem::path path, bool preload = false ) : db( path )
	{
		spdlog::info( path.string() + " was loaded as Main" );
		if ( preload )
		{
			loadSiblings();
			loadParents();
			spdlog::info( "Preload finished for Main" );
		}
	}

	void loadSiblings();

	void loadParents();

	std::vector<size_t> getParents( size_t id );

	size_t getSibling( size_t id );

	size_t recursiveSibling( size_t id );

};


#endif //HYDRUSCXX_MAINDB_HPP
