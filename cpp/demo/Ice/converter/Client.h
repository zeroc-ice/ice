// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DEMO_CLIENT_H
#define DEMO_CLIENT_H

#include <Ice/Ice.h>

namespace Demo
{

class Client : public Ice::Application
{
    virtual int run(int, char*[]);
};

}

#endif
