//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
