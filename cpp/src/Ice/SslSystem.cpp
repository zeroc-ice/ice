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

void ::IceInternal::incRef(::IceSecurity::Ssl::System* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSecurity::Ssl::System* p) { p->__decRef(); }

//
// Protected Methods
//

IceSecurity::Ssl::System::System()
{
}

IceSecurity::Ssl::System::~System()
{
}
