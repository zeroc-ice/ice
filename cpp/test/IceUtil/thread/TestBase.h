// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_BASE_H
#define TEST_BASE_H

//#include <string>

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>

class TestFailed
{
public:

    TestFailed(const std::string& n) : name(n) { }

    std::string name;
};

class TestBase : public IceUtil::Shared
{
public:

    TestBase(const std::string& n) :
	_name(n)
    {
    }
    
    virtual ~TestBase()
    {
    }

    const std::string& name() const { return _name; }

    void start();

protected:

    virtual void run() = 0;
    
    std::string _name;
};

typedef IceUtil::Handle<TestBase> TestBasePtr;

#endif
