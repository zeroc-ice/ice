// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#include <IceUtil/UUID.h>

// On Windows, we use Windows's RPC UUID generator.
// On other platforms, we use a high quality random number generator 
// (/dev/random) to generate "version 4" UUIDs, as described in 
// http://www.ietf.org/internet-drafts/draft-mealling-uuid-urn-00.txt

#ifdef _WIN32
#   include <rpc.h>
#else
#include <sys/types.h>
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

string IceUtil::UUIDGenerationException::_name = "IceUtil::UUIDGenerationException";

const string&
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
#ifdef _WIN32

    UUID uuid;
    UuidCreate(&uuid);

    unsigned char* str;

    UuidToString(&uuid, &str);

    string result = reinterpret_cast<char*>(str);

    RpcStringFree(&str);
    return result;
    
#else

    // On Linux, /dev/random, even when used in blocking mode, sometimes 
    // fails or returns fewer bytes.
    // Maybe we should use a combination of version 4 UUIDs (with /dev/random),
    // and version 1 UUIDs (MAC address + time), when /dev/random is exhausted?

    int fd = open("/dev/urandom", O_RDONLY);

    if (fd == -1)
    {
	assert(0);
	throw UUIDGenerationException(__FILE__, __LINE__);
    }

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

    // Limit the number of attempts to 20 reads to avoid
    // a potential "for ever" loop
    //
    while(reads <= 20 && index != sizeof(UUID))
    {
	ssize_t bytesRead = read(fd, buffer + index, sizeof(UUID) - index);
	
	if(bytesRead == -1 && errno != EINTR)
	{
	    int err = errno;
	    cerr << "Reading /dev/urandom returned " << strerror(err) << endl;
	    assert(0);
	    close(fd);
	    throw UUIDGenerationException(__FILE__, __LINE__);
	}
	else
	{
	    index += bytesRead;
	    reads++;
	}
    }
    
    if (index != sizeof(UUID))
    {
	assert(0);
	close(fd);
	throw UUIDGenerationException(__FILE__, __LINE__);
    }
    
    close(fd);
    
    // Adjust the bits that say "version 4" UUID
    //
    uuid.timeHighAndVersion[0] &= 0x0F;
    uuid.timeHighAndVersion[0] |= (4 << 4);
    uuid.clockSeqHiAndReserved &= 0x3F;
    uuid.clockSeqHiAndReserved |= 0x80;

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
