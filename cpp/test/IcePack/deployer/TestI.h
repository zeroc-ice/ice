// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
