// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI : public Test
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
