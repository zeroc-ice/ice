// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

using namespace std;

void
MyObjectI::ice_ping(const Ice::Current& current) const
{
    string name = current.id.name;

    if (name == "ObjectNotExist")
    {
        throw Ice::ObjectNotExistException{__FILE__, __LINE__};
    }
    else if (name == "FacetNotExist")
    {
        throw Ice::FacetNotExistException{__FILE__, __LINE__};
    }
}

string
MyObjectI::getName(const Ice::Current& current)
{
    string name = current.id.name;

    if (name == "ObjectNotExist")
    {
        throw Ice::ObjectNotExistException{__FILE__, __LINE__};
    }
    else if (name == "FacetNotExist")
    {
        throw Ice::FacetNotExistException{__FILE__, __LINE__};
    }

    return name;
}
