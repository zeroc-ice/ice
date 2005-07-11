// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/EndpointFactoryManager.h>
#include <Ice/Endpoint.h>
#include <Ice/UnknownEndpoint.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/DefaultsAndOverrides.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(EndpointFactoryManager* p) { p->__incRef(); }
void IceInternal::decRef(EndpointFactoryManager* p) { p->__decRef(); }

IceInternal::EndpointFactoryManager::EndpointFactoryManager(const InstancePtr& instance)
    : _instance(instance)
{
}

void
IceInternal::EndpointFactoryManager::add(const EndpointFactoryPtr& factory)
{
    IceUtil::Mutex::Lock sync(*this); // TODO: Necessary?

    //
    // TODO: Optimize with a map?
    //
    for(vector<EndpointFactoryPtr>::size_type i = 0; i < _factories.size(); i++)
    {
        if(_factories[i]->type() == factory->type())
        {
            assert(false); // TODO: Exception?
        }
    }
    _factories.push_back(factory);
}

EndpointFactoryPtr
IceInternal::EndpointFactoryManager::get(Short type) const
{
    IceUtil::Mutex::Lock sync(*this); // TODO: Necessary?

    //
    // TODO: Optimize with a map?
    //
    for(vector<EndpointFactoryPtr>::size_type i = 0; i < _factories.size(); i++)
    {
        if(_factories[i]->type() == type)
        {
            return _factories[i];
        }
    }
    return 0;
}

EndpointPtr
IceInternal::EndpointFactoryManager::create(const string& str) const
{
    IceUtil::Mutex::Lock sync(*this); // TODO: Necessary?

    const string delim = " \t\n\r";

    string::size_type beg = str.find_first_not_of(delim);
    if(beg == string::npos)
    {
        EndpointParseException ex(__FILE__, __LINE__);
	ex.str = str;
	throw ex;
    }

    string::size_type end = str.find_first_of(delim, beg);
    if(end == string::npos)
    {
        end = str.length();
    }

    string protocol = str.substr(beg, end - beg);

    if(protocol == "default")
    {
        protocol = _instance->defaultsAndOverrides()->defaultProtocol;
    }

    //
    // TODO: Optimize with a map?
    //
    for(vector<EndpointFactoryPtr>::size_type i = 0; i < _factories.size(); i++)
    {
        if(_factories[i]->protocol() == protocol)
        {
            return _factories[i]->create(str.substr(end));
        }
    }

    return 0;
}

EndpointPtr
IceInternal::EndpointFactoryManager::read(BasicStream* s) const
{
    IceUtil::Mutex::Lock sync(*this); // TODO: Necessary?

    Short type;
    s->read(type);

    //
    // TODO: Optimize with a map?
    //
    for(vector<EndpointFactoryPtr>::size_type i = 0; i < _factories.size(); i++)
    {
        if(_factories[i]->type() == type)
        {
            return _factories[i]->read(s);
        }
    }

    return new UnknownEndpoint(type, s);
}

void
IceInternal::EndpointFactoryManager::destroy()
{
    for(vector<EndpointFactoryPtr>::size_type i = 0; i < _factories.size(); i++)
    {
        _factories[i]->destroy();
    }
    _factories.clear();
}
