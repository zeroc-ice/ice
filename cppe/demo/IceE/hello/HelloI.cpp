// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <HelloI.h>

using namespace std;

void
HelloI::sayHello(const IceE::Current& c) const
{
#ifdef _WIN32_WCE
    c.adapter->getCommunicator()->getLogger()->trace("", "Hello World!");
#else
    printf("Hello World!\n");
#endif
}

void
HelloI::shutdown(const IceE::Current& c)
{
    printf("Shutting down...\n");
    c.adapter->getCommunicator()->shutdown();
}
