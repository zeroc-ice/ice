// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/DefaultsAndOverwrites.h>
#include <Ice/Properties.h>
#include <Ice/Network.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(DefaultsAndOverwrites* p) { p->__incRef(); }
void IceInternal::decRef(DefaultsAndOverwrites* p) { p->__decRef(); }

IceInternal::DefaultsAndOverwrites::DefaultsAndOverwrites(const PropertiesPtr& properties) :
    overwriteTimeout(false),
    overwriteTimeoutValue(-1),
    overwriteCompress(false),
    overwriteCompressValue(false)
{
    const_cast<string&>(defaultProtocol) = properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");

    const_cast<string&>(defaultHost) = properties->getProperty("Ice.Default.Host");
    if (defaultHost.empty())
    {
	const_cast<string&>(defaultHost) = getLocalHost(true);
    }

    const_cast<string&>(defaultRouter) = properties->getProperty("Ice.Default.Router");

    string value;
    
    value = properties->getProperty("Ice.Overwrite.Timeout");
    if (!value.empty())
    {
	const_cast<bool&>(overwriteTimeout) = true;
	const_cast<Int&>(overwriteTimeoutValue) = atoi(value.c_str());
    }

    value = properties->getProperty("Ice.Overwrite.Compress");
    if (!value.empty())
    {
	const_cast<bool&>(overwriteCompress) = true;
	const_cast<bool&>(overwriteCompressValue) = atoi(value.c_str());
    }
}
