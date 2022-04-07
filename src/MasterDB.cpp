//
// Created by kj16609 on 4/6/22.
//

#include "MasterDB.hpp"

void Master::loadTags()
{
	spdlog::debug( "Loading tags and other supporting info into memory" );
	//Get max count
	db << "select tag_id from tags order by tag_id DESC limit 1"
	   >> [&]( size_t tagCount )
	   {
		   tags.resize( tagCount + 1 );
	   };
	
	db << "select * from tags"
	   >> [&]( size_t tag_id, size_t namespace_id, size_t subtag_id )
	   {
		   tags.at( tag_id ) = std::pair( namespace_id, subtag_id );
	   };
	
	db
			<< "select namespace_id from namespaces order by namespace_id DESC limit 1"
			>> [&]( size_t count )
			{
				namespaceTags.resize( count + 1 );
			};
	
	db << "select * from namespaces"
	   >> [&]( size_t namespace_id, std::string namespaceText )
	   {
		   namespaceTags.at( namespace_id ) = namespaceText;
	   };
	
	db << "select subtag_id from subtags order by subtag_id DESC limit 1"
	   >> [&]( size_t count )
	   {
		   subtags.resize( count + 1 );
	   };
	
	db << "select * from subtags" >> [&]( size_t subtag_id, std::string text )
	{
		subtags.at( subtag_id ) = text;
	};
	
	//Get size
	db << "select * from urls order by url_id DESC limit 1"
	   >> [&]( size_t url_id, [[maybe_unused]]std::string url )
	   {
		   urls.resize( url_id + 1 );
	   };
	
	db << "select url_id, url from urls"
	   >> [&]( size_t url_id, std::string url )
	   {
		   urls.at( url_id ).push_back( url );
	   };
	
	
	spdlog::debug(
			"Tag memory usage: " +
			formatBytesize( tags.size() * ( sizeof( size_t ) * 2 )));
	
	size_t count { 0 };
	for ( const auto& item : subtags )
	{
		count += item.size();
	}
	spdlog::debug( "Subtags memory usage: " + formatBytesize( count ));
	
	count = 0;
	for ( const auto& item : namespaceTags )
	{
		count += item.size();
	}
	spdlog::debug( "Namespace memory usage: " + formatBytesize( count ));
	
	count = 0;
	for ( const auto& item : urls )
	{
		for ( const auto& url : item )
		{
			count += url.size();
		}
	}
	spdlog::debug( "Urls memory usage: " + formatBytesize( count ));
}

std::string Master::getSubtag( size_t subtag_id )
{
	return subtags.at( subtag_id );
}

std::string Master::getNamespace( size_t namespace_id )
{
	return namespaceTags.at( namespace_id );
}

std::pair<std::string, std::string> Master::getTagString( size_t tag_id )
{
	auto pairID = tags.at( tag_id );
	
	return std::pair( getNamespace( pairID.first ), getSubtag( pairID.second ));
}

size_t Master::getSubtagID( std::string str )
{
	
	auto ret = std::find( subtags.begin(), subtags.end(), str );
	if ( ret == subtags.end())
	{
		return 0;
	}
	
	return static_cast<size_t>(ret - subtags.begin());
}

size_t Master::getNamespaceID( std::string str )
{
	//Check if it's empty. If so return 1
	if ( str.empty())
	{
		return 1;
	}
	
	auto ret = std::find( namespaceTags.begin(), namespaceTags.end(), str );
	if ( ret == namespaceTags.end())
	{
		return 0;
	}
	
	return static_cast<size_t>(ret - namespaceTags.begin());
}

size_t Master::getTagID( size_t group, size_t subtag )
{
	auto ret = std::find( tags.begin(), tags.end(), std::pair( group, subtag ));
	if ( ret == tags.end())
	{
		return 0;
	}
	
	return static_cast<size_t>(ret - tags.begin());
}


size_t Master::getTagID( std::string group, std::string subtag )
{
	return getTagID( getNamespaceID( group ), getSubtagID( subtag ));
}

std::vector<std::string> Master::getTagStrings( std::vector<size_t> tag_idList )
{
	std::vector<std::string> list;
	
	for ( const auto& tag_id : tag_idList )
	{
		list.push_back(
				getTagString( tag_id ).first + ":" +
				getTagString( tag_id ).second );
	}
	
	return list;
}
