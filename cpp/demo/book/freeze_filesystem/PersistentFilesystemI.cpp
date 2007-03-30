// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <PersistentFilesystemI.h>

using namespace std;

//
// Filesystem::NodeI
//
Ice::ObjectAdapterPtr Filesystem::NodeI::_adapter;
Freeze::EvictorPtr Filesystem::NodeI::_evictor;

Filesystem::NodeI::NodeI()
{
}

Filesystem::NodeI::NodeI(const Ice::Identity& id)
    : _ID(id)
{
}

//
// Filesystem::FileI
//
string
Filesystem::FileI::name(const Ice::Current&)
{
    return nodeName;
}

void
Filesystem::FileI::destroy(const Ice::Current&)
{
    parent->removeNode(nodeName);
    _evictor->remove(_ID);
}

Filesystem::Lines
Filesystem::FileI::read(const Ice::Current&)
{
    IceUtil::Mutex::Lock lock(*this);
    return text;
}

void
Filesystem::FileI::write(const Filesystem::Lines& text, const Ice::Current&)
{
    IceUtil::Mutex::Lock lock(*this);
    this->text = text;
}

Filesystem::FileI::FileI()
{
}

Filesystem::FileI::FileI(const Ice::Identity& id)
    : NodeI(id)
{
}

//
// Filesystem::DirectoryI
//
string
Filesystem::DirectoryI::name(const Ice::Current& current)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    return nodeName;
}

void
Filesystem::DirectoryI::destroy(const Ice::Current& current)
{
    if(!parent)
    {
        throw Filesystem::PermissionDenied("cannot remove root directory");
    }

    NodeDict children;

    {
        IceUtil::Mutex::Lock lock(*this);

        if(_destroyed)
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
        }

        children = nodes;
        _destroyed = true;
    }

    //
    // We must iterate over the children outside of synchronization.
    //
    for(NodeDict::iterator p = children.begin(); p != children.end(); ++p)
    {
        p->second.proxy->destroy();
    }

    assert(nodes.empty());

    parent->removeNode(nodeName);
    _evictor->remove(_ID);
}

Filesystem::NodeDict
Filesystem::DirectoryI::list(Filesystem::ListMode mode, const Ice::Current& current)
{
    NodeDict result;
    {
        IceUtil::Mutex::Lock lock(*this);

        if(_destroyed)
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
        }

        result = nodes;
    }

    if(mode == RecursiveList)
    {
        for(NodeDict::iterator p = result.begin(); p != result.end(); ++p)
        {
            if(p->second.type == DirType)
            {
                DirectoryPrx dir = DirectoryPrx::uncheckedCast(p->second.proxy);
                NodeDict d = dir->list(mode);
                for(NodeDict::iterator q = d.begin(); q != d.end(); ++q)
                {
                    result[p->second.name + "/" + q->second.name] = q->second;
                }
            }
        }
    }

    return result;
}

Filesystem::NodeDesc
Filesystem::DirectoryI::resolve(const string& path, const Ice::Current& current)
{
    string::size_type pos = path.find('/');
    string child, remainder;
    if(pos == string::npos)
    {
        child = path;
    }
    else
    {
        child = path.substr(0, pos);
        pos = path.find_first_not_of("/", pos);
        if(pos != string::npos)
        {
            remainder = path.substr(pos);
        }
    }

    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    NodeDict::iterator p = nodes.find(child);
    if(p == nodes.end())
    {
        throw NoSuchName("no node exists with name `" + child + "'");
    }

    if(remainder.empty())
    {
        return p->second;
    }
    else
    {
        if(p->second.type != DirType)
        {
            throw NoSuchName("node `" + child + "' is not a directory");
        }
        DirectoryPrx dir = DirectoryPrx::checkedCast(p->second.proxy);
        assert(dir);
        return dir->resolve(remainder);
    }
}

Filesystem::DirectoryPrx
Filesystem::DirectoryI::createDirectory(const string& name, const Ice::Current& current)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    checkName(name);

    Ice::Identity id = current.adapter->getCommunicator()->stringToIdentity(IceUtil::generateUUID());
    PersistentDirectoryPtr dir = new DirectoryI(id);
    dir->nodeName = name;
    dir->parent = PersistentDirectoryPrx::uncheckedCast(current.adapter->createProxy(current.id));
    DirectoryPrx proxy = DirectoryPrx::uncheckedCast(_evictor->add(dir, id));

    NodeDesc nd;
    nd.name = name;
    nd.type = DirType;
    nd.proxy = proxy;
    nodes[name] = nd;

    return proxy;
}

Filesystem::FilePrx
Filesystem::DirectoryI::createFile(const string& name, const Ice::Current& current)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    checkName(name);

    Ice::Identity id = current.adapter->getCommunicator()->stringToIdentity(IceUtil::generateUUID());
    PersistentFilePtr file = new FileI(id);
    file->nodeName = name;
    file->parent = PersistentDirectoryPrx::uncheckedCast(current.adapter->createProxy(current.id));
    FilePrx proxy = FilePrx::uncheckedCast(_evictor->add(file, id));

    NodeDesc nd;
    nd.name = name;
    nd.type = FileType;
    nd.proxy = proxy;
    nodes[name] = nd;

    return proxy;
}

void
Filesystem::DirectoryI::removeNode(const string& name, const Ice::Current&)
{
    IceUtil::Mutex::Lock lock(*this);

    NodeDict::iterator p = nodes.find(name);
    assert(p != nodes.end());
    nodes.erase(p);
}

Filesystem::DirectoryI::DirectoryI() :
    _destroyed(false)
{
}

Filesystem::DirectoryI::DirectoryI(const Ice::Identity& id) :
    NodeI(id), _destroyed(false)
{
}

void
Filesystem::DirectoryI::checkName(const string& name) const
{
    if(name.empty() || name.find('/') != string::npos)
    {
        IllegalName e;
        e.reason = "illegal name `" + name + "'";
        throw e;
    }

    NodeDict::const_iterator p = nodes.find(name);
    if(p != nodes.end())
    {
        throw NameInUse("name `" + name + "' is already in use");
    }
}

//
// Filesystem::NodeFactory
//
Ice::ObjectPtr
Filesystem::NodeFactory::create(const string& type)
{
    if(type == "::Filesystem::PersistentFile")
    {
        return new FileI;
    }
    else if(type == "::Filesystem::PersistentDirectory")
    {
        return new DirectoryI;
    }
    else
    {
        assert(false);
        return 0;
    }
}

void
Filesystem::NodeFactory::destroy()
{
}

//
// Filesystem::NodeInitializer
//
void
Filesystem::NodeInitializer::initialize(const Ice::ObjectAdapterPtr&,
                                        const Ice::Identity& id,
                                        const string& facet,
                                        const Ice::ObjectPtr& obj)
{
    NodeIPtr node = NodeIPtr::dynamicCast(obj);
    assert(node);
    const_cast<Ice::Identity&>(node->_ID) = id;
}
