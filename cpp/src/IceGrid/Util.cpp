// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePatch2/Util.h>
#include <IceGrid/Util.h>
#include <IceGrid/Admin.h>

#include <fstream>

using namespace std;
using namespace IceGrid;

string 
IceGrid::toString(const vector<string>& v, const string& sep)
{
    ostringstream os;
    Ice::StringSeq::const_iterator p = v.begin();
    while(p != v.end())
    {
	os << *p;
	++p;
	if(p != v.end())
	{
	    os << sep;
	}
    }
    return os.str();
}

string
IceGrid::getProperty(const PropertyDescriptorSeq& properties, const string& name, const string& def)
{
    for(PropertyDescriptorSeq::const_iterator p = properties.begin(); p != properties.end(); ++p)
    {
	if(p->name == name)
	{
	    return p->value;
	}
    }
    return def;
}

string
IceGrid::getReplicaId(const AdapterDescriptor& adapter, const CommunicatorDescriptorPtr& comm, const string& serverId)
{
    if(!adapter.replicaId.empty())
    {
	return adapter.replicaId;
    }

    //
    // Compute the default replica id of an object adapter: if the
    // adapter belongs to a service the replica id will be "<server
    // id>.<service name>", if the adapter belongs to a server its
    // replica id will be "<server id>".
    //
    ServiceDescriptorPtr service = ServiceDescriptorPtr::dynamicCast(comm);
    return service ? serverId + "." + service->name : serverId;
}
