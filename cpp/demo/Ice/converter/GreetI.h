// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ECHO_I_H
#define ECHO_I_H

#include <Greet.h>

class GreetI : public Demo::Greet
{
public:

    virtual std::string exchangeGreeting(const std::string&, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

#endif
