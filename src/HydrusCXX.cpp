//
// Created by kj16609 on 3/29/22.
//

#define fixme throw std::runtime_error( std::string( __func__ ) + " in " + std::string( __FILE__ ) + " is incomplete." );

#include "HydrusCXX.hpp"
#include <cstring>
#include <bit>
#include <iostream>

HydrusCXX::HydrusCXX( const std::filesystem::path& path ) : client( path.string() + "client.db" ),
															master( path.string() + "client.master.db" ),
															mappings( path.string() + "client.mappings.db" )
{
}

/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
														 CLIENT
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
														 MASTER
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


uint Master::getHashIdFromHash( std::bitset<256> )
{
	fixme
	return 0;
}

int getHashCallback( void* data, int argc, char** argv, char** azColName )
{
	memcpy( data, argv[0], 256 / 8 );
	return 0;
}

std::bitset<256> Master::getHash( uint id )
{
	std::bitset<256> hash;

	std::string query = "select * from master.hashes where hash_id == " + std::to_string( id );

	sqlite3_exec( db, query.c_str(), getHashCallback, &hash, nullptr );

	return hash;
}

int getIDCallback( void* data, int argc, char** argv, char** colName )
{
	if ( argc == 0 )
	{
		return 0;
	}

	unsigned int num = std::stoull( argv[0] );
	memcpy( data, &num, sizeof( num ));

	return 0;
}


std::optional<uint> Master::getNamespaceIDFromString( const std::string& group )
{


	std::string query = "select namespace_id from namespaces where namespace == \'" + group + "\'";


	uint temp { 0 };

	sqlite3_exec( db, query.c_str(), getIDCallback, &temp, nullptr );

	if ( temp == 0 )
	{
		return std::nullopt;
	}


	return temp;
}

int getTextCallback( void* data, int argc, char** argv, char** colName )
{
	if ( argc == 0 )
	{
		return 0;
	}

	auto dataptr = reinterpret_cast<std::string*>(data);
	*dataptr = argv[0];

	return 0;
}

std::optional<std::string> Master::getNamespace( uint namespace_id )
{
	//Attempt to find the tag
	auto iter = namespaceMap.find( namespace_id );
	if ( iter == namespaceMap.end())
	{
		bool set = true;
		//There was no tag. Attempt to fetch one from the DB
		std::string temp;

		std::string query = "select namespace from namespaces where namespace_id == " + std::to_string( namespace_id );

		sqlite3_exec( db, query.c_str(), getTextCallback, &temp, nullptr );

		namespaceMap.emplace( namespace_id, temp );
		return temp;
	}

	return iter->second;
}

std::optional<uint> Master::getSubtagIDFromString( std::string str )
{
	uint temp { 0 };

	std::string query = "select subtag_id from subtags where subtag ==  \'" + str + "\'";

	sqlite3_exec( db, query.c_str(), getIDCallback, &temp, nullptr );

	if ( temp == 0 )
	{
		return std::nullopt;
	}

	return temp;
}

std::optional<std::string> Master::getSubtag( uint subtag_id )
{
	//Attempt to find the tag
	auto iter = subtagMap.find( subtag_id );
	if ( iter == subtagMap.end())
	{
		//There was no tag. Attempt to fetch one from the DB
		std::string temp;

		std::string query = "select subtag from subtags where subtag_id == " + std::to_string( subtag_id );

		sqlite3_exec( db, query.c_str(), getTextCallback, &temp, nullptr );

		if ( temp.empty())
		{
			return std::nullopt;
		}

		subtagMap.emplace( subtag_id, temp );
		return temp;
	}

	return iter->second;
}

std::optional<uint> Master::getTagIDFromPair( uint namespace_id, uint subtag_id )
{
	uint temp { 0 };

	std::string query =
			"select tag_id from tags where subtag_id == " + std::to_string( subtag_id ) + " and namespace_id == " +
			std::to_string( namespace_id );

	sqlite3_exec( db, query.c_str(), getIDCallback, &temp, nullptr );

	if ( temp == 0 )
	{
		return std::nullopt;
	}
	return temp;
}

std::optional<uint> Master::getTagIDFromStringPair( const std::string& group, const std::string& subtag )
{
	auto subtag_id = getSubtagIDFromString( subtag );
	auto namespace_id = getNamespaceIDFromString( group );

	if ( !subtag_id.has_value() || !namespace_id.has_value())
	{
		return std::nullopt;
	}

	return getTagIDFromPair( namespace_id.value(), subtag_id.value());
}

int getTagPairCallback( void* data, int argc, char** argv, char** colName )
{
	auto dataptr = reinterpret_cast<std::pair<uint, uint>*>(data);

	if ( argc == 0 )
	{
		*dataptr = std::pair<uint, uint>( 0, 0 );
		return 0;
	}

	unsigned int group_id = std::stoul( argv[0] );
	unsigned int subtag_id = std::stoul( argv[1] );

	*dataptr = std::pair<uint, uint>( group_id, subtag_id );

	return 0;
}


std::optional<std::pair<uint, uint>> Master::getTagPair( uint tag_id )
{
	

	auto iter = tagMap.find( tag_id );
	if ( iter == tagMap.end())
	{

		std::pair<uint, uint> temp;

		std::string query = "select namespace_id, subtag_id from tags where tag_id == " + std::to_string( tag_id );

		sqlite3_exec( db, query.c_str(), getTagPairCallback, &temp, nullptr );

		if ( temp == std::pair<uint, uint>( 0, 0 ))
		{
			return std::nullopt;
		}

		return temp;
	}


	return std::pair( iter->second.first, iter->second.second );
}

std::optional<std::pair<std::string, std::string>> Master::getTagPairString( const uint tag_id )
{
	auto pair = getTagPair( tag_id );

	if ( !pair.has_value())
	{
		return std::nullopt;
	}

	auto namespace_str = getNamespace( pair->first );
	auto subtag_str = getSubtag( pair->second );

	if ( !namespace_str.has_value())
	{
		throw std::runtime_error(
				"namespace didn't have a value. Trying to find string for value: " + std::to_string( pair->first ));
	}

	if ( !subtag_str.has_value())
	{
		throw std::runtime_error(
				"subtag didn't have a value. Trying to find string for value: " + std::to_string( pair->second ));
	}


	return std::pair( getNamespace( pair->first ).value(), getSubtag( pair->second ).value());

}

/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
														MAPPINGS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

int getIDVectorCallback( void* data, int argc, char** argv, char** colName )
{
	auto dataptr = reinterpret_cast<std::vector<uint>*>(data);

	dataptr->push_back( std::stoull( argv[0] ));

	return 0;

}

std::vector<uint> Mappings::getImageList( const uint tag_id )
{

	std::vector<uint> temp;

	std::string query = "select hash_id from current_mappings_8 where tag_id == " + std::to_string( tag_id );

	sqlite3_exec( db, query.c_str(), getIDVectorCallback, &temp, nullptr );

	return temp;

}

std::vector<uint> Mappings::getTagList( const uint hash_id )
{


	std::vector<uint> temp;

	std::string query = "select tag_id from current_mappings_8 where hash_id == " + std::to_string( hash_id );

	sqlite3_exec( db, query.c_str(), getIDVectorCallback, &temp, nullptr );

	return temp;

}