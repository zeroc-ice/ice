// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
Freeze::EvictorPtr Filesystem::NodeI::_evictor;

Filesystem::NodeI::NodeI()
  : _destroyed(false), _id(Ice::Identity())
{
}

Filesystem::NodeI::NodeI(const Ice::Identity& id)
    : _destroyed(false), _id(id)
{
}

//
// Filesystem::FileI
//
string
Filesystem::FileI::name(const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    return nodeName;
}

void
Filesystem::FileI::destroy(const Ice::Current& c)
{
    {
        IceUtil::Mutex::Lock lock(*this);

	if(_destroyed)
	{
	    throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
	}
	_destroyed = true;
    }

    parent->removeNode(nodeName);
    _evictor->remove(_id);
}

Filesystem::Lines
Filesystem::FileI::read(const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    return text;
}

void
Filesystem::FileI::write(const Filesystem::Lines& text, const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

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
Filesystem::DirectoryI::name(const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    return nodeName;
}

void
Filesystem::DirectoryI::destroy(const Ice::Current& c)
{
    if(!parent)
    {
        throw Filesystem::PermissionDenied("Cannot destroy root directory");
    }

    {
        IceUtil::Mutex::Lock lock(*this);

        if(_destroyed)
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
        }
        if(!nodes.empty())
        {
            throw Filesystem::PermissionDenied("Cannot destroy non-empty directory");
        }
        _destroyed = true;
    }

    parent->removeNode(nodeName);
    _evictor->remove(_id);
}

Filesystem::NodeDescSeq
Filesystem::DirectoryI::list(const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    NodeDict::const_iterator p;
    NodeDescSeq result;
    for(p = nodes.begin(); p != nodes.end(); ++p)
    {
        result.push_back(p->second);
    }
    return result;
}

Filesystem::NodeDesc
Filesystem::DirectoryI::find(const string& name, const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    NodeDict::iterator p = nodes.find(name);
    if(p == nodes.end())
    {
        throw NoSuchName(name);
    }
    return p->second;
}

Filesystem::DirectoryPrx
Filesystem::DirectoryI::createDirectory(const string& name, const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    if(name.empty() || nodes.find(name) != nodes.end())
    {
        throw NameInUse(name);
    }

    Ice::Identity id;
    id.name = IceUtil::generateUUID();
    PersistentDirectoryPtr dir = new DirectoryI(id);
    dir->nodeName = name;
    dir->parent = PersistentDirectoryPrx::uncheckedCast(c.adapter->createProxy(c.id));
    DirectoryPrx proxy = DirectoryPrx::uncheckedCast(_evictor->add(dir, id));

    NodeDesc nd;
    nd.name = name;
    nd.type = DirType;
    nd.proxy = proxy;
    nodes[name] = nd;

    return proxy;
}

Filesystem::FilePrx
Filesystem::DirectoryI::createFile(const string& name, const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    if(name.empty() || nodes.find(name) != nodes.end())
    {
        throw NameInUse(name);
    }

    Ice::Identity id;
    id.name = IceUtil::generateUUID();
    PersistentFilePtr file = new FileI(id);
    file->nodeName = name;
    file->parent = PersistentDirectoryPrx::uncheckedCast(c.adapter->createProxy(c.id));
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

Filesystem::DirectoryI::DirectoryI()
{
}

Filesystem::DirectoryI::DirectoryI(const Ice::Identity& id) :
    NodeI(id)
{
}

//
// Filesystem::NodeFactory
//
Ice::ObjectPtr
Filesystem::NodeFactory::create(const string& type)
{
    if(type == PersistentFile::ice_staticId())
    {
        return new FileI;
    }
    else if(type == PersistentDirectory::ice_staticId())
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
    const_cast<Ice::Identity&>(node->_id) = id;
}
