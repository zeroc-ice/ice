// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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

