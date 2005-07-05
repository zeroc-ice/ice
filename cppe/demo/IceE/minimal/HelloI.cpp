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
HelloI::sayHello(const IceE::Current&) const
{
    printf("Hello World!\n");
}
