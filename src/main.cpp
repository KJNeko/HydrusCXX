#define PTRSUPPORTMEMORY

#include "HydrusCXX.hpp"
#include "include/Extras/stopwatch.hpp"

#include <cmath>
#include <iostream>

#include <spdlog/spdlog.h>

void massTagTest( Mappings& map, Master& master )
{
	
	std::string namespaceTag { "" };
	std::string subTag { "1girl" };
	
	spdlog::debug(
			"Begining large tag fetch of images with \'" + namespaceTag + ":" +
			subTag + "\' and all tags attached to those images" );
	stopwatch::Stopwatch watch( "massTagTest: imageFetch" );
	watch.start();
	
	// Go fetch a tag
	size_t tag = master.getTagID( namespaceTag, subTag );
	
	// Fetch every image attached to the tag
	std::vector<size_t> imageList = map.getHashesOnTag( tag );
	watch.stop();
	
	
	// Fetch tag attached to the images
	stopwatch::Stopwatch watch2( "massTagTest: tagFetch" );
	watch2.start();
	
	std::vector<std::string> strs;
	
	for ( const size_t& image : imageList )
	{
		std::vector<size_t> tagList = map.getTags( image );
		
		for ( const size_t& item : tagList )
		{
			std::pair<std::string, std::string> pair = master.getTagString(
					item );
			
			strs.push_back( pair.first + ":" + pair.second );
		}
	}
	watch2.stop();
	
	std::stringstream ss;
	ss << watch;
	spdlog::debug( ss.str());
	spdlog::debug(
			"Number of images returned: " + std::to_string( imageList.size()));
	
	std::stringstream ss2;
	ss2 << watch2;
	spdlog::debug( ss2.str());
	spdlog::debug( "Number of tags returned: " + std::to_string( strs.size()));
}

void singleTest( Mappings& map, Master& master )
{
	stopwatch::Stopwatch watch( "singleTest: imageFetch" );
	size_t hash_id { 1337 };
	
	watch.start();
	std::vector<size_t> tagList = map.getTags( hash_id );
	
	std::vector<std::string> strs = master.getTagStrings( tagList );
	
	watch.stop();
	
	
	std::stringstream ss;
	ss << watch;
	spdlog::debug( ss.str());
	spdlog::debug(
			"Number of tags returned: " + std::to_string( tagList.size()));
}

int main()
{
	//Testing hashes
	
	std::vector<char> hash;
	hash.resize( 256 / 8 );
	
	spdlog::info( "Starting HydruCXX" );
	
	
	
	
	// JsonParser ptr;
	// ptr.parse(
	//		"/home/kj16609/Desktop/Projects/hydrusCXX/7f59a664fb4464f0d8cf63b1a0ea560743c009e20845b37894c1d72d8086451c" );
	
	// ptr.parse(
	//		"/home/kj16609/Desktop/Projects/hydrusCXX/7f062a8810ad3cb0a52cbaa4b864a92030e600cc413646bc255ae85a95693bea" );
	
	stopwatch::Stopwatch watch( "Load all data into memory" );
	watch.start();
	
	HydrusCXX db( "/home/kj16609/Desktop/Projects/hydrus/db" );
	
	watch.stop();
	
	std::stringstream ss;
	ss << watch;
	spdlog::info( ss.str());
	
	massTagTest( db.mappings, db.master );
	singleTest( db.mappings, db.master );
	
	return 0;
}
