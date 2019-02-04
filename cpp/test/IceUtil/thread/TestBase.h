//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
