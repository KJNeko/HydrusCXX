//
// Created by kj16609 on 4/6/22.
//

#ifndef HYDRUSCXX_HYDRUSMAPPINGDB_HPP
#define HYDRUSCXX_HYDRUSMAPPINGDB_HPP

#include <sqlite_modern_cpp.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <filesystem>

namespace HydrusCXX {
	
	class Mappings
	{
	public:
		sqlite::database db;
		
		std::vector<std::vector<size_t>> currentMappings {};
		std::unordered_map<size_t, size_t> hashToMemory {};
		std::unordered_map<size_t, size_t> memoryToHash {};
		
		
		std::vector<std::vector<size_t>> ptrMappings {};
		std::unordered_map<size_t, size_t> ptrHashToMemory {};
		std::unordered_map<size_t, size_t> ptrMemoryToHash {};
		
		
		std::mutex mappingLock {};
		
		void loadMappings();
		
		void loadPTR( bool filtered = true );
		
		
		explicit Mappings( const std::filesystem::path& path )
				:
				db( path )
		{
			spdlog::info( path.string() + " was opened as Mappings" );
		}
		
		std::vector<size_t> getTags( size_t hash, bool = true );
		
		std::vector<size_t> getHashesOnTag( size_t tag, bool = true );
		
		void sync();
	};
}

#endif //HYDRUSCXX_HYDRUSMAPPINGDB_HPP
