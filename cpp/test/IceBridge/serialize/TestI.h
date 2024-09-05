//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class MyClassI final : public Test::MyClass
{
public:
    void incCounter(int, const Ice::Current&) final;
    void waitCounter(int, const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;

private:
    std::mutex _mutex;
    std::condition_variable _condVar;
    int _counter = 0;
};

#endif
