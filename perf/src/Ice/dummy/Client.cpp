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
#else
#   include <unistd.h>
#   include <fcntl.h>
#   include <sys/socket.h>

#   if defined(__hpux)
#      include <sys/time.h>
#   else   
#      include <sys/select.h>
#   endif

#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#endif

#ifdef _WIN32
#   include <sys/timeb.h>
#else
#   include <sys/time.h>
#endif

#ifndef _WIN32
#   define SOCKET int
#   define SOCKET_ERROR -1
#   define INVALID_SOCKET -1
#endif

#include <iostream>

using namespace std;

char request[41] = { 0x49, 0x63, 0x65, 0x50, 0x01, 0x00, 0x01, 0x00,
                     0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x01, 0x00,
		     0x00, 0x00, 0x07, 0x6c, 0x61, 0x74, 0x65, 0x6e,
		     0x63, 0x79, 0x00, 0x00, 0x04, 0x70, 0x69, 0x6e,
		     0x67, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01,
		     0x00 };

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

    if(::recv(fd, buffer, 14, 0) != 14)
    {
        cerr << "Read connection validation failed!" << endl;
        return EXIT_FAILURE;
    }

#ifdef WIN32
    struct _timeb tb;
    _ftime(&tb);
    __int64 start = tb.time * 1000000 + tb.millitm * 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    long start = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

    int repetitions = 100000;
    for(int i = 0; i < repetitions; ++i)
    {
        char* id = reinterpret_cast<char*>(&repetitions);
	for(unsigned int j = 0; j < sizeof(int); ++j)
	{
	    request[j + 14] = id[j];
	}

        if(::send(fd, request, 41, 0) != 41)
        {
            cerr << "Send request failed!" << endl;
            return EXIT_FAILURE;
        }

        if(::recv(fd, buffer, 25, 0) != 25)
        {
            cerr << "Read response failed!" << endl;
            return EXIT_FAILURE;
        }
    }

#ifdef WIN32
    _ftime(&tb);
    float tm = (tb.time * 1000000 + tb.millitm * 1000 - start) / 1000.0f;
#else
    gettimeofday(&tv, 0);
    float tm = (tv.tv_sec * 1000000 + tv.tv_usec - start) / 1000;
#endif
    cout << (float) tm / repetitions << endl;

    return EXIT_SUCCESS;
}
