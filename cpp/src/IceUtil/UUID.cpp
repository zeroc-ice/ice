//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/UUID.h>
#include <IceUtil/Exception.h>
// On Windows, we use Windows's RPC UUID generator. On other platforms, we use a high quality random number generator
// (std::random_device) to generate "version 4" UUIDs, as described in
// http://www.ietf.org/internet-drafts/draft-mealling-uuid-urn-00.txt
#ifdef _WIN32
#   include <rpc.h>
#else
#   include <IceUtil/Random.h>
#endif

using namespace std;

namespace
{

// Helper char to hex functions
inline void halfByteToHex(unsigned char hb, char*& hexBuffer)
{
    if(hb < 10)
    {
        *hexBuffer++ = '0' + static_cast<char>(hb);
    }
    else
    {
        *hexBuffer++ = 'A' + static_cast<char>(hb - 10);
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

}

string
IceUtil::generateUUID()
{
#if defined(_WIN32)

    UUID uuid;
    RPC_STATUS ret = UuidCreate(&uuid);
    if(ret != RPC_S_OK && ret != RPC_S_UUID_LOCAL_ONLY && ret != RPC_S_UUID_NO_ADDRESS)
    {
        throw SyscallException(__FILE__, __LINE__);
    }

    unsigned char* str;

    ret = UuidToString(&uuid, &str);
    if(ret != RPC_S_OK)
    {
        throw SyscallException(__FILE__, __LINE__);
    }
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

    // Get a random sequence of bytes.
    char* buffer = reinterpret_cast<char*>(&uuid);
    IceUtilInternal::generateRandom(buffer, sizeof(UUID));

    // Adjust the bits that say "version 4" UUID
    uuid.timeHighAndVersion[0] &= 0x0F;
    uuid.timeHighAndVersion[0] |= (4 << 4);
    uuid.clockSeqHiAndReserved &= 0x3F;
    uuid.clockSeqHiAndReserved |= 0x80;

    // Convert to a UUID string
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
