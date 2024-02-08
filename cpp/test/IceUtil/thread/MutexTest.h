//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef MUTEX_TEST_H
#define MUTEX_TEST_H

#include <TestBase.h>

class MutexTest final : public TestBase
{
public:

    MutexTest();

private:

    void run() final;
};

#endif
