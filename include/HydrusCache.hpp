//
// Created by kj16609 on 4/15/22.
//

#ifndef HYDRUSCXX_CACHE_HPP
#define HYDRUSCXX_CACHE_HPP

#include <string>
#include <unordered_map>
#include <typeinfo>
#include <vector>
#include <stdexcept>

#include <tuple>

#include "HydrusDB.hpp"


template<bool cacheEnable, typename Key, bool multiReturn, class... TArgs>
class Query
{
	static constexpr bool useTuple = sizeof...( TArgs ) > 1;
	
	typedef std::conditional_t<useTuple, std::tuple<TArgs...>, std::tuple_element_t<0, std::tuple<TArgs...>>> Tuple;
	typedef std::conditional_t<multiReturn, std::vector<Tuple>, Tuple> storageType;
	typedef std::conditional_t<cacheEnable, std::unordered_map<Key, std::optional<storageType>>, void*> cacheType;
	
	std::string query;
	
	bool useEmpty;
	
	cacheType cache {};
	
	DB* db { nullptr };
	
	
	std::string formatValue( Key value )
	{
		//String
		if constexpr( std::is_same<Key, std::string>::value )
		{
			return "'" + value + "'";
		}
		
		if constexpr( std::is_integral<Key>::value )
		{
			return std::to_string( value );
		}
		
		if constexpr ( std::is_array<Key>::value )
		{
			//TODO
			throw std::runtime_error( "Not implemented" );
		}
		
		throw std::runtime_error( "Unsupported type in cache" );
	}

public:
	
	Query( DB* databasePtr, std::string queryText, bool useEmptySpace = true )
			:
			query( queryText ), db( databasePtr ), useEmpty( useEmptySpace )
	{
	
	}
	
	Query operator=( const Query& ) = delete;
	
	Query( const Query& ) = delete;
	
	
	size_t cacheSize()
	{
		if constexpr( cacheEnable )
		{
			return cache.size();
		}
		else
		{
			return 0;
		}
	}
	
	std::optional<storageType> get( Key value, bool forceQuery = false )
	{
		if constexpr( cacheEnable )
		{
			auto ret = cache.find( value );
			
			//Remove the previous cache if forceQuery is true
			if ( ret != cache.end() && forceQuery )
			{
				cache.erase( ret );
			}
			
			if ( ret != cache.end() || forceQuery )
			{
				return ret->second;
			}
			else
			{
				
				std::string formattedQuery { "" };
				
				for ( const auto& c : query )
				{
					if ( c == '?' )
					{
						formattedQuery += formatValue( value );
					}
					else
					{
						formattedQuery += c;
					}
				}
				std::optional<storageType> retQuery = db->query<storageType, multiReturn, TArgs...>(
						formattedQuery );
				if ( retQuery.has_value())
				{
					cache.emplace( value, retQuery.value());
					return retQuery;
				}
				
				if ( useEmpty )
				{
					cache.emplace( value, std::nullopt );
					return std::nullopt;
				}
				
				return std::nullopt;
			}
		}
		else
		{
			std::string formattedQuery { "" };
			
			for ( const auto& c : query )
			{
				if ( c == '?' )
				{
					formattedQuery += formatValue( value );
				}
				else
				{
					formattedQuery += c;
				}
			}
			return db->query<storageType, multiReturn, TArgs...>(
					formattedQuery );
		}
		
	}
};

#endif //HYDRUSCXX_CACHE_HPP