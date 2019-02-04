//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <TestBase.h>
#include <list>

extern std::list<TestBasePtr> allTests;

void initializeTestSuite();

#endif
