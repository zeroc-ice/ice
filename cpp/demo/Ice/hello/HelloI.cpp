// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
HelloI::hello()
{
    cout << "Hello World!" << endl;
}

void
HelloI::shutdown()
{
    cout << "Shutting down..." << endl;
    _communicator->shutdown();
}
