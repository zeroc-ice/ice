// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HelloI.h>

using namespace std;

HelloI::HelloI(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
HelloI::sayHello(const Ice::Current&) const
{
    cout << "Hello World!" << endl;
}

void
HelloI::shutdown(const Ice::Current&)
{
    cout << "Shutting down..." << endl;
    _communicator->shutdown();
}
