// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

#include <errno.h>
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
// Magic number (4 chars)
// Protocol version major (char)
// Protocol version minor (char)
// Encoding version major (char)
// Encoding version minor (char)
// Message type (char)
// Compression status (char)
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

const char magic[] = { 0x49, 0x63, 0x65, 0x50 };	// 'I', 'c', 'e', 'P'

#ifdef __BCPLUSPLUS__
const char requestHdr[headerSize + sizeof(Ice::Int)] = 
#else
const char requestHdr[] = 
#endif
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

#ifdef __BCPLUSPLUS__
const char requestBatchHdr[headerSize + sizeof(Ice::Int)] =
#else
const char requestBatchHdr[] =
#endif
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

static int
readSize(char* base, int& offset)
{
    int sz = base[offset++];
    if(sz == 255)
    {
	return getInt(base, offset);
    }
    return sz;
}

static bool
rawcmp(char* src, int srcLen, char* dest, int offset, int len)
{
    if(srcLen != len)
    {
	return false;
    }
    for(int i = 0; i < len; ++i)
    {
	if(src[i] != dest[i+offset])
	{
	    return false;
	}
    }
    return true;
}

static void
run(SOCKET fd)
{
    char header[Protocol::headerSize];

    //
    // Construct validation message, send.
    //
    int offset = 0;
    memcpy(header, Protocol::magic, sizeof(Protocol::magic));
    offset += sizeof(Protocol::magic);
    header[offset++] = Protocol::protocolMajor;
    header[offset++] = Protocol::protocolMinor;
    header[offset++] = Protocol::encodingMajor;
    header[offset++] = Protocol::encodingMinor;
    header[offset++] = Protocol::validateConnectionMsg;
    header[offset++] = (char)0;
    putInt(header, offset, Protocol::headerSize);

    if(::send(fd, header, sizeof(header), 0) != sizeof(header))
    {
	return;
    }

    char req[500042];

    char reply[25];
    memcpy(reply, Protocol::replyHdr, sizeof(Protocol::replyHdr));
    offset = 10;
    putInt(reply, offset, sizeof(reply));
    putInt(reply, offset, 0); // request id
    reply[offset++] = (char)0; // Success
    // Encapsulation
    putInt(reply, offset, 6);
    reply[offset++] = Protocol::encodingMajor;
    reply[offset++] = Protocol::encodingMinor;

    char isaReply[26];
    memcpy(isaReply, Protocol::replyHdr, sizeof(Protocol::replyHdr));
    offset = 10;
    putInt(isaReply, offset, sizeof(isaReply));
    putInt(isaReply, offset, 0); // request id
    isaReply[offset++] = (char)0; // Success
    // Encapsulation
    putInt(isaReply, offset, 7);
    isaReply[offset++] = Protocol::encodingMajor;
    isaReply[offset++] = Protocol::encodingMinor;
    isaReply[offset++] = (char)1;

    char opIceIsA[] = "ice_isA";
    char opIcePing[] = "ice_ping";
    char opSendByteSeq[] = "sendByteSeq";

    //
    // Process requests.
    //
    while(true)
    {
	int len = 0;
	while(len != sizeof(header))
	{
	    int n = ::recv(fd, &header[len], sizeof(header) - len, 0);
	    if(n <= 0)
	    {
		return;
	    }
	    len += n;
	}

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
	//char compress = header[offset++];
	offset++;
	int size = getInt(header, offset);
	//Console.WriteLine("size: " + size);

	if(messageType == Protocol::closeConnectionMsg)
	{
	    return;
	}
	if(messageType != Protocol::requestMsg)
	{
	    cerr << "unexpected message: " << messageType << endl;
	    return;
	}
		    
	size -= Protocol::headerSize;
	if(size < 0 || size > sizeof(req))
	{
	    cerr << "booting client: unsupported size" << endl;
	    return;
	}
	//assert(size <= req.Length);

	len = 0;
	while(len != size)
	{
	    int n = ::recv(fd, &req[len], size - len, 0);
	    if(n <= 0)
	    {
		return;
	    }
	    //Console.WriteLine("read: " + n);
	    len += n;
	}

	offset = 0;
	int requestId = getInt(req, offset);

	// id
	int sz = readSize(req, offset);
	offset += sz;
	sz = readSize(req, offset);
	offset += sz;
	assert(req[offset] == 0);
	++offset; // facet
	// operation
	sz = readSize(req, offset);

	if(!rawcmp(opIceIsA, sizeof(opIceIsA)-1, req, offset, sz) &&
	   !rawcmp(opIcePing, sizeof(opIcePing)-1, req, offset, sz) &&
	   !rawcmp(opSendByteSeq, sizeof(opSendByteSeq)-1, req, offset, sz))
	{
	    string op(&req[offset], sz);
	    cerr << "unsupported op: " << op << endl;
	    return;
	}
	
	
	char* r;
	int l;
	if(rawcmp(opIceIsA, sizeof(opIceIsA)-1, req, offset, sz))
	{
	    r = isaReply;
	    l = sizeof(isaReply);
	}
	else
	{
	    r = reply;
	    l = sizeof(reply);
	}
	
	//
	// Compose the reply.
	//
	offset = 14;
	putInt(r, offset, requestId);

	if(::send(fd, r, l, 0) != l)
	{
	    return;
	}
    }
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

    SOCKET fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(fd == INVALID_SOCKET)
    {
#ifdef WIN32
        cerr << "Create socket failed! " << WSAGetLastError() << endl;
#else
        cerr << "Create socket failed! " << errno << endl;
#endif
	return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(bind(fd, reinterpret_cast<struct sockaddr*>(&addr), int(sizeof(addr))) == SOCKET_ERROR)
    {
        cerr << "Bind failed!" << endl;
	return EXIT_FAILURE;
    }

    if(::listen(fd, 5) == SOCKET_ERROR)
    {
        cerr << "Listen failed!" << endl;
	return EXIT_FAILURE;
    }

    cout << "Server ready" << endl;

    while(true)
    {
	SOCKET fd2 = ::accept(fd, 0, 0);
	if(fd2 == INVALID_SOCKET)
	{
	    cerr << "Accept failed!" << endl;
	    return EXIT_FAILURE;
	}
	cout << "Accepted new client" << endl;
	run(fd2);
	cout << "Disconnected client" << endl;
#ifdef WIN32
	::closesocket(fd2);
#else
	::close(fd2);
#endif
    }

    return EXIT_SUCCESS;
}
