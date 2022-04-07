//
// Created by kj16609 on 4/6/22.
//

#ifndef HYDRUSCXX_HYDRUCXXUTILS_HPP
#define HYDRUSCXX_HYDRUCXXUTILS_HPP

#include <string>
#include <sys/ioctl.h>

inline size_t getColumnCount()
{
	winsize w;
	ioctl( 0, TIOCGWINSZ, &w );
	return w.ws_col;
}

inline std::string formatBytesize( size_t count )
{
	if ( count > 1000 )
	{
		//B
		if ( count > 1000000 )
		{
			//KB
			if ( count > 1000000000 )
			{
				return std::to_string( count / 1000000000 ) + "GB";
				
			}
			else
			{
				return std::to_string( count / 1000000 ) + "MB";
			}
		}
		else
		{
			return std::to_string( count / 1000 ) + "KB";
		}
	}
	else
	{
		return std::to_string( count ) + "B";
	}
}

#endif //HYDRUSCXX_HYDRUCXXUTILS_HPP
