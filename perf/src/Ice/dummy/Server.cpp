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

const char validate[14] = { 0x49, 0x63, 0x65, 0x50, 0x01, 0x00, 0x01, 0x00,
                            0x03, 0x00, 0x0e, 0x00, 0x00, 0x00 };

char response[25] = { 0x49, 0x63, 0x65, 0x50, 0x01, 0x00, 0x01, 0x00,
                      0x02, 0x00, 0x19, 0x00, 0x00, 0x00, 0x01, 0x00,
                      0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01,
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
        cerr << "Create socket failed! " << WSAGetLastError() << endl;
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

    cout << "Latency ready" << endl;

    SOCKET fd2 = ::accept(fd, 0, 0);
    if(fd2 == INVALID_SOCKET)
    {
        cerr << "Accept failed!" << endl;
	return EXIT_FAILURE;
    }

    if(::send(fd2, validate, 14, 0) != 14)
    {
        cerr << "Send validation failed!" << endl;
        return EXIT_FAILURE;
    }

    while(true)
    {
        if(::recv(fd2, buffer, 14, 0) != 14)
        {
            cerr << "Read request header failed!" << endl;
            return EXIT_FAILURE;
        }

        if(::recv(fd2, buffer, 27, 0) != 27)
        {
            cerr << "Read request failed!" << endl;
            return EXIT_FAILURE;
        }

	for(unsigned int i = 0; i < sizeof(int); ++i)
	{
	    response[i + 14] = buffer[i];
	}

        if(::send(fd2, response, 25, 0) != 25)
        {
            cerr << "Send response failed!" << endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
