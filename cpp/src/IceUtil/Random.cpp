// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Random.h>
#include <IceUtil/StaticMutex.h>

#ifdef _WIN32
#   include <Wincrypt.h>
#else
#   include <unistd.h>
#   include <fcntl.h>
#endif

using namespace std;
using namespace IceUtil;

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
static StaticMutex staticMutex = ICE_STATIC_MUTEX_INITIALIZER;
#ifdef _WIN32
static HCRYPTPROV context = NULL;
#else
static int fd = -1;
#endif

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
#ifdef _WIN32
        if(context != NULL)
        {
            CryptReleaseContext(context, 0);
            context = NULL;
        }
#else
        if(fd != -1)
        {
            close(fd);
            fd = -1;
        }
#endif
    }
};
static RandomCleanup uuidCleanup;
}

void
IceUtilInternal::generateRandom(char* buffer, int size)
{
#ifdef _WIN32
    //
    // It's not clear from the Microsoft documentation if CryptGenRandom 
    // can be called concurrently from several threads. To be on the safe
    // side, we also serialize calls to to CryptGenRandom with the static 
    // mutex.
    //

    IceUtil::StaticMutex::Lock lock(staticMutex);
    if(context == NULL)
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
#else

    //
    // Serialize access to /dev/urandom; see comment above.
    //
    StaticMutex::Lock lock(staticMutex);
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

int
IceUtilInternal::random(int limit)
{
    int r;
    generateRandom(reinterpret_cast<char*>(&r), static_cast<int>(sizeof(int)));
    if(limit > 0)
    {
        r = r % limit;
    }
    if(r < 0)
    {
        r = -r;
    }
    return r;
}
