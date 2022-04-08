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

void doTask(
		std::mutex& terminalLock, std::mutex& listLock,
		std::vector<std::pair<std::string, std::function<void()>>>& tasks )
{
	while ( true )
	{
		listLock.lock();
		if ( tasks.empty())
		{
			listLock.unlock();
			terminalLock.lock();
			spdlog::info( "Thread finished all tasks" );
			terminalLock.unlock();
			return;
		}
		
		std::pair<std::string, std::function<void()>> task = tasks.back();
		tasks.pop_back();
		listLock.unlock();
		
		
		spdlog::info( "Thread started task: {}", task.first );
		
		task.second();
		
		spdlog::info( "Thread finished task: {}", task.first );
		std::this_thread::yield();
	}
	
}

void initalizeMappings( Mappings& mappings, bool enablePTR, bool fullPTR )
{
	std::cout << "Doing work" << std::endl;
	mappings.loadMappings( enablePTR, fullPTR );
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
		std::shared_ptr<WorkHook> hook = threadManager.template addWork(
				initalizeMappings, mappings, true, false );
		
		spdlog::info( "Testing hook aquire" );
		
		size_t counter { 0 };
		while ( !hook.get()->checkComplete())
		{
			++counter;
			std::this_thread::yield();
		}
		
		spdlog::info( "Took {} itterations to complete the work", counter );
		
		//Load the DBs
		
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
		
		spdlog::info( "HydrusCXX finished starting up." );
	}
	
	
};


#endif
