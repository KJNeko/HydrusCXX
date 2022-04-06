#include <sqlite_modern_cpp.h>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <fstream>

#include <zstr.hpp>
#include <spdlog/spdlog.h>

#include <sys/ioctl.h>

size_t getColumnCount()
{
	struct winsize w;
	ioctl( 0, TIOCGWINSZ, &w );
	return w.ws_col;
}

std::string formatBytesize( size_t count )
{
	if ( count > 1000 )
	{
		//B
		if ( count > 1000000 )
		{
			//KB
			if ( count > 1000000000 )
			{
				return std::to_string( count / 1000000000 ) + "GB";

			}
			else
			{
				return std::to_string( count / 1000000 ) + "MB";
			}
		}
		else
		{
			return std::to_string( count / 1000 ) + "KB";
		}
	}
	else
	{
		return std::to_string( count ) + "B";
	}
}

class Mappings
{
public:
	sqlite::database db;

	std::vector<std::vector<size_t>> currentMappings {};

	std::unordered_map<size_t, size_t> hashToMemory {};
	std::unordered_map<size_t, size_t> memoryToHash {};

	void loadMappings()
	{
		spdlog::debug( "Loading mappings into memory" );
		//Get a max count
		db << "select hash_id from current_mappings_8 order by hash_id DESC limit 1" >> [&]( uint32_t hashCount )
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

		db << "select hash_id, tag_id from current_mappings_8" >> [&]( uint32_t hash_id, uint32_t tag_id )
		{
			auto ret = hashToMemory.find( hash_id );
			if ( ret != hashToMemory.end())
			{
				currentMappings.at( ret->second ).push_back( tag_id );
			}
		};

		//Analyze the memory and print it to the debug log
		size_t count { 0 };
		size_t wastedCount { 0 };

		count += sizeof( std::vector<std::vector<size_t>> );
		count += sizeof( std::vector<size_t> ) * currentMappings.size();
		for ( const auto& v : currentMappings )
		{
			count += sizeof( size_t ) * v.size();
		}

		count += sizeof( std::unordered_map<size_t, size_t> ) * 2;
		count += ( sizeof( std::pair<size_t, size_t> ) * hashToMemory.size() * 2 );

		spdlog::debug( "Memory size after loading local Mappings: " + formatBytesize( count ));
		spdlog::debug( "Wasted memory: " + formatBytesize( wastedCount * sizeof( size_t )));
	}

	void loadPTR( bool filtered = true )
	{
		spdlog::debug( "Loading PTR mappings into memory" );
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
						
						if ( hash_id % 250000 == 0 )
						{
							spdlog::info( "Loaded mapping number " + std::to_string( hash_id ));
						}
					};
		}
		else
		{
			spdlog::warn(
					"loadPTR was told to load ALL tags from PTR. This could take awhile and increase memory size" );

			db << "select hash_id, tag_id from current_mappings_14" >> [&]( uint32_t hash_id, uint32_t tag_id )
			{
				if ( hash_id % 250000 == 0 )
				{
					spdlog::info( "Loaded mapping number " + std::to_string( hash_id ));
				}

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

		spdlog::info( "Loaded Mappings" );

		//Analyze the memory and print it to the debug log
		size_t count { 0 };
		for ( const auto& map : currentMappings )
		{
			count += map.size() * sizeof( size_t );
		}

		spdlog::debug( "Memory size after loading PTR: " + formatBytesize( count ));
	}


	Mappings( std::filesystem::path path, bool preload = false ) : db( path )
	{
		spdlog::info( path.string() + " was opened as Mappings" );
		if ( preload )
		{
			loadMappings();
			loadPTR();
			spdlog::info( "Preload finished for Mappings" );
		}
	}

	std::vector<size_t> getTags( size_t hash )
	{
		auto ret = hashToMemory.find( hash );
		if ( ret == hashToMemory.end())
		{
			spdlog::error( "Failed to get tags for hash: " + std::to_string( hash ));
			return std::vector<size_t>();
		}

		return currentMappings.at( ret->second );
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

};


class Master
{
public:
	sqlite::database db;

	std::vector<std::pair<size_t, size_t>> tags {};
	std::vector<std::string> subtags {};
	std::vector<std::string> namespaceTags {};
	std::vector<std::vector<std::string>> urls {};

	void loadTags()
	{
		spdlog::debug( "Loading tags and other supporting info into memory" );
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

		//Get size
		db << "select * from urls order by url_id DESC limit 1" >> [&]( size_t url_id, [[maybe_unused]]std::string url )
		{
			urls.resize( url_id + 1 );
		};

		db << "select url_id, url from urls" >> [&]( size_t url_id, std::string url )
		{
			urls.at( url_id ).push_back( url );
		};


		spdlog::debug( "Tag memory usage: " + formatBytesize( tags.size() * ( sizeof( size_t ) * 2 )));

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

	Master( std::filesystem::path path, bool preload = false ) : db( path )
	{
		spdlog::info( path.string() + " was opened in Master" );

		if ( preload )
		{
			loadTags();
			spdlog::info( "Preload finished for Master" );
		}
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

	size_t getTagID( size_t group, size_t subtag )
	{
		auto ret = std::find( tags.begin(), tags.end(), std::pair( group, subtag ));
		if ( ret == tags.end())
		{
			return 0;
		}

		return static_cast<size_t>(ret - tags.begin());
	}


	size_t getTagID( std::string group, std::string subtag )
	{
		return getTagID( getNamespaceID( group ), getSubtagID( subtag ));
	}
};

class Main
{
public:
	sqlite::database db;

	std::vector<std::pair<size_t, size_t>> siblings {};
	std::vector<std::pair<size_t, size_t>> parents {};

	Main( std::filesystem::path path, bool preload = false ) : db( path )
	{
		spdlog::info( path.string() + " was loaded as Main" );
		if ( preload )
		{
			loadSiblings();
			loadParents();
			spdlog::info( "Preload finished for Main" );
		}
	}

	void loadSiblings()
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

	void loadParents()
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

	std::vector<size_t> getParents( size_t id )
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

	size_t getSibling( size_t id )
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

	size_t recursiveSibling( size_t id )
	{
		auto ret = getSibling( id );
		if ( ret == id )
		{
			return id;
		}

		return recursiveSibling( ret );
	}
};

struct DBData
{
	std::vector<std::string> subtags;
	std::vector<std::string> namespaceTags;
	std::vector<std::pair<size_t, size_t>> tagPair;

	std::vector<std::vector<size_t>> mappings;
};
/*
class JsonParser
{
	DBData add;
	DBData remove;

public:
	void parse( std::filesystem::path path )
	{
		//Read the file raw
		std::string data;

		if ( std::ifstream ifs( path ); ifs )
		{
			//Read the file
			std::stringstream ss;
			ss << ifs.rdbuf();

			//Decompress with zstr istream
			zstr::istream zs( ss );

			std::stringstream ss2;
			ss2 << zs.rdbuf();

			data = ss2.str();

			//print data
			std::cout << data << std::endl;
		}


	}

};
*/

