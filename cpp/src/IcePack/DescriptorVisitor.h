// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPACK_DESCRIPTOR_VISITOR_H
#define ICEPACK_DESCRIPTOR_VISITOR_H

namespace IcePack
{

class ApplicationWrapper;
class ServerWrapper;
class ServiceWrapper;
class AdapterWrapper;
class ObjectWrapper;
class DbEnvWrapper;

class DescriptorVisitor
{
public:
    
    virtual ~DescriptorVisitor() { }
    
    virtual bool visitApplicationStart(const ApplicationWrapper&, const ApplicationDescriptorPtr&) { return true; }
    virtual void visitApplicationEnd(const ApplicationWrapper&, const ApplicationDescriptorPtr&) { }
    virtual bool visitServerStart(const ServerWrapper&, const ServerDescriptorPtr&) { return true; }
    virtual void visitServerEnd(const ServerWrapper&, const ServerDescriptorPtr&) { }
    virtual bool visitServiceStart(const ServiceWrapper&, const ServiceDescriptorPtr&) { return true; }
    virtual void visitServiceEnd(const ServiceWrapper&, const ServiceDescriptorPtr&) { }
    virtual bool visitAdapterStart(const AdapterWrapper&, const AdapterDescriptor&) { return true; }
    virtual void visitAdapterEnd(const AdapterWrapper&, const AdapterDescriptor&) { }
    virtual void visitDbEnv(const DbEnvWrapper&, const DbEnvDescriptor&) { }
    virtual void visitObject(const ObjectWrapper&, const ObjectDescriptor&) { }
};

class ApplicationWrapper
{
public:

    ApplicationWrapper(const ApplicationDescriptorPtr&);
    
    void visit(DescriptorVisitor&);
    const ApplicationDescriptorPtr& getDescriptor() const;

private:
    
    const ApplicationDescriptorPtr _descriptor;
};

class ComponentWrapper
{
public:

    ComponentWrapper(const ComponentDescriptorPtr&);

    void visit(DescriptorVisitor&);

private:

    ComponentDescriptorPtr _descriptor;

};

class ServerWrapper : public ComponentWrapper
{
public:

    ServerWrapper(const ServerDescriptorPtr&);

    void visit(DescriptorVisitor&);
    const ServerDescriptorPtr& getDescriptor() const;

private:

    ServerDescriptorPtr _descriptor;
};

class ServiceWrapper : public ComponentWrapper
{
public:

    ServiceWrapper(const ServerWrapper&, const ServiceDescriptorPtr&);

    void visit(DescriptorVisitor&);
    const ServiceDescriptorPtr& getDescriptor() const;

private:

    const ServerWrapper& _server;
    ServiceDescriptorPtr _descriptor;
};

class DbEnvWrapper
{
public:
    
    DbEnvWrapper(const ComponentWrapper&, const DbEnvDescriptor&);
    
    void visit(DescriptorVisitor&);
    const DbEnvDescriptor& getDescriptor() const;

private:
    
    const ComponentWrapper& _component;
    const DbEnvDescriptor& _descriptor;    
};

class AdapterWrapper
{
public:
    
    AdapterWrapper(const ComponentWrapper&, const AdapterDescriptor&);
    
    void visit(DescriptorVisitor&);
    const AdapterDescriptor& getDescriptor() const;

private:
    
    const ComponentWrapper& _component;
    const AdapterDescriptor& _descriptor;    
};

class ObjectWrapper
{
public:
    
    ObjectWrapper(const AdapterWrapper&, const ObjectDescriptor&);
    
    void visit(DescriptorVisitor&);
    const ObjectDescriptor& getDescriptor() const;

private:
    
    const AdapterWrapper& _adapter;
    const ObjectDescriptor& _descriptor;    
};

}

#endif


