#include <sqlite_modern_cpp.h>
#include <vector>
#include <filesystem>
#include <algorithm>

class Mappings
{
public:
	sqlite::database db;

	std::vector<std::vector<size_t>> currentMappings;


	Mappings( std::filesystem::path path ) : db( path )
	{
	}

	void loadMappings()
	{
		//Get a max count
		db << "select hash_id from current_mappings_8 order by hash_id DESC limit 1" >> [&]( size_t hashCount )
		{
			currentMappings.resize( hashCount + 1 );
		};

		//Resize the internal vector to the size of the tags per image
		db << "select hash_id, count(*) from current_mappings_8 group by hash_id" >> [&]( size_t hash_id, size_t count )
		{
			currentMappings.at( hash_id ).resize( count );
		};


	}

	std::vector<size_t> getHashesOnTag( size_t tag )
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

			hashes.push_back( i );
		}

		return hashes;
	}

};


class Master
{
public:
	sqlite::database db;

	std::vector<std::pair<size_t, size_t>> tags;
	std::vector<std::string> subtags;
	std::vector<std::string> namespaceTags;


	Master( std::filesystem::path path ) : db( path )
	{
	}

	void loadTags()
	{
		//Get max count
		db << "select tag_id from tags order by tag_id DESC limit 1" >> [&]( size_t tagCount )
		{
			tags.resize( tagCount + 1 );
		};

		db << "select * from tags" >> [&]( size_t tag_id, size_t namespace_id, size_t subtag_id )
		{
			tags.at( tag_id ) = std::pair( namespace_id, subtag_id );
		};

		db << "select namespace_id from namespaces order by namespace_id DESC limit 1" >> [&]( size_t count )
		{
			namespaceTags.resize( count + 1 );
		};

		db << "select * from namespaces" >> [&]( size_t namespace_id, std::string namespaceText )
		{
			namespaceTags.at( namespace_id ) = namespaceText;
		};

		db << "select subtag_id from subtags order by subtag_id DESC limit 1" >> [&]( size_t count )
		{
			subtags.resize( count + 1 );
		};

		db << "select * from subtags" >> [&]( size_t subtag_id, std::string text )
		{
			subtags.at( subtag_id ) = text;
		};


	}

	std::string getSubtag( size_t subtag_id )
	{
		return subtags.at( subtag_id );
	}

	std::string getNamespace( size_t namespace_id )
	{
		return namespaceTags.at( namespace_id );
	}

	std::pair<std::string, std::string> getTagString( size_t tag_id )
	{
		auto pairID = tags.at( tag_id );

		return std::pair( getNamespace( pairID.first ), getSubtag( pairID.second ));
	}

	size_t getSubtagID( std::string str )
	{

		auto ret = std::find( subtags.begin(), subtags.end(), str );
		if ( ret == subtags.end())
		{
			return 0;
		}

		return static_cast<size_t>(ret - subtags.begin());
	}

	size_t getNamespaceID( std::string str )
	{
		auto ret = std::find( namespaceTags.begin(), namespaceTags.end(), str );
		if ( ret == namespaceTags.end())
		{
			return 0;
		}

		return static_cast<size_t>(ret - namespaceTags.begin());
	}

	size_t getTagID( std::string group, std::string subtag )
	{
		size_t namespaceID = getNamespaceID( group );
		size_t subtagID = getSubtagID( subtag );

		auto ret = std::find( tags.begin(), tags.end(), std::pair( namespaceID, subtagID ));
		if ( ret == tags.end())
		{
			return 0;
		}

		return static_cast<size_t>(ret - tags.begin());
	}

};


