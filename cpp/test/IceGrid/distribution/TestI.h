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

    TestI(const Ice::PropertiesPtr&);

    virtual std::string getServerFile(const std::string&, const Ice::Current&);
    virtual std::string getApplicationFile(const std::string&, const Ice::Current&);

private:

    Ice::PropertiesPtr _properties;
};

#endif
