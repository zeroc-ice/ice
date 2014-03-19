// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#if defined(_MSC_VER) && _MSC_VER >= 1400
#    define _CRT_SECURE_NO_DEPRECATE 1  // C4996 '<C function>' was declared deprecated/
#    pragma warning( 4 : 4996 ) // C4996 'std::<function>' was declared deprecated
#endif

#ifdef _WIN32
#   include <winsock2.h>
typedef int ssize_t;
#   include <sys/timeb.h>
#else

#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <sys/time.h>

#   define SOCKET int
#   define SOCKET_ERROR -1
#   define INVALID_SOCKET -1

#endif

#include <iostream>
#include <string>
#include <assert.h>

#if defined(__i386)     || defined(_M_IX86) || defined(__x86_64)  || \
    defined(_M_X64)     || defined(_M_IA64) || defined(__alpha__) || \
    defined(__MIPSEL__)
#   define ICE_LITTLE_ENDIAN
#elif defined(__sparc) || defined(__sparc__) || defined(__hppa)      || \
      defined(__ppc__) || defined(_ARCH_COM) || defined(__MIPSEB__)
#   define ICE_BIG_ENDIAN
#else
#   error "Unknown architecture"
#endif

using namespace std;

//
// Ripped off from the Ice core.
//
namespace Protocol
{

//
// Size of the Ice protocol header
//
// Magic number (4 Bytes)
// Protocol version major (Byte)
// Protocol version minor (Byte)
// Encoding version major (Byte)
// Encoding version minor (Byte)
// Message type (Byte)
// Compression status (Byte)
// Message size (Int)
//
const int headerSize = 14;

//
// The current Ice protocol and encoding version
//
const char protocolMajor = 1;
const char protocolMinor = 0;
const char encodingMajor = 1;
const char encodingMinor = 0;

//
// The Ice protocol message types
//
const char requestMsg = 0;
const char requestBatchMsg = 1;
const char replyMsg = 2;
const char validateConnectionMsg = 3;
const char closeConnectionMsg = 4;

//
// The request header, batch request header and reply header.
//

const char magic[] = { 0x49, 0x63, 0x65, 0x50 };        // 'I', 'c', 'e', 'P'

const char requestHdr[] = 
{
    magic[0],
    magic[1],
    magic[2],
    magic[3],
    protocolMajor,
    protocolMinor,
    encodingMajor,
    encodingMinor,
    requestMsg,
    0, // Compression status
    0, 0, 0, 0, // Message size (placeholder)
    0, 0, 0, 0 // Request id (placeholder)
};

const char requestBatchHdr[] =
{
    magic[0],
    magic[1],
    magic[2],
    magic[3],
    protocolMajor,
    protocolMinor,
    encodingMajor,
    encodingMinor,
    requestBatchMsg,
    0, // Compression status
    0, 0, 0, 0, // Message size (place holder)
    0, 0, 0, 0  // Number of requests in batch (placeholder)
};

const char replyHdr[] = 
{
    magic[0],
    magic[1],
    magic[2],
    magic[3],
    protocolMajor,
    protocolMinor,
    encodingMajor,
    encodingMinor,
    replyMsg,
    0, // Compression status
    0, 0, 0, 0 // Message size (placeholder)
};

}

static void
putInt(char* base, int& offset, int v)
{
    char* dest = base+offset;
#ifdef ICE_BIG_ENDIAN
    const char* src = reinterpret_cast<const char*>(&v) + sizeof(Ice::Int) - 1;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest = *src;
#else
    const char* src = reinterpret_cast<const char*>(&v);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#endif
    offset += sizeof(int);
}

static int
getInt(const char* base, int& offset)
{
    const char* src = base+offset;
    offset += sizeof(int);
    int v;
#ifdef ICE_BIG_ENDIAN
    char* dest = reinterpret_cast<char*>(&v) + sizeof(Ice::Int) - 1;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest = *src;
#else
    char* dest = reinterpret_cast<char*>(&v);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#endif

    return v;
}

static void
run(SOCKET fd)
{
    const int seq = 500000;
    char req[seq + Protocol::headerSize + 42]; // 42 is the size of the message header
    memset(req, 0, sizeof(req));
    memcpy(req, Protocol::requestHdr, Protocol::headerSize);

    int offset = 10;
    putInt(req, offset, sizeof(req));
    putInt(req, offset, 0); // request id
    string idName = "throughput";
    req[offset++] = (char)idName.size();
    memcpy(&req[offset], idName.c_str(), idName.size());
    offset += idName.size();

    req[offset++] = 0;
    req[offset++] = 0; // facet

    string opName = "sendByteSeq"; // operation name
    req[offset++] = (char)opName.size();
    memcpy(&req[offset], opName.c_str(), opName.size());
    offset += opName.size();

    req[offset++] = 0; // mode
    req[offset++] = 0; // context

    // encapsulation for the sendByteSeq request data.
    putInt(req, offset, seq + 11);
    req[offset++] = Protocol::encodingMajor;
    req[offset++] = Protocol::encodingMinor;
    req[offset++] = (char)255; // number of elements.
    putInt(req, offset, seq);

    int requestId = 1;
    char reply[25];
    char header[14];
    char magic[4];

    // Read connection validation.
    int len = 0;
    int size = sizeof(header);
    while(len != size)
    {
        int n = ::recv(fd, &header[len], size - len, 0);
        if(n <= 0)
        {
            return;
        }
        len += n;
    }

    // Process validation message.
    assert(header[0] == Protocol::magic[0] && header[1] == Protocol::magic[1] &&
           header[2] == Protocol::magic[2] && header[3] == Protocol::magic[3]);
    offset = 4;

    char pMajor = header[offset++];
    assert(pMajor == Protocol::protocolMajor);
    //char pMinor = header[offset++];
    offset++;

    char eMajor = header[offset++];
    assert(eMajor == Protocol::encodingMajor);
    //char eMinor = header[offset++];
    offset++;

    char messageType = header[offset++];
    assert(messageType == Protocol::validateConnectionMsg);

    //char compress = header[offset++];
    offset++;
    size = getInt(header, offset);

#ifdef WIN32
    struct _timeb tb;
    _ftime(&tb);
    __int64 start = tb.time * 1000000 + tb.millitm * 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    long start = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

    // Send some requests. First we want to "warm up" the JIT compiler.
    int repetitions = 200;
    for (int i = 0; i < repetitions; ++i)
    {
        // Request message body.
        offset = 14;
        putInt(req, offset, requestId);
        ++requestId;

        if(::send(fd, req, sizeof(req), 0) != sizeof(req))
        {
            return;
        }

        len = 0;
        size = sizeof(reply);
        while(len != size)
        {
            int n = ::recv(fd, &reply[len], size-len, 0);
            if(n <= 0)
            {
                return;
            }
            len += n;
        }
    }

#ifdef WIN32
    _ftime(&tb);
    float tm = (tb.time * 1000000 + tb.millitm * 1000 - start) / 1000.0f;
#else
    gettimeofday(&tv, 0);
    float tm = (tv.tv_sec * 1000000 + tv.tv_usec - start) / 1000;
#endif
    cout << "time for " << repetitions << " sequences: " << (float) tm << "ms" << endl;
    cout << "time per sequence: " << (float) tm / repetitions << "ms" << endl;
}

int
main(int argc, char* argv[])
{
#ifdef _WIN32
    WORD version = MAKEWORD(1, 1);
    WSADATA data;
    if(WSAStartup(version, &data) != 0)
    {
        cerr << "WSAStartup failed!" << endl;
    }
#endif

    char buffer[64];

    SOCKET fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(fd == INVALID_SOCKET)
    {
        cerr << "Create socket failed!" << endl;
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), int(sizeof(addr))) == SOCKET_ERROR)
    {
        cerr << "Connect failed!" << endl;
        return EXIT_FAILURE;
    }

    run(fd);

    return EXIT_SUCCESS;
}
