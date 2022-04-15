#include "HydrusCache.hpp"
#include "HydrusDB.hpp"
#include "Extras/stopwatch.hpp"

int main()
{
	DB master(
			"/home/kj16609/Desktop/Projects/hydrus/db/client.master.db" );
	DB mappings(
			"/home/kj16609/Desktop/Projects/hydrus/db/client.mappings.db" );
	
	Cache<size_t, false, std::string> subtags(
			&master, "select subtag from subtags where subtag_id == {}" );
	
	Cache<size_t, false, std::string> namespaces(
			&master,
			"select namespace from namespaces where namespace_id == {}" );
	
	Cache<size_t, false, size_t, size_t> tags(
			&master,
			"select namespace_id, subtag_id from tags where tag_id == {}" );
	
	Cache<size_t, true, size_t> tagMappings(
			&mappings,
			"select tag_id from current_mappings_8 where hash_id == {}", true );
	
	size_t count = 512 * 512;
	stopwatch::Stopwatch watch( "First Run: " + std::to_string( count ));
	watch.start();
	for ( size_t i = 1; i < count; ++i )
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
	}
	
	watch.stop();
	
	size_t missCounter { 0 };
	
	stopwatch::Stopwatch watch2( "Second Run: " + std::to_string( count ));
	watch2.start();
	for ( size_t i = 1; i < count; ++i )
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
		else
		{
			missCounter++;
		}
	}
	watch2.stop();
	
	std::cout << watch << std::endl;
	std::cout << watch2 << std::endl;
	
	std::cout << "missCounter: " << missCounter << std::endl;
	
	std::cout << "Subtags cached: " << subtags.cacheSize() << std::endl;
	std::cout << "Namespaces cached: " << namespaces.cacheSize() << std::endl;
	std::cout << "Tags cached: " << tags.cacheSize() << std::endl;
	return 0;
}
