// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/TraceLevels.h>
#include <Ice/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(TraceLevels* p) { p->__incRef(); }
void IceInternal::decRef(TraceLevels* p) { p->__decRef(); }

IceInternal::TraceLevels::TraceLevels(const PropertiesPtr& properties) :
    network(0),
    networkCat("Network"),
    protocol(0),
    protocolCat("Protocol"),
    retry(0),
    retryCat("Retry")
{
    string value;
    const string keyBase = "Ice.Trace.";

    value = properties->getProperty(keyBase + networkCat);
    if (!value.empty())
    {
	const_cast<int&>(network) = atoi(value.c_str());
    }
    
    value = properties->getProperty(keyBase + protocolCat);
    if (!value.empty())
    {
	const_cast<int&>(protocol) = atoi(value.c_str());
    }

    value = properties->getProperty(keyBase + retryCat);
    if (!value.empty())
    {
	const_cast<int&>(retry) = atoi(value.c_str());
    }
}

IceInternal::TraceLevels::~TraceLevels()
{
}
