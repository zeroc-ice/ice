// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_BASE_H
#define TEST_BASE_H

#include <IceE/Shared.h>
#include <IceE/Handle.h>

class TestFailed
{
public:

    TestFailed(const std::string&);

    const std::string name;
};

class TestBase : public Ice::Shared
{
public:

    TestBase(const std::string&);

    std::string name() const;
    void start();

protected:

    virtual void run() = 0;

    const std::string _name;
};

typedef Ice::Handle<TestBase> TestBasePtr;

#endif
