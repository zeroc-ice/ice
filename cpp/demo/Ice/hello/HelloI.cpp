// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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
HelloI::sayHello(const Ice::Current&)
{
    cout << "Hello World!" << endl;
}

void
HelloI::shutdown(const Ice::Current&)
{
    cout << "Shutting down..." << endl;
    _communicator->shutdown();
}
