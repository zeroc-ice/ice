// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
