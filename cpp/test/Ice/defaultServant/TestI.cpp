// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;

void
MyObjectI::ice_ping(const Ice::Current& current) const
{
    string name = current.id.name;

    if(name == "ObjectNotExist")
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    else if(name == "FacetNotExist")
    {
        throw Ice::FacetNotExistException(__FILE__, __LINE__);
    }
}

std::string
MyObjectI::getName(const Ice::Current& current)
{
    string name = current.id.name;

    if(name == "ObjectNotExist")
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    else if(name == "FacetNotExist")
    {
        throw Ice::FacetNotExistException(__FILE__, __LINE__);
    }

    return name;
}
