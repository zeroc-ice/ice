// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Test.h>
#include <iostream>

using namespace IceUtil; 
using namespace std;

void
Test::RetryException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nretry dispatch";
}
