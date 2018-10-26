// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>

#include <stdlib.h>
#include <TestHelper.h>
#include <TestSuite.h>
#include <IceUtil/MutexProtocol.h>

using namespace std;

#ifndef _WIN32

namespace IceUtil
{

ICE_API MutexProtocol
getDefaultMutexProtocol()
{
#  if defined(_POSIX_THREAD_PRIO_INHERIT) && _POSIX_THREAD_PRIO_INHERIT > 0
    return PrioInherit;
#  else
    return PrioNone;
#  endif
}

}

#endif

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int, char**)
{
    initializeTestSuite();

    for(list<TestBasePtr>::const_iterator p = allTests.begin(); p != allTests.end(); ++p)
    {
        (*p)->start();
    }
}

DEFINE_TEST(Client)
