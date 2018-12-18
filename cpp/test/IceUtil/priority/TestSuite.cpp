// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/MutexProtocol.h>

#include <TestSuite.h>
#include <ThreadPriority.h>
#include <TimerPriority.h>
#include <PriorityInversion.h>

std::list<TestBasePtr> allTests;

void
initializeTestSuite()
{
    allTests.push_back(new ThreadPriorityTest);
    allTests.push_back(new TimerPriorityTest);
    if(IceUtil::getDefaultMutexProtocol() == IceUtil::PrioInherit)
    {
        allTests.push_back(new PriorityInversionTest);
    }
}
