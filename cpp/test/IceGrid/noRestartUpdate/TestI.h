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

    TestI(const Ice::ObjectAdapterPtr&, const Ice::PropertiesPtr&);

    virtual void shutdown(const Ice::Current&);
    virtual std::string getProperty(const std::string&, const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    Ice::PropertiesPtr _properties;
};

#endif
