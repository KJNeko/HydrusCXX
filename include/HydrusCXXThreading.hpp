//
// Created by kj16609 on 4/8/22.
//

#ifndef HYDRUSCXX_HYDRUSCXXTHREADING_HPP
#define HYDRUSCXX_HYDRUSCXXTHREADING_HPP

#include <semaphore>
#include <thread>
#include <string>
#include <memory>
#include <queue>

#include <future>

#include "include/Extras/ringBuffer.hpp"

#include "spdlog/spdlog.h"

class WorkHook;

class WorkBasic
{
public:
	virtual ~WorkBasic() = default;
	
	virtual void doTask() = 0;
};

template<typename Ret, typename Func, typename... Args>
class WorkUnit final : public WorkBasic
{
	std::packaged_task<Ret( Args... )> task;
	std::tuple<Args...> args;

public:
	WorkUnit( Func func, Args... arguments )
			:
			task( func ), args( arguments... )
	{}
	
	std::future<Ret> getFuture()
	{
		return task.get_future();
	}
	
	void doTask() override
	{
		task();
	}
};


class HydrusCXXThreadManager
{
	std::vector<std::thread> threads {};
	
	ringBuffer<WorkBasic*, 15> workQueue {};
	
	bool stopThreads { false };
	
	void doWork()
	{
		spdlog::debug( "[HydrusCXX]: A thread has started" );
		while ( !stopThreads )
		{
			//GetWork
			auto work = workQueue.getNext_for(
					std::chrono::milliseconds( 200 ));
			
			if ( !work.has_value())
			{
				//No work to do.
				std::this_thread::yield();
				continue;
			}
			
			work.value()->doTask();
			
			delete work.value();
		}
		spdlog::debug( "[HydrusCXX]: A thread has stopped" );
	}
	
	//ringBuffer for work to be done
public:
	HydrusCXXThreadManager( size_t size )
	{
		if ( size == 0 )
		{
			spdlog::critical(
					"Abnormal thread count for thread manager. Threads: {}",
					size );
		}
		if ( size > std::thread::hardware_concurrency())
		{
			spdlog::warn(
					"{} threads where allocated when the system reports only {} threads",
					size, std::thread::hardware_concurrency());
		}
		
		spdlog::info( "Starting {} threads", size );
		for ( size_t i = 0; i < size; ++i )
		{
			threads.push_back(
					std::thread(
							&HydrusCXXThreadManager::doWork, this ));
		}
	}
	
	template<typename Ret, typename Func, typename... Args>
	std::future<Ret> submit( Func func, Args& ... args )
	{
		auto* workUnit = new WorkUnit<Ret, Func, Args...>( func, args... );
		( workQueue.pushNext(
				static_cast<WorkBasic*>(workUnit), std::chrono::seconds( 2 )));
		
		return workUnit->getFuture();
	}
	
	~HydrusCXXThreadManager()
	{
		stopThreads = true;
		for ( auto& th : threads )
		{
			th.join();
		}
	}
	
};


#endif //HYDRUSCXX_HYDRUSCXXTHREADING_HPP
