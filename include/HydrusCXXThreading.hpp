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

#include "include/Extras/ringBuffer.hpp"

#include "spdlog/spdlog.h"

class WorkHook;

class WorkBasic
{
public:
	virtual ~WorkBasic() = default;
	
	
	virtual void doWork() = 0;
	
	virtual std::shared_ptr<WorkHook> acquireHook() = 0;
};


class WorkHook
{
	std::binary_semaphore lockFlag { 0 };

public:
	bool checkComplete()
	{
		auto retBool = lockFlag.try_acquire();
		if ( retBool )
		{
			//We managed to get the semaphore.
			lockFlag.release();
			return true;
		}
		else
		{
			return false;
		}
	}
	
	void waitOn()
	{
		lockFlag.acquire();
	}
	
	void setComplete()
	{
		lockFlag.release();
	}
};


template<typename TFunc, typename... Ts>
class WorkUnit final : public WorkBasic
{
	TFunc func;
	std::tuple<Ts...> args;
	std::shared_ptr<WorkHook> hook;

public:
	WorkUnit( TFunc function, Ts& ... argsPack )
			:
			func( function ), args( argsPack... ), hook( new WorkHook )
	{
	}
	
	std::shared_ptr<WorkHook> acquireHook() override
	{
		return hook;
	}
	
	void doWork() override
	{
		std::apply(
				[&]( auto&& ... argPack )
				{ func( argPack... ); }, args );
		//Work complete
		hook->setComplete();
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
			
			work.value()->doWork();
			
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
	
	template<typename TFunc, typename... Ts>
	std::shared_ptr<WorkHook> submit( TFunc func, Ts& ... args )
	{
		
		auto* workUnit = new WorkUnit<TFunc, Ts& ...>( func, args... );
		( workQueue.pushNext(
				static_cast<WorkBasic*>(workUnit), std::chrono::seconds( 2 )));
		
		return workUnit->acquireHook();
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
