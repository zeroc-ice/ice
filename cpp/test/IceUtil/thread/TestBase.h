// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef TEST_BASE_H
#define TEST_BASE_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>

class TestFailed
{
public:

    TestFailed(const std::string&);

    const std::string name;
};

class TestBase : public IceUtil::Shared
{
public:

    TestBase(const std::string&);

    std::string name() const;
    void start();

protected:

    virtual void run() = 0;

    const std::string _name;
};

typedef IceUtil::Handle<TestBase> TestBasePtr;

#endif
