// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/Util.h>

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
