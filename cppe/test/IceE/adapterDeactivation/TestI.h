// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI : public Test::TestIntf
{
public:

    virtual void transient(const IceE::Current&);
    virtual void deactivate(const IceE::Current&);
};

class CookieI : public Test::Cookie
{
public:

    virtual std::string message() const;
};

#endif
