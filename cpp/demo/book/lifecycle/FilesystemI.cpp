// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
FilesystemI::NodeI::name(const Current&)
{
    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
    }

    return _name;
}

// Return the object identity for this node.

Identity
FilesystemI::NodeI::id() const
{
    return _id;
}

FilesystemI::NodeI::NodeI(const string& name, const DirectoryIPtr& parent)
    : _name(name), _parent(parent), _destroyed(false)
{
}

// Slice File::read() operation.

Lines
FilesystemI::FileI::read(const Current&)
{
    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
    }

    return _lines;
}

// Slice File::write() operation.

void
FilesystemI::FileI::write(const Lines& text, const Current&)
{
    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
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
            throw ObjectNotExistException(__FILE__, __LINE__);
        }
        _destroyed = true;
    }

    IceUtil::StaticMutex::Lock lock(DirectoryI::_lcMutex);

    c.adapter->remove(id());
    _parent->addReapEntry(_name);
}

// FileI constructor.

FilesystemI::FileI::FileI(const ObjectAdapterPtr& a, const string& name, const DirectoryIPtr& parent)
    : NodeI(name, parent)
{
    _id.name = IceUtil::generateUUID();
    parent->addChild(name, this);
    a->add(this, _id);
}

// Slice Directory::list() operation.

NodeDescSeq
FilesystemI::DirectoryI::list(const Current& c)
{
    {
        IceUtil::Mutex::Lock lock(_m);

        if(_destroyed)
        {
            throw ObjectNotExistException(__FILE__, __LINE__);
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
            throw ObjectNotExistException(__FILE__, __LINE__);
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
            throw ObjectNotExistException(__FILE__, __LINE__);
        }
    }

    IceUtil::StaticMutex::Lock lock(_lcMutex);

    reap();

    if(_contents.find(name) != _contents.end())
    {
        throw NameInUse(name);
    }

    FileIPtr f = new FileI(c.adapter, name, this);
    return FilePrx::uncheckedCast(c.adapter->createProxy(f->id()));
}

// Slice Directory::createDirectory() operation.

DirectoryPrx
FilesystemI::DirectoryI::createDirectory(const string& name, const Current& c)
{
    {
        IceUtil::Mutex::Lock lock(_m);

        if(_destroyed)
        {
            throw ObjectNotExistException(__FILE__, __LINE__);
        }
    }

    IceUtil::StaticMutex::Lock lock(_lcMutex);

    reap();

    if(_contents.find(name) != _contents.end())
    {
        throw NameInUse(name);
    }

    DirectoryIPtr d = new DirectoryI(c.adapter, name, this);
    return DirectoryPrx::uncheckedCast(c.adapter->createProxy(d->id()));
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
        throw ObjectNotExistException(__FILE__, __LINE__);
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

FilesystemI::DirectoryI::DirectoryI(const ObjectAdapterPtr& a, const string& name, const DirectoryIPtr& parent)
    : NodeI(name, parent)
{
    if(!parent)
    {
        _id.name = "RootDir";
    }
    else
    {
        _id.name = IceUtil::generateUUID();
        _parent->addChild(name, this);
    }
    a->add(this, _id);
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
