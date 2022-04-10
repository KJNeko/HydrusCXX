//
// Created by kj16609 on 4/6/22.
//

#include "HydrusMainDB.hpp"
#include "HydrusThreading.hpp"

namespace HydrusCXX {
	
	void Main::loadSiblings()
	{
		std::lock_guard<std::mutex> guard( siblingLock );
		spdlog::debug( "Loading siblings" );
		db << "select count(*) from tag_siblings" >> [&]( size_t count )
		{
			siblings.reserve( count );
		};
		
		db << "select * from tag_siblings" >> [&](
				[[maybe_unused]]size_t service_id, size_t bad_tag_id,
				size_t good_tag_id, [[maybe_unused]]bool status )
		{
			siblings.emplace_back( bad_tag_id, good_tag_id );
		};
		spdlog::info( "Loaded {} siblings", siblings.size());
	}
	
	void Main::loadParents()
	{
		std::lock_guard<std::mutex> guard( parentLock );
		spdlog::debug( "Loading parents" );
		db << "select count(*) from tag_parents" >> [&]( size_t count )
		{
			parents.reserve( count );
		};
		
		db << "select * from tag_parents" >> [&](
				[[maybe_unused]]size_t service_id, size_t child_id,
				size_t parent_id, [[maybe_unused]]bool status )
		{
			parents.emplace_back( child_id, parent_id );
		};
		spdlog::info( "Loaded {} parents", parents.size());
	}
	
	std::vector<size_t> Main::getParents( size_t id )
	{
		std::lock_guard<std::mutex> guard( parentLock );
		std::vector<size_t> ret;
		
		for ( auto& parent : parents )
		{
			if ( parent.first == id )
			{
				ret.push_back( parent.second );
			}
		}
		
		return ret;
	}
	
	size_t Main::getSibling( size_t id )
	{
		std::lock_guard<std::mutex> guard( siblingLock );
		for ( auto& sibling : siblings )
		{
			if ( sibling.first == id )
			{
				return sibling.second;
			}
		}
		
		
		//Return itself if no siblings
		return id;
	}
	
	size_t Main::recursiveSibling( size_t id )
	{
		auto ret = getSibling( id );
		if ( ret == id )
		{
			return id;
		}
		
		return recursiveSibling( ret );
	}
	
	void Main::sync()
	{
		spdlog::warn( "Main::sync() has been called. Wiping and recaching" );
		
		//Wipe
		parents.clear();
		siblings.clear();
		
		using namespace HydrusCXX::Threading;
		ThreadManager& ref = ThreadManager::getInstance();
		
		auto wait1 = ref.submit<void>(
				[&]()
				{ this->loadParents(); } );
		auto wait2 = ref.submit<void>(
				[&]()
				{ this->loadSiblings(); } );
		
		wait1.wait();
		wait2.wait();
	}
}