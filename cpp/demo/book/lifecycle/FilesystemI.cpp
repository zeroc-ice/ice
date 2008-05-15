// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <FilesystemI.h>

using namespace std;
using namespace Ice;
using namespace Filesystem;
using namespace FilesystemI;

IceUtil::StaticMutex FilesystemI::DirectoryI::_lcMutex = ICE_STATIC_MUTEX_INITIALIZER;
FilesystemI::DirectoryI::ReapMap FilesystemI::DirectoryI::_reapMap;

// Slice Node::name() operation.

std::string
FilesystemI::NodeI::name(const Current& c)
{
    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
        throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    return _name;
}

// Return the object identity for this node.

Identity
FilesystemI::NodeI::id() const
{
    return _id;
}

// Activate the servant and add it to the parent's contents map.

ObjectPrx
FilesystemI::NodeI::activate(const ObjectAdapterPtr& a)
{
    ObjectPrx node = a->add(this, _id);
    if(_parent)
    {
        _parent->addChild(_name, this);
    }
    return node;
}

// NodeI constructor.

FilesystemI::NodeI::NodeI(const string& name, const DirectoryIPtr& parent)
    : _name(name), _parent(parent), _destroyed(false)
{
    // Create an identity. The root directory has the fixed identity "RootDir".
    //
    if(parent != 0)
    {
        _id.name = IceUtil::generateUUID();
    }
    else
    {
        _id.name = "RootDir";
    }
}

// Slice File::read() operation.

Lines
FilesystemI::FileI::read(const Current& c)
{
    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
        throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    return _lines;
}

// Slice File::write() operation.

void
FilesystemI::FileI::write(const Lines& text, const Current& c)
{
    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
        throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    _lines = text;
}

// Slice File::destroy() operation.

void
FilesystemI::FileI::destroy(const Current& c)
{
    {
        IceUtil::Mutex::Lock lock(_m);

        if(_destroyed)
        {
            throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
        }
        _destroyed = true;
    }

    IceUtil::StaticMutex::Lock lock(DirectoryI::_lcMutex);

    c.adapter->remove(id());
    _parent->addReapEntry(_name);
}

// FileI constructor.

FilesystemI::FileI::FileI(const string& name, const DirectoryIPtr& parent)
    : NodeI(name, parent)
{
}

// Slice Directory::list() operation.

NodeDescSeq
FilesystemI::DirectoryI::list(const Current& c)
{
    {
        IceUtil::Mutex::Lock lock(_m);

        if(_destroyed)
        {
            throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
        }
    }

    IceUtil::StaticMutex::Lock lock(_lcMutex);

    reap();

    NodeDescSeq ret;
    for(Contents::const_iterator i = _contents.begin(); i != _contents.end(); ++i)
    {
        NodeDesc d;
        d.name = i->first;
        d.type = FilePtr::dynamicCast(i->second) ? FileType : DirType;
        d.proxy = NodePrx::uncheckedCast(c.adapter->createProxy(i->second->id()));
        ret.push_back(d);
    }
    return ret;
}

// Slice Directory::find() operation.

NodeDesc
FilesystemI::DirectoryI::find(const string& name, const Current& c)
{
    {
        IceUtil::Mutex::Lock lock(_m);

        if(_destroyed)
        {
            throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
        }
    }

    IceUtil::StaticMutex::Lock lock(_lcMutex);

    reap();

    Contents::const_iterator pos = _contents.find(name);
    if(pos == _contents.end())
    {
        throw NoSuchName(name);
    }

    NodeIPtr p = pos->second;
    NodeDesc d;
    d.name = name;
    d.type = FilePtr::dynamicCast(p) ? FileType : DirType;
    d.proxy = NodePrx::uncheckedCast(c.adapter->createProxy(p->id()));
    return d;
}

// Slice Directory::createFile() operation.

FilePrx
FilesystemI::DirectoryI::createFile(const string& name, const Current& c)
{
    {
        IceUtil::Mutex::Lock lock(_m);

        if(_destroyed)
        {
            throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
        }
    }

    IceUtil::StaticMutex::Lock lock(_lcMutex);

    reap();

    if(name.empty() || _contents.find(name) != _contents.end())
    {
        throw NameInUse(name);
    }

    FileIPtr f = new FileI(name, this);
    return FilePrx::uncheckedCast(f->activate(c.adapter));
}

// Slice Directory::createDirectory() operation.

DirectoryPrx
FilesystemI::DirectoryI::createDirectory(const string& name, const Current& c)
{
    {
        IceUtil::Mutex::Lock lock(_m);

        if(_destroyed)
        {
            throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
        }
    }

    IceUtil::StaticMutex::Lock lock(_lcMutex);

    reap();

    if(name.empty() || _contents.find(name) != _contents.end())
    {
        throw NameInUse(name);
    }

    DirectoryIPtr d = new DirectoryI(name, this);
    return DirectoryPrx::uncheckedCast(d->activate(c.adapter));
}

// Slice Directory::destroy() operation.

void
FilesystemI::DirectoryI::destroy(const Current& c)
{

    if(!_parent)
    {
        throw PermissionDenied("Cannot destroy root directory");
    }

    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
        throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    IceUtil::StaticMutex::Lock lcLock(_lcMutex);

    reap();

    if(!_contents.empty())
    {
        throw PermissionDenied("Cannot destroy non-empty directory");
    }

    c.adapter->remove(id());
    _parent->addReapEntry(_name);
    _destroyed = true;
}

// DirectoryI constructor.

FilesystemI::DirectoryI::DirectoryI(const string& name, const DirectoryIPtr& parent)
    : NodeI(name, parent)
{
}

// Add the passed name-node pair to the _contents map.

void
FilesystemI::DirectoryI::addChild(const string& name, const NodeIPtr& node)
{
    _contents[name] = node;
}


// Add this directory and the name of a deleted entry to the reap map.

void
FilesystemI::DirectoryI::addReapEntry(const string& name)
{
    ReapMap::iterator pos = _reapMap.find(this);
    if(pos != _reapMap.end())
    {
        pos->second.push_back(name);
    }
    else
    {
        vector<string> v;
        v.push_back(name);
        _reapMap[this] = v;
    }
}

// Remove all names in the reap map from the corresponding directory contents.

void
FilesystemI::DirectoryI::reap()
{
   for(ReapMap::const_iterator i = _reapMap.begin(); i != _reapMap.end(); ++i)
   {
        for(vector<string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
        {
            i->first->_contents.erase(*j);
        }
   }
   _reapMap.clear();
}
