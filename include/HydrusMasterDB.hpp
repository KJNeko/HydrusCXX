//
// Created by kj16609 on 4/6/22.
//

#ifndef HYDRUSCXX_HYDRUSMASTERDB_HPP
#define HYDRUSCXX_HYDRUSMASTERDB_HPP

#include <sqlite_modern_cpp.h>
#include <vector>
#include <string>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace HydrusCXX {
	
	
	class Master
	{
	public:
		sqlite::database db;
		
		std::vector<std::pair<size_t, size_t>> tags {};
		std::vector<std::string> subtags {};
		std::vector<std::string> namespaceTags {};
		std::vector<std::vector<std::string>> urls {};
		
		std::mutex tagsLock {};
		std::mutex subtagsLock {};
		std::mutex namespaceLock {};
		std::mutex urlsLock {};
		
		
		void loadTags();
		
		explicit Master( const std::filesystem::path& path )
				:
				db( path )
		{
			spdlog::info( path.string() + " was opened as Master" );
		}
		
		std::string getSubtag( size_t subtag_id );
		
		std::string getNamespace( size_t namespace_id );
		
		std::pair<std::string, std::string> getTagString( size_t tag_id );
		
		size_t getSubtagID( const std::string& str );
		
		size_t getNamespaceID( const std::string& str );
		
		size_t getTagID( size_t group, size_t subtag );
		
		size_t getTagID( std::string group, std::string subtag );
		
		std::vector<std::string>
		getTagStrings( const std::vector<size_t>& tag_idList );
		
		void loadNamespaces();
		
		void loadSubtags();
		
		void loadURLs();
	};
	
}

#endif //HYDRUSCXX_HYDRUSMASTERDB_HPP
