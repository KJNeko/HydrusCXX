//
// Created by kj16609 on 4/8/22.
//

#ifndef HYDRUSCXX_HYDRUSTHREADING_HPP
#define HYDRUSCXX_HYDRUSTHREADING_HPP

#include <semaphore>
#include <thread>
#include <string>
#include <memory>
#include <queue>

#include <future>

#include "include/Extras/ringBuffer.hpp"

#include "spdlog/spdlog.h"

namespace HydrusCXX::Threading {
	namespace Internal {
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
			explicit WorkUnit( Func func, Args... arguments )
					:
					task( func ), args( arguments... )
			{}
			
			std::future<Ret> getFuture()
			{
				return task.get_future();
			}
			
			void doTask() override
			{
				if constexpr( std::tuple<>() == std::tuple<Args...>())
				{
					//We don't have any tasks
					task();
				}
				else
				{
					task( args );
				}
				
			}
		};
	}
	
	class ThreadManager
	{
		std::vector<std::thread> threads {};
		
		ringBuffer<Internal::WorkBasic*, 15> workQueue {};
		
		bool stopThreadsFlag { false };
		
		size_t threadCount = 4;
		
		void doWork()
		{
			spdlog::debug( "[HydrusCXX]: A thread has started" );
			while ( !stopThreadsFlag )
			{
				//GetWork
				auto work = workQueue.getNext_for(
						std::chrono::milliseconds( 500 ));
				
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
		
		static ThreadManager& getInstance()
		{
			static ThreadManager instance;
			return instance;
		}
		
		void startThreads()
		{
			if ( threadCount <= threads.size())
			{
				return;
			}
			
			stopThreadsFlag = false;
			for ( size_t i = 0; i < threadCount; ++i )
			{
				threads.emplace_back(
						&ThreadManager::doWork, this );
			}
			spdlog::info( "{} threads have been started", threadCount );
		}
		
		ThreadManager()
		{ startThreads(); }
		
		void stopThreads()
		{
			stopThreadsFlag = true;
			for ( auto& th : threads )
			{
				th.join();
			}
		}
		
		ThreadManager( ThreadManager const& ) = delete;
		
		ThreadManager operator=( ThreadManager const& ) = delete;
		
		template<typename Ret, typename Func, typename... Args>
		std::future<Ret> submit( Func func, Args& ... args )
		{
			auto* workUnit = new Internal::WorkUnit<Ret, Func, Args...>(
					func, args... );
			( workQueue.pushNext(
					static_cast<Internal::WorkBasic*>(workUnit),
					std::chrono::seconds( 2 )));
			
			return workUnit->getFuture();
		}
		
		~ThreadManager()
		{
			stopThreads();
		}
		
	};
}

#endif //HYDRUSCXX_HYDRUSTHREADING_HPP
