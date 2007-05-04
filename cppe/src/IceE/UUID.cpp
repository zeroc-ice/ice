// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>
#include <IceE/UUID.h>

// On Windows, we use Windows's RPC UUID generator.
//
// Under WinCE we use the Cryptography functions to generate random
// numbers which should be suitable for a "version 4" UUID.
//
// On other platforms, we use a high quality random number generator 
// (/dev/random) to generate "version 4" UUIDs, as described in 
// http://www.ietf.org/internet-drafts/draft-mealling-uuid-urn-00.txt
//


#ifdef _WIN32_WCE
#   include <wincrypt.h>
#   include <IceE/StaticMutex.h>
#elif defined(_WIN32)
#   include <rpc.h>
#else
#   include <IceE/StaticMutex.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <unistd.h>
#endif

using namespace std;

// Helper char to hex functions
//
inline void halfByteToHex(unsigned char hb, char*& hexBuffer)
{
    if(hb < 10)
    {
	*hexBuffer++ = '0' + hb;
    }
    else
    {
	*hexBuffer++ = 'A' + (hb - 10);
    } 
}

inline void bytesToHex(unsigned char* bytes, size_t len, char*& hexBuffer)
{
    for(size_t i = 0; i < len; i++)
    {
	halfByteToHex((bytes[i] & 0xF0) >> 4, hexBuffer);
	halfByteToHex((bytes[i] & 0x0F), hexBuffer);
    }
}


IceUtil::UUIDGenerationException::UUIDGenerationException(const char* file, int line) :
    Exception(file, line)
{
}

const char* IceUtil::UUIDGenerationException::_name = "IceUtil::UUIDGenerationException";

#ifdef _WIN32_WCE

static IceUtil::StaticMutex staticMutex = ICE_STATIC_MUTEX_INITIALIZER;
static HCRYPTPROV cryptProv = 0;

namespace
{

//
// Close the crypt provider on exit.
//
class UUIDCleanup
{
public:
    
    ~UUIDCleanup()
    {
         IceUtil::StaticMutex::Lock lock(staticMutex);
	 if(cryptProv != 0)
	 {
	     CryptReleaseContext(cryptProv, 0);
	     cryptProv = 0;
	 }
    }
};

static UUIDCleanup uuidCleanup;

}

#elif !defined(_WIN32)
//
// Unfortunately on Linux (at least up to 2.6.9), concurrent access to /dev/urandom
// can return the same value. Search for "Concurrent access to /dev/urandom" in the 
// linux-kernel mailing list archive for additional details.
// Since /dev/urandom on other platforms is usually a port from Linux, this problem 
// could be widespread. Therefore, instead of using 122 random bits that could be 
// duplicated, we replace the last 15 bits of all "random" UUIDs by the last 15 bits 
// of the process id, and in each process, we serialize access to /dev/urandom using 
// a static mutex.
//

static IceUtil::StaticMutex staticMutex = ICE_STATIC_MUTEX_INITIALIZER;
static int fd = -1;
static char myPid[2];

namespace
{

//
// Close fd at exit
//
class UUIDCleanup
{
public:
    
    ~UUIDCleanup()
    {
         IceUtil::StaticMutex::Lock lock(staticMutex);
	 if(fd != -1)
	 {
	     close(fd);
	     fd = -1;
	 }
    }
};
static UUIDCleanup uuidCleanup;
}


#endif

string
IceUtil::UUIDGenerationException::ice_name() const
{
    return _name;
}

IceUtil::Exception*
IceUtil::UUIDGenerationException::ice_clone() const
{
    return new UUIDGenerationException(*this);
}

void
IceUtil::UUIDGenerationException::ice_throw() const
{
    throw *this;
}


string
IceUtil::generateUUID()
{
#if defined(_WIN32 ) && !defined(_WIN32_WCE)

    UUID uuid;
    UuidCreate(&uuid);

    unsigned char* str;

    UuidToString(&uuid, &str);

    string result = reinterpret_cast<char*>(str);

    RpcStringFree(&str);
    return result;
    
#else

    struct UUID
    {
	unsigned char timeLow[4];
	unsigned char timeMid[2];
	unsigned char timeHighAndVersion[2];
	unsigned char clockSeqHiAndReserved;
	unsigned char clockSeqLow;
	unsigned char node[6];
    };
    UUID uuid;

    assert(sizeof(UUID) == 16);

    char* buffer = reinterpret_cast<char*>(&uuid);
    int reads = 0;
    size_t index = 0;

#ifdef _WIN32_WCE

    HCRYPTPROV localProv;

    {
	IceUtil::StaticMutex::Lock lock(staticMutex);
	if(cryptProv == 0)
	{
	    if(!CryptAcquireContext(&cryptProv, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	    {
		throw UUIDGenerationException(__FILE__, __LINE__);
	    }
	}
	localProv = cryptProv;
    }

    memset(buffer, 0, 16);
    if(!CryptGenRandom(localProv, 16, (unsigned char*)buffer))
    {
	throw UUIDGenerationException(__FILE__, __LINE__);
    }

#else
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
		throw UUIDGenerationException(__FILE__, __LINE__);
	    }
	    
	    //
	    // Initialize myPid as well
	    // 
	    pid_t pid = getpid();
	    myPid[0] = (pid >> 8) & 0x7F;
	    myPid[1] = pid & 0xFF;
	}
	

	//
	// Limit the number of attempts to 20 reads to avoid
	// a potential "for ever" loop
	//
	while(reads <= 20 && index != sizeof(UUID))
	{
	    ssize_t bytesRead = read(fd, buffer + index, sizeof(UUID) - index);
	    
	    if(bytesRead == -1 && errno != EINTR)
	    {
		int err = errno;
		fprintf(stderr, "Reading /dev/urandom returned %s\n", strerror(err));
		assert(0);
		throw UUIDGenerationException(__FILE__, __LINE__);
	    }
	    else
	    {
		index += bytesRead;
		reads++;
	    }
	}
    }
	
    if (index != sizeof(UUID))
    {
	assert(0);
	throw UUIDGenerationException(__FILE__, __LINE__);
    }
   
    //
    // Replace the end of the node by myPid (15 bits) 
    //
    uuid.node[4] = (uuid.node[4] & 0x80) | myPid[0];
    uuid.node[5] = myPid[1];
#endif

    //
    // Adjust the bits that say "version 4" UUID
    //
    uuid.timeHighAndVersion[0] &= 0x0F;
    uuid.timeHighAndVersion[0] |= (4 << 4);
    uuid.clockSeqHiAndReserved &= 0x3F;
    uuid.clockSeqHiAndReserved |= 0x80;

    //
    // Convert to a UUID string
    //
    char uuidString[16 * 2 + 4 + 1]; // 16 bytes, 4 '-' and a final '\0'
    char* uuidIndex = uuidString;
    bytesToHex(uuid.timeLow, sizeof(uuid.timeLow), uuidIndex);
    *uuidIndex++ = '-';
    bytesToHex(uuid.timeMid, sizeof(uuid.timeMid), uuidIndex);
    *uuidIndex++ = '-';
    bytesToHex(uuid.timeHighAndVersion, sizeof(uuid.timeHighAndVersion), uuidIndex);
    *uuidIndex++ = '-';
    bytesToHex(&uuid.clockSeqHiAndReserved, sizeof(uuid.clockSeqHiAndReserved), uuidIndex);
    bytesToHex(&uuid.clockSeqLow, sizeof(uuid.clockSeqLow), uuidIndex);
    *uuidIndex++ = '-';
    bytesToHex(uuid.node, sizeof(uuid.node), uuidIndex);
    *uuidIndex = '\0';

    return uuidString;

#endif
}
