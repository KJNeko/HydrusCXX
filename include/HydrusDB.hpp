//
// Created by kj16609 on 4/15/22.
//

#ifndef HYDRUSCXX_HYDRUSDB_HPP
#define HYDRUSCXX_HYDRUSDB_HPP

#include <string>
#include <sqlite_modern_cpp.h>
#include <filesystem>
#include <type_traits>

class DB
{
	sqlite::database db;

public:
	DB( std::filesystem::path path )
			:
			db( path.string())
	{}
	
	
	template<typename returnType, bool multiReturn, class... TArgs>
	std::optional<returnType> query( std::string query )
	{
		bool success = false;
		returnType result;
		
		db << query >> [&]( TArgs... args )
		{
			success = true;
			if constexpr( multiReturn )
			{
				result.push_back( args... );
			}
			else
			{
				result = returnType( args... );
			}
		};
		
		if ( success )
		{
			return result;
		}
		else
		{
			return std::nullopt;
		}
	}
};

#endif //HYDRUSCXX_HYDRUSDB_HPP
