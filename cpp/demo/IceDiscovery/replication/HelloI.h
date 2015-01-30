// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef HELLO_I_H
#define HELLO_I_H

#include <Hello.h>

class HelloI : public Demo::Hello
{
public:

    HelloI(const std::string&);

    virtual std::string getGreeting(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

	// Required to prevent compiler warnings with MSVC++
	HelloI& operator=(const HelloI&);

    const std::string _name;
};

#endif
