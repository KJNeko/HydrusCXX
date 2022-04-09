//
// Created by kj16609 on 4/6/22.
//

#ifndef HYDRUSCXX_HYDRUSCXXMAPPINGDB_HPP
#define HYDRUSCXX_HYDRUSCXXMAPPINGDB_HPP

#include <sqlite_modern_cpp.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <filesystem>

#include "HydrusCXXUtils.hpp"

class Mappings
{
public:
	sqlite::database db;
	
	std::vector<std::vector<size_t>> currentMappings {};
	
	std::unordered_map<size_t, size_t> hashToMemory {};
	std::unordered_map<size_t, size_t> memoryToHash {};
	
	void loadMappings();
	
	void loadPTR( bool filtered = true );
	
	
	Mappings( std::filesystem::path path )
			:
			db( path )
	{
		spdlog::info( path.string() + " was opened as Mappings" );
	}
	
	std::vector<size_t> getTags( size_t hash );
	
	std::vector<size_t> getHashesOnTag( size_t tag );
	
};


#endif //HYDRUSCXX_HYDRUSCXXMAPPINGDB_HPP
