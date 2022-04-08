#ifndef HYDRUSCXX_HPP
#define HYDRUSCXX_HPP


#include "MappingDB.hpp"
#include "MainDB.hpp"
#include "MasterDB.hpp"
#include "HydrusCXXThreading.hpp"

#include <string>
#include <vector>
#include <mutex>
#include <thread>

void initalizeMappings( Mappings& mappings, bool enablePTR, bool fullPTR )
{
	spdlog::info(
			"Loading mappings table with flags: enablePTR:{}, fullPTR:{}",
			enablePTR, fullPTR );
	mappings.loadMappings( enablePTR, fullPTR );
	spdlog::info( "Loaded {} mappings", mappings.currentMappings.size());
}

void initalizeMaster( Master& master )
{
	spdlog::info( "Loading Master table" );
	spdlog::info( "Loading Tags" );
	master.loadTags();
	spdlog::info( "Loaded {} tags", master.tags.size());
	spdlog::info( "Loading subtags" );
	master.loadSubtags();
	spdlog::info( "Loaded {} subtags", master.subtags.size());
	spdlog::info( "Loading namespaces" );
	master.loadNamespaces();
	spdlog::info( "Loaded {} namespaces", master.namespaceTags.size());
	spdlog::info( "Loading URLs" );
	master.loadURLs();
	spdlog::info( "Loaded {} URLs", master.urls.size());
}

void initalizeMain( Main& main )
{
	main.loadParents();
	main.loadSiblings();
}

class HydrusCXX
{
public:
	
	Mappings mappings;
	Master master;
	Main main;
	
	HydrusCXXThreadManager threadManager;
	
	HydrusCXX(
			std::filesystem::path dbDir, size_t threadCount = 4,
			bool enablePTR = false, bool enableFULLPTR = false )
			:
			mappings( dbDir.string() + "/client.mappings.db" ),
			master( dbDir.string() + "/client.master.db" ),
			main( dbDir.string() + "/client.db" ), threadManager( threadCount )
	{
		
		
		//@formatter:off
		std::vector<std::shared_ptr<WorkHook>> hooks
		{
			threadManager.addWork( initalizeMappings, mappings, enablePTR, enableFULLPTR ),
			threadManager.addWork( initalizeMaster, master ),
			threadManager.addWork( initalizeMain, main )
		};
		
		//@formatter:on
		for ( auto& hook : hooks )
		{
			hook.get()->waitOn();
		}
		
		spdlog::info( "Mapping count: {}", mappings.currentMappings.size());
		
		spdlog::info( "Finished loading all information" );
		/*
		std::mutex terminalLock;
		std::mutex listLock;
		
		
		if ( threadCount < 1 )
		{
			threadCount = 1;
			spdlog::warn(
					"Only {} thread(s) detected.",
					std::thread::hardware_concurrency());
		}
		
		spdlog::info(
				"Using {} out of a maximum of {} threads", threadCount,
				std::thread::hardware_concurrency());
		
		// @formatter:off
		std::vector<std::pair<std::string, std::function<void()>>> tasks
		{
			{ "Mappings:loadMappings()",    [&](){ mappings.loadMappings(enablePTR, enableFULLPTR); }},
			{ "Master:loadTags()", [&](){master.loadTags();}},
			{ "Master:loadSubtags()", [&](){master.loadSubtags();}},
			{ "Master:loadNamespaces()", [&](){master.loadNamespaces();}},
			{ "Master:loadURLs()", [&](){master.loadURLs();}},
			{ "Main:loadSiblings()", [&](){main.loadSiblings();}},
			{ "Main:loadParents()", [&](){main.loadParents();}}
		};
		// @formatter:on
		
		std::vector<std::thread> threads;
		
		for ( size_t i = 0; i < threadCount; ++i )
		{
			threads.push_back(
					std::thread(
							doTask, std::ref( terminalLock ),
							std::ref( listLock ), std::ref( tasks )));
		}
		
		for ( auto& th : threads )
		{
			th.join();
		}
		
		spdlog::info( "All threads complete" );
		*/
		spdlog::info( "HydrusCXX finished starting up." );
	}
	
	
};


#endif
