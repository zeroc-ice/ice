// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

    TestI(const Ice::PropertiesPtr&);

    virtual std::string getServerFile(const std::string&, const Ice::Current&);
    virtual std::string getApplicationFile(const std::string&, const Ice::Current&);

private:

    Ice::PropertiesPtr _properties;
};

#endif
