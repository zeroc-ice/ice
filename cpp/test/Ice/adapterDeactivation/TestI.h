// **********************************************************************
//
// Copyright (c) 2002
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

    virtual void deactivate(const Ice::Current&);
};

class CookieI : public Cookie
{
public:

    virtual std::string message() const;
};

#endif
