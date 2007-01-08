// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ECHO_I_H
#define ECHO_I_H

#include <Echo.h>

class EchoI : public Demo::Echo
{
public:

    virtual std::string echoString(const std::string&, const Ice::Current&) const;
    virtual void shutdown(const Ice::Current&);
};

#endif
