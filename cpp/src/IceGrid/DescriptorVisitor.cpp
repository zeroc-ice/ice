// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/Admin.h>
#include <IceGrid/DescriptorVisitor.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

ApplicationWrapper::ApplicationWrapper(const ApplicationDescriptorPtr& descriptor) : _descriptor(descriptor)
{
}

void
ApplicationWrapper::visit(DescriptorVisitor& visitor)
{
    if(visitor.visitApplicationStart(*this, _descriptor))
    {
	for(ServerDescriptorSeq::const_iterator p = _descriptor->servers.begin(); p != _descriptor->servers.end(); ++p)
	{
	    ServerWrapper(*p).visit(visitor);
	}
	visitor.visitApplicationEnd(*this, _descriptor);
    }
}

const ApplicationDescriptorPtr&
ApplicationWrapper::getDescriptor() const
{
    return _descriptor;
}

ComponentWrapper::ComponentWrapper(const ComponentDescriptorPtr& descriptor) : _descriptor(descriptor)
{
}

void
ComponentWrapper::visit(DescriptorVisitor& visitor)
{
    for(AdapterDescriptorSeq::const_iterator p = _descriptor->adapters.begin(); p != _descriptor->adapters.end(); ++p)
    {
	AdapterWrapper(*this, *p).visit(visitor);
    }

    for(DbEnvDescriptorSeq::const_iterator q = _descriptor->dbEnvs.begin(); q != _descriptor->dbEnvs.end(); ++q)
    {
	DbEnvWrapper(*this, *q).visit(visitor);
    }
}

ServerWrapper::ServerWrapper(const ServerDescriptorPtr& descriptor) :
    ComponentWrapper(descriptor),
    _descriptor(descriptor)
{
}

void
ServerWrapper::visit(DescriptorVisitor& visitor)
{
    if(visitor.visitServerStart(*this, _descriptor))
    {
	ComponentWrapper::visit(visitor);

	CppIceBoxDescriptorPtr cppIceBox = CppIceBoxDescriptorPtr::dynamicCast(_descriptor);
	JavaIceBoxDescriptorPtr javaIceBox = JavaIceBoxDescriptorPtr::dynamicCast(_descriptor);
	
	const ServiceDescriptorSeq& services = 
	    cppIceBox ? cppIceBox->services : (javaIceBox ? javaIceBox->services : ServiceDescriptorSeq());

	for(ServiceDescriptorSeq::const_iterator p = services.begin(); p != services.end(); ++p)
	{
	    ServiceWrapper(*this, *p).visit(visitor);
	}	    
	visitor.visitServerEnd(*this, _descriptor);
    }
}

const ServerDescriptorPtr&
ServerWrapper::getDescriptor() const
{
    return _descriptor;
}

ServiceWrapper::ServiceWrapper(const ServerWrapper& server, const ServiceDescriptorPtr& descriptor) :
    ComponentWrapper(descriptor),
    _server(server),
    _descriptor(descriptor)
{
}

void
ServiceWrapper::visit(DescriptorVisitor& visitor)
{
    if(visitor.visitServiceStart(*this, _descriptor))
    {
	ComponentWrapper::visit(visitor);
	visitor.visitServiceEnd(*this, _descriptor);
    }
}

const ServiceDescriptorPtr&
ServiceWrapper::getDescriptor() const
{
    return _descriptor;
}

DbEnvWrapper::DbEnvWrapper(const ComponentWrapper& component, const DbEnvDescriptor& descriptor) :
    _component(component),
    _descriptor(descriptor)
{
}

void
DbEnvWrapper::visit(DescriptorVisitor& visitor)
{
    visitor.visitDbEnv(*this, _descriptor);
}

const DbEnvDescriptor&
DbEnvWrapper::getDescriptor() const
{
    return _descriptor;
}

AdapterWrapper::AdapterWrapper(const ComponentWrapper& component, const AdapterDescriptor& descriptor) :
    _component(component),
    _descriptor(descriptor)
{
}

void
AdapterWrapper::visit(DescriptorVisitor& visitor)
{
    if(visitor.visitAdapterStart(*this, _descriptor))
    {
	for(ObjectDescriptorSeq::const_iterator p = _descriptor.objects.begin(); p != _descriptor.objects.end(); ++p)
	{
	    ObjectWrapper(*this, *p).visit(visitor);
	}
	visitor.visitAdapterEnd(*this, _descriptor);
    }
}

const AdapterDescriptor&
AdapterWrapper::getDescriptor() const
{
    return _descriptor;
}

ObjectWrapper::ObjectWrapper(const AdapterWrapper& adapter, const ObjectDescriptor& descriptor): 
    _adapter(adapter),
    _descriptor(descriptor)
{
}

void
ObjectWrapper::visit(DescriptorVisitor& visitor)
{
    visitor.visitObject(*this, _descriptor);
}

const ObjectDescriptor&
ObjectWrapper::getDescriptor() const
{
    return _descriptor;
}


