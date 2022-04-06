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


	void loadMappings()
	{
		spdlog::info( "Loading mappings into memory" );
		//Get a max count
		db << "select hash_id from current_mappings_8 order by hash_id DESC limit 1" >> [&]( uint32_t hashCount )
		{
			currentMappings.resize( hashCount + 1 );
		};

		//Resize the internal vector to the size of the tags per image
		db << "select hash_id, count(*) from current_mappings_8 group by hash_id"
		   >> [&]( uint32_t hash_id, uint32_t count )
		   {
			   currentMappings.at( hash_id ).reserve( count );
		   };

		db << "select hash_id, tag_id from current_mappings_8" >> [&]( uint32_t hash_id, uint32_t tag_id )
		{
			currentMappings.at( hash_id ).push_back( tag_id );
		};

		//Analyze the memory and print it to the debug log
		size_t count { 0 };
		for ( const auto& map : currentMappings )
		{
			count += map.size() * sizeof( size_t );
		}

		spdlog::info( "Memory size after loading local Mappings: " + formatBytesize( count ));
	}

	void loadPTR( bool filtered = true )
	{
		spdlog::info( "Loading PTR mappings into memory" );
		//Load tags of images that are matched

		auto bar = [&]( std::string name, size_t count, size_t max )
		{
			size_t textSpacing = ( name.size() + 2 );

			std::string endingText = " ( " + std::to_string( count ) + " / " + std::to_string( max ) + " )";


			int barSize = getColumnCount() - ( textSpacing + endingText.size() + 33 );

			size_t current = static_cast<int>(( static_cast<double>(barSize) / static_cast<double>(max)) *
											  static_cast<double>(count));

			std::stringstream ss;

			if ( barSize < 1 )
			{
				ss << name;
				ss << endingText;
			}
			else
			{
				ss << std::setfill( '#' );
				ss << name << " |" << std::setw( current ) << "";
				ss << std::setfill( ' ' );
				ss << std::setw( barSize - current ) << "|" << endingText;
			}


			spdlog::info( ss.str());

			if ( ss.str().size() + 33 > getColumnCount())
			{
				//Figure out how many times we need to go up
				size_t lines = ceil( static_cast<double>(ss.str().size()) / static_cast<double>(getColumnCount()));

				for ( size_t i = 0; i < lines; i++ )
				{
					std::cout << "\033[A";
				}
				std::cout << std::flush;
			}

			std::cout << "\033[A";
		};


		if ( filtered )
		{

			db
					<< "select hash_id, tag_id from current_mappings_14 where hash_id in (select hash_id from current_mappings_8)"
					>> [&]( uint32_t hash_id, uint32_t tag_id )
					{
						//Check to see if we already have the tag
						for ( auto& map : currentMappings.at( hash_id ))
						{
							if ( map == tag_id )
							{
								return;
							}
						}

						if ( hash_id % 1000 == 0 || hash_id == currentMappings.size() - 1 )
						{
							bar( "Selective PTR memory mapping", hash_id, currentMappings.size() - 1 );
						}

						currentMappings.at( hash_id ).push_back( tag_id );
					};

		}
		else
		{
			spdlog::warn(
					"loadPTR was told to load ALL tags from PTR. This could take awhile and increase memory size" );

			//Resize currentMappings to new size
			db << "select hash_id from current_mappings_14 order by hash_id DESC limit 1" >> [&]( uint32_t count )
			{
				currentMappings.resize( count + 1 );
			};

			db << "select hash_id, tag_id from current_mappings_14" >> [&]( uint32_t hash_id, uint32_t tag_id )
			{

				if ( hash_id % 1000 == 0 || hash_id == currentMappings.size() - 1 )
				{
					bar( "Full PTR memory mapping", hash_id, currentMappings.size() - 1 );
				}
				currentMappings.at( hash_id ).push_back( tag_id );
			};
		}

		std::cout << std::endl;

		//Analyze the memory and print it to the debug log
		size_t count { 0 };
		for ( const auto& map : currentMappings )
		{
			count += map.size() * sizeof( size_t );
		}

		spdlog::info( "Memory size after loading PTR: " + formatBytesize( count ));
	}


	Mappings( std::filesystem::path path, bool preload = false ) : db( path )
	{
		spdlog::info( path.string() + " was opened as Mappings" );
		if ( preload )
		{
			loadMappings();
			loadPTR();
		}
	}

	std::vector<size_t> getTags( size_t hash )
	{
		return currentMappings.at( hash );
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

	std::vector<std::pair<size_t, size_t>> tags {};
	std::vector<std::string> subtags {};
	std::vector<std::string> namespaceTags {};
	std::vector<std::vector<std::string>> urls {};

	void loadTags()
	{
		spdlog::info( "Loading tags and other supporting info into memory" );
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


		spdlog::info( "Tag memory usage: " + formatBytesize( tags.size() * ( sizeof( size_t ) * 2 )));

		size_t count { 0 };
		for ( const auto& item : subtags )
		{
			count += item.size();
		}
		spdlog::info( "Subtags memory usage: " + formatBytesize( count ));

		count = 0;
		for ( const auto& item : namespaceTags )
		{
			count += item.size();
		}
		spdlog::info( "Namespace memory usage: " + formatBytesize( count ));

		count = 0;
		for ( const auto& item : urls )
		{
			for ( const auto& url : item )
			{
				count += url.size();
			}
		}
		spdlog::info( "Urls memory usage: " + formatBytesize( count ));
	}

	Master( std::filesystem::path path, bool preload = false ) : db( path )
	{
		spdlog::info( path.string() + " was opened in Master" );

		if ( preload )
		{
			loadTags();
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
		}
	}

	void loadSiblings()
	{
		spdlog::info( "Loading siblings from tag_siblings into memory" );
		db << "select count(*) from tag_siblings" >> [&]( size_t count )
		{
			siblings.reserve( count );
		};

		db << "select * from tag_siblings" >> [&](
				[[maybe_unused]]size_t service_id, size_t bad_tag_id, size_t good_tag_id, [[maybe_unused]]bool status )
		{
			siblings.push_back( std::pair( bad_tag_id, good_tag_id ));
		};

		spdlog::info(
				"Memory size after loading Siblings: " + formatBytesize( siblings.size() * ( sizeof( size_t ) * 2 )));
	}

	void loadParents()
	{
		spdlog::info( "Loading parents from tag_parents into memory" );
		db << "select count(*) from tag_parents" >> [&]( size_t count )
		{
			parents.reserve( count );
		};

		db << "select * from tag_parents"
		   >> [&]( [[maybe_unused]]size_t service_id, size_t child_id, size_t parent_id, [[maybe_unused]]bool status )
		   {
			   parents.push_back( std::pair( child_id, parent_id ));
		   };

		spdlog::info(
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

