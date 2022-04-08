//
// Created by kj16609 on 4/8/22.
//

#ifndef HYDRUSCXX_HYDRUSCXXTHREADING_HPP
#define HYDRUSCXX_HYDRUSCXXTHREADING_HPP

#include <semaphore>
#include <thread>
#include <string>
#include <memory>

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
	std::binary_semaphore lockFlag { 1 };

public:
	bool checkComplete()
	{
		auto retBool = lockFlag.try_acquire();
		if ( retBool )
		{
			//We managed to aquired
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
class WorkUnit : public WorkBasic
{
	TFunc func;
	std::tuple<Ts...> args;
	std::shared_ptr<WorkHook> hook;
	bool hookAquired { false };

public:
	WorkUnit( TFunc function, Ts... argsPack )
			:
			func( function ), args( argsPack... ), hook( new WorkHook )
	{
	}
	
	std::shared_ptr<WorkHook> acquireHook() override
	{
		hookAquired = true;
		return hook;
	}
	
	void doWork() override
	{
		if ( !hookAquired )
		{
			spdlog::warn(
					"Attempting to do work without acquiring the hook first. Skipping work" );
			return;
		}
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
	
	std::vector<std::unique_ptr<WorkBasic>> workList {};
	
	std::mutex workLock {};
	
	void pullAndDo();
	
	bool stopThreads { false };
	
	void doWork()
	{
		while ( !stopThreads )
		{
			//GetWork
			workLock.lock();
			if ( workList.size() == 0 )
			{
				std::this_thread::yield();
				continue;
			}
			
			std::unique_ptr<WorkBasic> work( workList.back().release());
			workList.pop_back();
			workLock.unlock();
			
			work->doWork();
		}
		spdlog::info( "A thread has stopped" );
	}
	
	//ringBuffer for work to be done
public:
	HydrusCXXThreadManager( size_t size )
	{
		if ( size == 0 )
		{
			spdlog::warn( "wtf are you doing?" );
		}
		if ( size > std::thread::hardware_concurrency())
		{
			spdlog::warn(
					"{} threads where allocated when the system reports only {} threads",
					size, std::thread::hardware_concurrency());
		}
		
		for ( size_t i = 0; i < size; ++i )
		{
			threads.push_back(
					std::thread( &HydrusCXXThreadManager::doWork, this ));
		}
	}
	
	template<typename TFunc, typename... Ts>
	std::shared_ptr<WorkHook> addWork( TFunc func, Ts... args )
	{
		WorkUnit<TFunc, Ts...>* workUnit = new WorkUnit<TFunc, Ts...>(
				func, args... );
		
		workList.emplace_back(
				static_cast<WorkBasic*>(workUnit));
		auto ptr = workList.back()->acquireHook();
		if ( ptr == nullptr )
		{
			throw std::runtime_error(
					"Hook pointer for work unit was nullptr" );
		}
		else
		{
			return ptr;
		}
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
