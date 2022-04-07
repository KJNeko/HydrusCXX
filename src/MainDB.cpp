//
// Created by kj16609 on 4/6/22.
//

#include "MainDB.hpp"

void Main::loadSiblings()
{
	spdlog::debug( "Loading siblings from tag_siblings into memory" );
	db << "select count(*) from tag_siblings" >> [&]( size_t count )
	{
		siblings.reserve( count );
	};

	db << "select * from tag_siblings" >> [&](
			[[maybe_unused]]size_t service_id, size_t bad_tag_id, size_t good_tag_id, [[maybe_unused]]bool status )
	{
		siblings.push_back( std::pair( bad_tag_id, good_tag_id ));
	};

	spdlog::debug(
			"Memory size after loading Siblings: " + formatBytesize( siblings.size() * ( sizeof( size_t ) * 2 )));
}

void Main::loadParents()
{
	spdlog::debug( "Loading parents from tag_parents into memory" );
	db << "select count(*) from tag_parents" >> [&]( size_t count )
	{
		parents.reserve( count );
	};

	db << "select * from tag_parents"
	   >> [&]( [[maybe_unused]]size_t service_id, size_t child_id, size_t parent_id, [[maybe_unused]]bool status )
	   {
		   parents.push_back( std::pair( child_id, parent_id ));
	   };

	spdlog::debug(
			"Memory size after loading Parents: " + formatBytesize( siblings.size() * ( sizeof( size_t ) * 2 )));
}

std::vector<size_t> Main::getParents( size_t id )
{
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