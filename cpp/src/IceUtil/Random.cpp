// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Random.h>

#ifdef _WIN32
// TODO
#else
#   include <IceUtil/StaticMutex.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <unistd.h>
#endif

using namespace std;


IceUtil::RandomGeneratorException::RandomGeneratorException(const char* file, int line) :
    Exception(file, line)
{
}

const char* IceUtil::RandomGeneratorException::_name = "IceUtil::RandomGeneratorException";

#ifndef _WIN32
//
// Unfortunately on Linux (at least up to 2.6.9), concurrent access to /dev/urandom
// can return the same value. Search for "Concurrent access to /dev/urandom" in the 
// linux-kernel mailing list archive for additional details.
// Since /dev/urandom on other platforms is usually a port from Linux, this problem 
// could be widespread. Therefore, we serialize access to /dev/urandom using a static 
// mutex.
//
static IceUtil::StaticMutex staticMutex = ICE_STATIC_MUTEX_INITIALIZER;
static int fd = -1;

namespace
{

//
// Close fd at exit
//
class RandomCleanup
{
public:
    
    ~RandomCleanup()
    {
         IceUtil::StaticMutex::Lock lock(staticMutex);
	 if(fd != -1)
	 {
	     close(fd);
	     fd = -1;
	 }
    }
};
static RandomCleanup uuidCleanup;
}


#endif

const string
IceUtil::RandomGeneratorException::ice_name() const
{
    return _name;
}

IceUtil::Exception*
IceUtil::RandomGeneratorException::ice_clone() const
{
    return new RandomGeneratorException(*this);
}

void
IceUtil::RandomGeneratorException::ice_throw() const
{
    throw *this;
}

void
IceUtil::generateRandom(char* buffer, int size)
{
#ifdef _WIN32
    //
    // TODO: XXX
    //
#else
    int reads = 0;
    size_t index = 0;

    {
	//
	// Serialize access to /dev/urandom; see comment above.
	//
	IceUtil::StaticMutex::Lock lock(staticMutex);
	if(fd == -1)
	{
	    fd = open("/dev/urandom", O_RDONLY);
	    if (fd == -1)
	    {
		assert(0);
		throw RandomGeneratorException(__FILE__, __LINE__);
	    }
	}
	
	//
	// Limit the number of attempts to 20 reads to avoid
	// a potential "for ever" loop
	//
	while(reads <= 20 && index != static_cast<size_t>(size))
	{
	    ssize_t bytesRead = read(fd, buffer + index, static_cast<size_t>(size) - index);
	    
	    if(bytesRead == -1 && errno != EINTR)
	    {
		int err = errno;
		cerr << "Reading /dev/urandom returned " << strerror(err) << endl;
		assert(0);
		throw RandomGeneratorException(__FILE__, __LINE__);
	    }
	    else
	    {
		index += bytesRead;
		reads++;
	    }
	}
    }
	
    if(index != static_cast<size_t>(size))
    {
	assert(0);
	throw RandomGeneratorException(__FILE__, __LINE__);
    }
#endif
}

int
IceUtil::random(int limit)
{
    int r;
    generateRandom(reinterpret_cast<char*>(&r), sizeof(int));
    return r % limit;
}
