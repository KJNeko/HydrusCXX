#include "HydrusCache.hpp"
#include "HydrusDB.hpp"
#include "Extras/stopwatch.hpp"


int main()
{
	DB master(
			"/home/kj16609/Desktop/Projects/hydrus/db/client.master.db" );
	DB mappings(
			"/home/kj16609/Desktop/Projects/hydrus/db/client.mappings.db" );
	
	Query<true, size_t, false, std::string> subtags(
			&master, "select subtag from subtags where subtag_id == ?" );
	
	Query<true, size_t, false, std::string> namespaces(
			&master,
			"select namespace from namespaces where namespace_id == ?" );
	
	Query<true, size_t, false, size_t, size_t> tags(
			&master,
			"select namespace_id, subtag_id from tags where tag_id == ?" );
	
	Query<true, size_t, true, size_t> tagMappings(
			&mappings,
			"select tag_id from current_mappings_8 where hash_id == ?", true );
	
	Query<false, size_t, true, size_t> getMappingsHashID(
			&mappings,
			"select hash_id from current_mappings_8 group by hash_id limit 100000000" );
	
	
	auto getTagFor = [&]( size_t i )
	{
		std::optional<std::vector<size_t>> tagList = tagMappings.get(
				i );
		
		if ( tagList.has_value())
		{
			for ( auto tag_id : tagList.value())
			{
				std::optional<std::tuple<size_t, size_t>> tag = tags.get(
						tag_id );
				
				if ( tag.has_value())
				{
					size_t namespace_ID = std::get<0>( tag.value());
					size_t subtag_ID = std::get<1>( tag.value());
					
					std::optional<std::string> subtag = subtags.get(
							subtag_ID );
					
					if ( subtag.has_value())
					{
						std::optional<std::string> namespace_ = namespaces.get(
								namespace_ID );
						
						if ( namespace_.has_value())
						{
							/*std::cout << namespace_.value() << ":"
									  << subtag.value() << std::endl;*/
						}
					}
				}
			}
		}
	};
	
	std::optional<std::vector<size_t>> list = getMappingsHashID.get( 0 );
	
	size_t count { 512 };
	
	
	for ( size_t i = 0; i < 30; ++i )
	{
		auto start = std::chrono::high_resolution_clock::now();
		for ( size_t i = 0; i < count; ++i )
		{
			size_t randIndex = rand() % list.value().size();
			getTagFor( list.value()[randIndex] );
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = end - start;
		auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				duration );
		
		std::cout << duration_ms.count() << "\t" << subtags.cacheSize() << "\t"
				  << namespaces.cacheSize() << "\t" << tags.cacheSize() << "\t"
				  << tagMappings.cacheSize() << std::endl;
	}
	
	
	return 0;
}
