// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/Util.h>
#include <IceStorm/LLUMap.h>

using namespace Freeze;
using namespace IceStormElection;
using namespace IceStorm;
using namespace std;

string
IceStormInternal::describeEndpoints(const Ice::ObjectPrx& proxy)
{
    ostringstream os;
    if(proxy)
    {
        Ice::EndpointSeq endpoints = proxy->ice_getEndpoints();
        for(Ice::EndpointSeq::const_iterator i = endpoints.begin(); i != endpoints.end(); ++i)
        {
            if(i != endpoints.begin())
            {
                os << ", "; 
            }
            os << "\"" << (*i)->toString() << "\"";
        }
    }
    else
    {
        os << "subscriber proxy is null";
    }
    return os.str();
}

namespace
{

const string lluDbName = "llu";

}

void
IceStormInternal::putLLU(const ConnectionPtr& connection, const LogUpdate& llu)
{
    LLUMap llumap(connection, lluDbName);
    LLUMap::iterator ci = llumap.find("_manager");
    if(ci == llumap.end())
    {
        llumap.put(LLUMap::value_type("_manager", llu));
    }
    else
    {
        ci.set(llu);
    }
}

LogUpdate
IceStormInternal::getLLU(const ConnectionPtr& connection)
{
    LLUMap llumap(connection, lluDbName);
    LLUMap::iterator ci = llumap.find("_manager");
    assert(ci != llumap.end());
    return ci->second;
}