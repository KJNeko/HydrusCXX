#include "HydrusCXX.hpp"
#include "stopwatch.hpp"

#include <iostream>
#include <cmath>

std::string format( int64_t count )
{
	if ( count < 1000 )
	{
		return std::to_string( count ) + "B";
	}
	if ( count / 1000 > 999 )
	{
		if ( count / 1000 / 1000 > 999 )
		{
			return std::to_string( count / 1000 / 1000 / 1000 ) + "GB";
		}
		else
		{
			return std::to_string( count / 1000 / 1000 ) + "MB";
		}
	}
	else
	{
		return std::to_string( count / 1000 ) + "KB";
	}

}

void formatMemory( Mappings& mappingDB, Master& masterDB )
{
	size_t currentMappings { 0 };
	size_t tagPair { 0 };
	size_t namespaceTags { 0 };
	size_t subtags { 0 };

	for ( const auto& list : mappingDB.currentMappings )
	{
		currentMappings += list.size() * sizeof( size_t );
	}

	tagPair += masterDB.tags.size() * ( sizeof( size_t ) * 2 );

	for ( const auto& str : masterDB.namespaceTags )
	{
		namespaceTags += str.size();

	}

	for ( const auto& str : masterDB.subtags )
	{
		subtags += str.size();
	}

	size_t total { currentMappings + tagPair + namespaceTags + subtags };
	std::cout << "Total memory usage: " << format( static_cast<int64_t>(total)) << std::endl;


	std::cout << "Formatted memory usage: \n";

	std::cout << "Mapping memory usage: " << format( static_cast<int64_t>(currentMappings)) << std::endl;
	std::cout << "Tag Pair memory usage: " << format( static_cast<int64_t>(tagPair)) << std::endl;
	std::cout << "Namespace memory usage: " << format( static_cast<int64_t>(namespaceTags)) << std::endl;
	std::cout << "Subtag memory usage: " << format( static_cast<int64_t>(subtags)) << std::endl;

	return;
}

void massTagTest( Mappings& map, Master& master )
{
	std::cout << "Begining large tag fetch of tag '1girl' and all images with the tag" << std::endl;
	stopwatch::Stopwatch watch( "massTagTest" );
	watch.start();

	//Go fetch a tag
	size_t tag = master.getTagID( "", "1girl" );

	//Fetch every image attached to the tag
	auto imageList = map.getHashesOnTag( tag );
	watch.stop();
	std::cout << watch << std::endl;
	std::cout << "Number of images returned: " << imageList.size() << std::endl;

}

int main()
{
	stopwatch::Stopwatch watch( "main" );
	watch.start();

	Mappings mappingDB { "/home/kj16609/Desktop/Projects/hydrus/db/client.mappings.db" };

	Master masterDB { "/home/kj16609/Desktop/Projects/hydrus/db/client.master.db" };


	mappingDB.loadMappings();


	masterDB.loadTags();


	watch.stop();
	std::cout << watch << std::endl;


	stopwatch::Stopwatch testing( "testing" );
	testing.start();

	auto pair = masterDB.getTagString( 4 );
	//std::cout << pair.first << ":" << pair.second << std::endl;


	std::cout << masterDB.getSubtag( 64000 ) << std::endl;

	size_t id = masterDB.getSubtagID( masterDB.getSubtag( 64000 ));

	testing.stop();

	std::cout << id << std::endl;


	std::cout << testing << std::endl;

	formatMemory( mappingDB, masterDB );
	massTagTest( mappingDB, masterDB );

	return 0;
}
