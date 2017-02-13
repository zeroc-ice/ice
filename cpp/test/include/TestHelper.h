// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <cassert>
#include <cstdlib>
#include <string>
#include <iostream>

namespace Test
{

class MainHelper : public std::streambuf
{
public:

    MainHelper()
    {
        setp(&data[0], &data[sizeof(data) - 1]);
    }

    virtual void serverReady() = 0;

    virtual void shutdown() = 0;

    virtual void waitForCompleted() = 0;

    virtual bool redirect() = 0;

    virtual void print(const std::string& msg) = 0;

    virtual void
    flush()
    {
    }

    virtual void
    newLine()
    {
        print("\n");
    }

private:

    //
    // streambuf redirection implementation
    //

    int sync()
    {
        std::streamsize n = pptr() - pbase();
        print(std::string(pbase(), static_cast<int>(n)));
        pbump(-static_cast<int>(pptr() - pbase()));
        return 0;
    }

    int overflow(int ch)
    {
        sync();
        if(ch != EOF)
        {
            assert(pptr() != epptr());
            sputc(ch);
        }
        return 0;
    }

    int sputc(char c)
    {
        if(c == '\n')
        {
            pubsync();
        }
        return std::streambuf::sputc(c);
    }

    char data[1024];
};

}
#endif
