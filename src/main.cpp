
#include <filesystem>
#include <iostream>
#include <string>

#include "HydrusCXX.hpp"
#include <ringBuffer.hpp>

int main()
{

	HydrusCXX db { "/home/kj16609/Desktop/Projects/hydrus/db/" };

	auto ref = db.master;

	auto tag_id = ref.getSubtagIDFromString( "toujou koneko" );

	std::cout << tag_id << std::endl;

	std::cout << ref.getSubtag( tag_id ) << std::endl;

	auto tagPairID = ref.getTagIdFromStringPair( "character", "toujou koneko" );
	std::cout << tagPairID << std::endl;
}
