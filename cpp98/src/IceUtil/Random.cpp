//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifdef _WIN32
#  define _CRT_RAND_S
#endif

#include <IceUtil/Random.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

#ifndef _WIN32
#   include <unistd.h>
#   include <fcntl.h>
#endif

using namespace std;
using namespace IceUtil;

#if !defined(_WIN32)
namespace
{

//
// The static mutex is required to lazy initialize the file
// descriptor for /dev/urandom (Unix)
//
// Also, unfortunately on Linux (at least up to 2.6.9), concurrent
// access to /dev/urandom can return the same value. Search for
// "Concurrent access to /dev/urandom" in the linux-kernel mailing
// list archive for additional details.  Since /dev/urandom on other
// platforms is usually a port from Linux, this problem could be
// widespread. Therefore, we serialize access to /dev/urandom using a
// static mutex.
//
Mutex* staticMutex = 0;
int fd = -1;

//
// Callback to use with pthread_atfork to reset the "/dev/urandom"
// fd state. We don't need to close the fd here as that is done
// during static destruction.
//
extern "C"
{

void childAtFork()
{
    if(fd != -1)
    {
        fd = -1;
    }
}

}

class Init
{
public:

    Init()
    {
        staticMutex = new IceUtil::Mutex;

        //
        // Register a callback to reset the "/dev/urandom" fd
        // state after fork.
        //
        pthread_atfork(0, 0, &childAtFork);
    }

    ~Init()
    {
        if(fd != -1)
        {
            close(fd);
            fd = -1;
        }
        delete staticMutex;
        staticMutex = 0;
    }
};

Init init;

}
#endif

void
IceUtilInternal::generateRandom(char* buffer, size_t size)
{
#ifdef _WIN32
    int i = 0;
    const size_t randSize = sizeof(unsigned int);

    while(size - i >= randSize)
    {
        unsigned int r = 0;
        errno_t err = rand_s(&r);
        if(err != 0)
        {
            throw SyscallException(__FILE__, __LINE__, errno);
        }
        memcpy(buffer + i, &r, randSize);
        i += randSize;
    }

    if(size - i > 0)
    {
        assert(size - i < randSize);
        unsigned int r = 0;
        errno_t err = rand_s(&r);
        if(err != 0)
        {
            throw SyscallException(__FILE__, __LINE__, errno);
        }
        memcpy(buffer + i, &r, size - i);
    }
#else
    //
    // Serialize access to /dev/urandom; see comment above.
    //
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(staticMutex);
    if(fd == -1)
    {
        fd = open("/dev/urandom", O_RDONLY);
        if(fd == -1)
        {
            throw SyscallException(__FILE__, __LINE__, errno);
        }
    }

    //
    // Limit the number of attempts to 20 reads to avoid
    // a potential "for ever" loop
    //
    int reads = 0;
    size_t index = 0;
    while(reads <= 20 && index != size)
    {
        ssize_t bytesRead = read(fd, buffer + index, size - index);

        if(bytesRead == -1 && errno != EINTR)
        {
            throw SyscallException(__FILE__, __LINE__, errno);
        }
        else
        {
            index += static_cast<size_t>(bytesRead);
            reads++;
        }
    }

    if(index != size)
    {
        throw SyscallException(__FILE__, __LINE__, 0);
    }
#endif
}

unsigned int
IceUtilInternal::random(int limit)
{
    unsigned int r;
#ifdef _WIN32
    errno_t err = rand_s(&r);
    if(err != 0)
    {
        throw SyscallException(__FILE__, __LINE__, errno);
    }
#else
    generateRandom(reinterpret_cast<char*>(&r), sizeof(unsigned int));
#endif
    if(limit > 0)
    {
        r = r % static_cast<unsigned int>(limit);
    }
    return r;
}
