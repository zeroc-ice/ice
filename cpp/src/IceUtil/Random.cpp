// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _MSC_VER
#  define _CRT_RAND_S
#endif

#include <IceUtil/Random.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

#ifdef _WIN32
#   include <Wincrypt.h>
#else
#   include <unistd.h>
#   include <fcntl.h>
#endif

using namespace std;
using namespace IceUtil;

#if !defined(_WIN32) || !defined(_MSC_VER)
namespace
{

//
// The static mutex is required to lazy initialize the file
// descriptor for /dev/urandom (Unix) or the cryptographic 
// context (Windows).
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
#ifdef _WIN32
HCRYPTPROV context = 0;
#else
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
#endif

class Init
{
public:

    Init()
    {
        staticMutex = new IceUtil::Mutex;
#ifndef _WIN32
        //
        // Register a callback to reset the "/dev/urandom" fd 
        // state after fork.
        //
        pthread_atfork(0, 0, &childAtFork);
#endif
    }
    
    ~Init()
    {
#ifdef _WIN32
        if(context != 0)
        {
            CryptReleaseContext(context, 0);
            context = 0;
        }
#else
        if(fd != -1)
        {
            close(fd);
            fd = -1;
        }
#endif
        delete staticMutex;
        staticMutex = 0;
    }
};

Init init;

}
#endif

void
IceUtilInternal::generateRandom(char* buffer, int size)
{
#ifdef _WIN32

#  if defined(_MSC_VER)
    for(int i = 0; i < size; ++i)
    {
        buffer[i] = random(256);
    }
#  else
    //
    // It's not clear from the Microsoft documentation if CryptGenRandom 
    // can be called concurrently from several threads. To be on the safe
    // side, we also serialize calls to CryptGenRandom with the static 
    // mutex.
    //

    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(staticMutex);
    if(context == 0)
    {
        if(!CryptAcquireContext(&context, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        {
            throw SyscallException(__FILE__, __LINE__, GetLastError());
        }
    }

    if(!CryptGenRandom(context, size, reinterpret_cast<unsigned char*>(buffer)))
    {
        throw SyscallException(__FILE__, __LINE__, GetLastError());
    }
#  endif

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
            assert(0);
            throw SyscallException(__FILE__, __LINE__, errno);
        }
    }
    
    //
    // Limit the number of attempts to 20 reads to avoid
    // a potential "for ever" loop
    //
    int reads = 0;
    size_t index = 0;    
    while(reads <= 20 && index != static_cast<size_t>(size))
    {
        ssize_t bytesRead = read(fd, buffer + index, static_cast<size_t>(size) - index);
        
        if(bytesRead == -1 && errno != EINTR)
        {
            SyscallException ex(__FILE__, __LINE__, errno);
            cerr << "Reading /dev/urandom failed:\n" << ex << endl;
            assert(0);
            throw ex;
        }
        else
        {
            index += bytesRead;
            reads++;
        }
    }
        
    if(index != static_cast<size_t>(size))
    {
        assert(0);
        throw SyscallException(__FILE__, __LINE__, 0);
    }
#endif
}

unsigned int
IceUtilInternal::random(int limit)
{
    unsigned int r;
#if defined(_MSC_VER)
    errno_t err = rand_s(&r);
    if(err != 0)
    {
        SyscallException ex(__FILE__, __LINE__, errno);
        cerr << "rand_s failed:\n" << ex << endl;
        assert(0);
        throw ex;
    }
#else
    generateRandom(reinterpret_cast<char*>(&r), static_cast<unsigned int>(sizeof(unsigned int)));
#endif
    if(limit > 0)
    {
        r = r % limit;
    }
    return r;
}
