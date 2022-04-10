//
// Created by kj16609 on 4/9/22.
//

#ifndef HYDRUSCXX_HYDRUSPTR_HPP
#define HYDRUSCXX_HYDRUSPTR_HPP

#include "HydrusUtils.hpp"
#include "HydrusMainDB.hpp"
#include "HydrusMappingDB.hpp"
#include "HydrusMasterDB.hpp"

namespace HydrusCXX {
	
	class HydrusPTR
	{
		Main& mainRef;
		Mappings& mappingsRef;
		Master& masterRef;
		
		std::vector<size_t> hashes;
		
		HydrusPTR( Main& mainDB, Mappings& mappingDB, Master& masterDB )
				:
				mainRef( mainDB ), mappingsRef( mappingDB ),
				masterRef( masterDB )
		{
			//Get a list of all the PTR objects to process
			
			mainRef.db << "select * from repository_updates_14"
					   >> [&]( size_t hash_id, size_t update_index )
					   {
						   hashes.push_back( hash_id );
					   };
			
		}
		
		void processPTR()
		{
		
			
		}
		
		
	};
}

#endif //HYDRUSCXX_HYDRUSPTR_HPP
