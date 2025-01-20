// Copyright (c) ZeroC, Inc.

#include "Ice/UUID.h"

// We use a high quality random number generator
// (std::random_device) to generate "version 4" UUIDs, as described in
// http://www.ietf.org/internet-drafts/draft-mealling-uuid-urn-00.txt
#include "Random.h"

using namespace std;

namespace
{
    // Helper char to hex functions
    inline void halfByteToHex(unsigned char hb, char*& hexBuffer)
    {
        if (hb < 10)
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
        for (size_t i = 0; i < len; i++)
        {
            halfByteToHex((bytes[i] & 0xF0) >> 4, hexBuffer);
            halfByteToHex((bytes[i] & 0x0F), hexBuffer);
        }
    }
}

string
Ice::generateUUID()
{
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

    static_assert(sizeof(UUID) == 16);

    // Get a random sequence of bytes.
    char* buffer = reinterpret_cast<char*>(&uuid);
    IceInternal::generateRandom(buffer, sizeof(UUID));

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
}
