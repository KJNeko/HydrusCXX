//
// Created by kj16609 on 4/6/22.
//

#include "HydrusMappingDB.hpp"
#include "HydrusThreading.hpp"

namespace HydrusCXX {
	
	
	void Mappings::loadMappings()
	{
		spdlog::debug( "Loading mappings" );
		//Get a max count
		db
				<< "select hash_id from current_mappings_8 order by hash_id DESC limit 1"
				>> [&]( uint32_t hashCount )
				{
					currentMappings.reserve( hashCount + 1 );
				};
		
		//Resize the internal vector to the size of the tags per image
		db
				<< "select hash_id, count(*) from current_mappings_8 group by hash_id"
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
		
		spdlog::info( "Loaded {} mappings", currentMappings.size());
	}
	
	void Mappings::loadPTR( bool filtered )
	{
		spdlog::debug( "Loading PTR mappings" );
		//Get a max count
		db
				<< "select hash_id from current_mappings_14 where hash_id in (select hash_id from current_mappings_8) order by hash_id DESC limit 1"
				>> [&]( uint32_t hashCount )
				{
					ptrMappings.reserve( hashCount + 1 );
				};
		
		//Resize the internal vector to the size of the tags per image
		db
				<< "select hash_id, count(*) from current_mappings_14 where hash_id in (select hash_id from current_mappings_8) group by hash_id"
				>> [&]( uint32_t hash_id, uint32_t count )
				{
					ptrMappings.emplace_back( std::vector<size_t>());
					ptrMappings.back().reserve( count );
					ptrHashToMemory.emplace(
							hash_id, ptrMappings.size() - 1 );
					ptrMemoryToHash.emplace(
							ptrMappings.size() - 1, hash_id );
				};
		
		db
				<< "select hash_id, tag_id from current_mappings_14 where hash_id in (select hash_id from current_mappings_8)"
				>> [&]( uint32_t hash_id, uint32_t tag_id )
				{
					auto ret = ptrHashToMemory.find( hash_id );
					if ( ret != ptrHashToMemory.end())
					{
						ptrMappings.at( ret->second ).push_back(
								tag_id );
					}
				};
		
		ptrMappings.shrink_to_fit();
		
		spdlog::info( "Loaded {} PTR mappings", ptrMappings.size());
	}
	
	std::vector<size_t> Mappings::getTags( size_t hash, bool getPTR )
	{
		std::lock_guard<std::mutex> guard( mappingLock );
		auto ret = hashToMemory.find( hash );
		auto ret2 = ptrHashToMemory.find( hash );
		
		if ( ret == hashToMemory.end() && ret2 == ptrHashToMemory.end())
		{
			spdlog::error(
					"Failed to get tags for hash or there was no tags: " +
					std::to_string( hash ));
			return {};
		}
		
		std::vector<size_t> list;
		
		if ( ret != hashToMemory.end())
		{
			list = currentMappings.at( ret->second );
		}
		
		if ( ret2 != ptrHashToMemory.end())
		{
			for ( const auto& item : ptrMappings.at( ret2->second ))
			{
				list.push_back( item );
			}
		}
		
		return list;
	}
	
	std::vector<size_t> Mappings::getHashesOnTag( size_t tag, bool getPTR )
	{
		std::lock_guard<std::mutex> guard( mappingLock );
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
			
			//Check to find the tag from ret
			if ( ret != list.end())
			{
				auto hash_id = memoryToHash.find( i );
				if ( hash_id != memoryToHash.end())
				{
					hashes.push_back( hash_id->second );
				}
			}
		}
		
		for ( size_t i = 0; i < ptrMappings.size(); ++i )
		{
			auto& list = ptrMappings.at( i );
			auto ret = std::find( list.begin(), list.end(), tag );
			if ( ret == list.end())
			{
				continue;
			}
			
			//Check to find the tag from ret
			if ( ret != list.end())
			{
				auto hash_id = ptrMemoryToHash.find( i );
				if ( hash_id != ptrMemoryToHash.end())
				{
					hashes.push_back( hash_id->second );
				}
			}
		}
		
		return hashes;
	}
	
	void Mappings::sync()
	{
		using namespace HydrusCXX::Threading;
		auto& threadingManager = ThreadManager::getInstance();
		
		threadingManager.submit<void>(
				[&]()
				{ this->loadMappings(); } ).wait();
		
		threadingManager.submit<void>(
				[&]()
				{ this->loadPTR(); } ).wait();
	}
	
}