// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI : public ::Test::TestIntf
{
public:

    TestI(const Ice::StringSeq&);

    virtual std::string getProperty(ICE_IN(std::string), const Ice::Current&);
    virtual Ice::StringSeq getArgs(const Ice::Current&);

private:

    const Ice::StringSeq _args;
};

#endif
