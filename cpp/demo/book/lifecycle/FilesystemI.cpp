// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

// NodeI constructor.

FilesystemI::NodeI::NodeI(const string& name, const DirectoryIPtr& parent)
    : _name(name), _parent(parent), _destroyed(false)
{
    //
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

        c.adapter->remove(id());
        _destroyed = true;
    }

    _parent->removeEntry(_name);
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
    IceUtil::Mutex::Lock lock(_m);
    
    if(_destroyed)
    {
        throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

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
    IceUtil::Mutex::Lock lock(_m);
    
    if(_destroyed)
    {
        throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

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
    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
        throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    if(name.empty() || _contents.find(name) != _contents.end())
    {
        throw NameInUse(name);
    }

    FileIPtr f = new FileI(name, this);
    ObjectPrx node = c.adapter->add(f, f->id());
    _contents[name] = f;
    return FilePrx::uncheckedCast(node);
}

// Slice Directory::createDirectory() operation.

DirectoryPrx
FilesystemI::DirectoryI::createDirectory(const string& name, const Current& c)
{
    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
        throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    if(name.empty() || _contents.find(name) != _contents.end())
    {
        throw NameInUse(name);
    }

    DirectoryIPtr d = new DirectoryI(name, this);
    ObjectPrx node = c.adapter->add(d, d->id());
    _contents[name] = d;
    return DirectoryPrx::uncheckedCast(node);
}

// Slice Directory::destroy() operation.

void
FilesystemI::DirectoryI::destroy(const Current& c)
{
    if(!_parent)
    {
        throw PermissionDenied("Cannot destroy root directory");
    }

    {
        IceUtil::Mutex::Lock lock(_m);

        if(_destroyed)
        {
            throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
        }

        if(!_contents.empty())
        {
            throw PermissionDenied("Cannot destroy non-empty directory");
        }

        c.adapter->remove(id());
        _destroyed = true;
    }

    _parent->removeEntry(_name);
}

// DirectoryI constructor.

FilesystemI::DirectoryI::DirectoryI(const string& name, const DirectoryIPtr& parent)
    : NodeI(name, parent)
{
}

// Remove the entry from the _contents map.

void
FilesystemI::DirectoryI::removeEntry(const string& name)
{
    IceUtil::Mutex::Lock lock(_m);
    Contents::iterator i = _contents.find(name);
    if(i != _contents.end())
    {
        _contents.erase(i);
    }
}
