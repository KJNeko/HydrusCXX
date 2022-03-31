#include "HydrusCXX.hpp"
#include "stopwatch.hpp"

#include <iostream>

int main()
{
	stopwatch::Stopwatch watch( "main" );
	watch.start();

	HydrusCXX db { "/home/kj16609/Desktop/Projects/hydrus/db/" };

	auto tag_id = db.master.getTagIDFromStringPair( "", "1girl" );

	if ( !tag_id.has_value())
	{
		throw std::runtime_error( "FUCK" );
	}

	auto imageList = db.mappings.getImageList( tag_id.value());

	std::cout << imageList.size() << std::endl;

	for ( const auto& hash_id : imageList )
	{
		auto tagList = db.mappings.getTagList( hash_id );
		for ( const auto& tag_id : tagList )
		{
			auto strPair = db.master.getTagPairString( tag_id );
			//std::cout << strPair.value().first << ":" << strPair.value().second << std::endl;
		}
	}

	watch.stop();
	std::cout << watch << std::endl;

}
