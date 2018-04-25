// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <TestBase.h>
#include <list>

extern std::list<TestBasePtr> allTests;

void initializeTestSuite();

#endif
