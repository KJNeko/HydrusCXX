//
// Created by kj16609 on 4/6/22.
//

#include "HydrusCXXMappingDB.hpp"

void Mappings::loadMappings()
{
	spdlog::debug( "Loading mappings" );
	//Get a max count
	db << "select hash_id from current_mappings_8 order by hash_id DESC limit 1"
	   >> [&]( uint32_t hashCount )
	   {
		   currentMappings.reserve( hashCount + 1 );
	   };
	
	//Resize the internal vector to the size of the tags per image
	db << "select hash_id, count(*) from current_mappings_8 group by hash_id"
	   >> [&]( uint32_t hash_id, uint32_t count )
	   {
		   currentMappings.emplace_back( std::vector<size_t>());
		   currentMappings.back().reserve( count );
		   hashToMemory.emplace( hash_id, currentMappings.size() - 1 );
		   memoryToHash.emplace( currentMappings.size() - 1, hash_id );
	   };
	
	db << "select hash_id, tag_id from current_mappings_8"
	   >> [&]( uint32_t hash_id, uint32_t tag_id )
	   {
		   auto ret = hashToMemory.find( hash_id );
		   if ( ret != hashToMemory.end())
		   {
			   currentMappings.at( ret->second ).push_back( tag_id );
		   }
	   };
	
	currentMappings.shrink_to_fit();
	
	//Analyze the memory and print it to the debug log
	size_t count { 0 };
	
	count += sizeof( std::vector<std::vector<size_t>> );
	count += sizeof( std::vector<size_t> ) * currentMappings.size();
	for ( const auto& v : currentMappings )
	{
		count += sizeof( size_t ) * v.size();
	}
	
	count += sizeof( std::unordered_map<size_t, size_t> ) * 2;
	count += (( sizeof( std::pair<size_t, size_t> ) * hashToMemory.size()) *
			  2 );
	spdlog::info( "Loaded {} mappings", currentMappings.size());
}

void Mappings::loadPTR( bool filtered )
{
	spdlog::debug( "Loading PTR Mappings" );
	//Load tags of images that are matched
	
	if ( filtered )
	{
		
		db
				<< "select hash_id, tag_id from current_mappings_14 where hash_id in (select hash_id from current_mappings_8)"
				>> [&]( uint32_t hash_id, uint32_t tag_id )
				{
					//Check to see if we already have the tag
					
					auto ret = hashToMemory.find( hash_id );
					if ( ret == hashToMemory.end())
					{
						//Add the hash to the mapping
						currentMappings.emplace_back( std::vector<size_t>());
						hashToMemory.emplace(
								hash_id, currentMappings.size() - 1 );
						memoryToHash.emplace(
								currentMappings.size() - 1, hash_id );
						currentMappings.back().push_back( tag_id );
					}
					else
					{
						for ( auto& map : currentMappings.at( ret->second ))
						{
							if ( map == tag_id )
							{
								return;
							}
						}
						
						currentMappings.at( ret->second ).push_back( tag_id );
					}
					
					if ( hash_id % 250000 == 0 )
					{
					}
				};
	}
	else
	{
		spdlog::warn(
				"loadPTR was told to load ALL tags from PTR. This could take awhile and increase memory size" );
		
		db << "select hash_id, tag_id from current_mappings_14"
		   >> [&]( uint32_t hash_id, uint32_t tag_id )
		   {
			   auto ret = hashToMemory.find( hash_id );
			   if ( ret == hashToMemory.end())
			   {
				   //Add the hash to the mapping
				   currentMappings.emplace_back( std::vector<size_t>());
				   hashToMemory.emplace( hash_id, currentMappings.size() - 1 );
				   memoryToHash.emplace( currentMappings.size() - 1, hash_id );
				   currentMappings.back().push_back( tag_id );
			   }
			   else
			   {
				   for ( auto& map : currentMappings.at( ret->second ))
				   {
					   if ( map == tag_id )
					   {
						   return;
					   }
				   }
				
				   currentMappings.at( ret->second ).push_back( tag_id );
			   }
			
			
		   };
		
	}
	
	spdlog::info( "Loaded PTR Extra mappings", currentMappings.size());
}

std::vector<size_t> Mappings::getTags( size_t hash )
{
	auto ret = hashToMemory.find( hash );
	if ( ret == hashToMemory.end())
	{
		spdlog::error(
				"Failed to get tags for hash: " + std::to_string( hash ));
		return std::vector<size_t>();
	}
	
	return currentMappings.at( ret->second );
}

std::vector<size_t> Mappings::getHashesOnTag( size_t tag )
{
	std::vector<size_t> hashes;
	
	for ( size_t i = 0; i < currentMappings.size(); ++i )
	{
		auto& list = currentMappings.at( i );
		
		//Check the internal list
		auto ret = std::find( list.begin(), list.end(), tag );
		
		if ( ret == list.end())
		{
			continue;
		}
		
		auto ret2 = memoryToHash.find( i );
		if ( ret2 == memoryToHash.end())
		{
			continue;
		}
		else
		{
			hashes.push_back( ret2->second );
		}
		
	}
	return hashes;
}

