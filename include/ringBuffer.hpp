//
// Created by kj16609 on 3/24/22.
//

#ifndef HYDRUSCXX_RINGBUFFER_HPP
#define HYDRUSCXX_RINGBUFFER_HPP

#include <array>
#include <functional>
#include <semaphore>
#include <stdexcept>
#include <vector>

template <typename T, int num>
requires std::is_trivial_v<T> && std::is_assignable_v<T&, T>
class ringBuffer
{
  private:
	std::array<T, num> buffer;

	T* read { buffer.begin() };
	T* write { buffer.begin() };

	std::mutex readLock;
	std::mutex writeLock;

	std::counting_semaphore<num> writeCounter { num };
	std::counting_semaphore<num> readCounter { 0 };

#ifdef HYUNSAFE
  public:
#endif

	T& getNext_UNSAFE()
	{
		auto ret = read;
		read++;

		if ( read > buffer.end() )
		{
			// Out of bounds
			read = buffer.data();
		}

		return ret;
	}

  public:
	void operateNext( std::function<void( T& )> func )
	{
		std::lock_guard<std::mutex> lock( readLock );
		readCounter.acquire();

		if ( read > buffer.end() )
		{
			read = buffer.data();
		}

		// Do the operation
		func( getNext_UNSAFE() );

		writeCounter.release();
		return;
	}

	T getNext()
	{
		std::lock_guard<std::mutex> lock( readLock );

		readCounter.acquire();

		auto ret = read;
		read++;

		if ( read > buffer.end() )
		{
			// Out of bounds
			read = buffer.data();
		}

		writeCounter.release();
		return *ret;
	}

	void pushNext( T var )
	{
		std::lock_guard<std::mutex> lock( writeLock );

		writeCounter.acquire();

		if ( write > buffer.end() )
		{
			write = buffer.data();
		}

		*write = var;
		write++;

		readCounter.release();
		return;
	}
};


#endif // HYDRUSCXX_RINGBUFFER_HPP
