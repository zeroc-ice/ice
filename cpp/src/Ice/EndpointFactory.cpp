// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/EndpointFactory.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(EndpointFactory* p) { p->__incRef(); }
void IceInternal::decRef(EndpointFactory* p) { p->__decRef(); }

IceInternal::EndpointFactory::EndpointFactory()
{
}

IceInternal::EndpointFactory::~EndpointFactory()
{
}
