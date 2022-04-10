#ifndef HYDRUSCXX_HPP
#define HYDRUSCXX_HPP


#include "HydrusMappingDB.hpp"
#include "HydrusMainDB.hpp"
#include "HydrusMasterDB.hpp"
#include "HydrusThreading.hpp"

#include <string>
#include <vector>
#include <mutex>
#include <thread>


class HydrusDB
{
public:
	
	HydrusCXX::Mappings mappings;
	HydrusCXX::Master master;
	HydrusCXX::Main main;
	
	explicit HydrusDB(
			const std::filesystem::path& dbDir )
			:
			mappings( dbDir.string() + "/client.mappings.db" ),
			master( dbDir.string() + "/client.master.db" ),
			main( dbDir.string() + "/client.db" )
	{
		
		auto& threadManager = HydrusCXX::Threading::ThreadManager::getInstance();
		
		
		std::vector<std::future<void>> waitList {
		
		};
		
		waitList.push_back(
				threadManager.submit<void>(
						[&]()
						{ mappings.loadMappings(); } ));
		waitList.push_back(
				threadManager.submit<void>(
						[&]()
						{ mappings.loadPTR(); } ));
		waitList.push_back(
				threadManager.submit<void>(
						[&]()
						{
							main.loadParents();
							main.loadSiblings();
						} ));
		waitList.push_back(
				threadManager.submit<void>(
						[&]()
						{ master.loadTags(); } ));
		waitList.push_back(
				threadManager.submit<void>(
						[&]()
						{ master.loadSubtags(); } ));
		waitList.push_back(
				threadManager.submit<void>(
						[&]()
						{
							master.loadNamespaces();
							master.loadURLs();
						} ));
		
		
		for ( auto& future : waitList )
		{
			future.wait();
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
