// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <string>
#include <Ice/SslSystem.h>

using namespace std;

//
// Protected Methods
//

IceSecurity::Ssl::System::System(string& systemID) :
                         _systemID(systemID),
                         _refCount(0)
{
}

IceSecurity::Ssl::System::~System()
{
}
