//
// Created by kj16609 on 4/6/22.
//

#ifndef HYDRUSCXX_HYDRUSMAINDB_HPP
#define HYDRUSCXX_HYDRUSMAINDB_HPP

#include <sqlite_modern_cpp.h>
#include <vector>
#include <string>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace HydrusCXX {
	
	class Main
	{
	public:
		sqlite::database db;
		
		std::vector<std::pair<size_t, size_t>> siblings {};
		std::vector<std::pair<size_t, size_t>> parents {};
		
		std::mutex siblingLock {};
		std::mutex parentLock {};
		
		explicit Main( const std::filesystem::path& path )
				:
				db( path )
		{
			spdlog::info( path.string() + " was opened as Main" );
		}
		
		void loadSiblings();
		
		void loadParents();
		
		std::vector<size_t> getParents( size_t id );
		
		size_t getSibling( size_t id );
		
		size_t recursiveSibling( size_t id );
		
		void sync();
		
	};
}

#endif //HYDRUSCXX_HYDRUSMAINDB_HPP
