// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <PersistentFilesystemI.h>
#include <Freeze/Freeze.h>

using namespace std;

//
// Filesystem::NodeI
//

IdentityNodeMap* Filesystem::NodeI::_map;

Filesystem::NodeI::NodeI(const DirectoryIPtr& parent)
    : _parent(parent), _destroyed(false)
{
}

//
// Filesystem::FileI
//

void
Filesystem::FileI::destroy(const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(_nodeMutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }
    _destroyed = true;
    _parent->removeEntry(_file->name);
    _map->erase(c.id);
    c.adapter->remove(c.id);
}

string
Filesystem::FileI::name(const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(_nodeMutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    return _file->name;
}


Filesystem::Lines
Filesystem::FileI::read(const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(_nodeMutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    return _file->text;
}

void
Filesystem::FileI::write(const Filesystem::Lines& text, const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(_nodeMutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    _file->text = text;
    _map->put(make_pair(c.id, _file));
}

Filesystem::FileI::FileI(const PersistentFilePtr& file, const DirectoryIPtr& parent)
    : NodeI(parent), _file(file)
{
}

//
// Filesystem::DirectoryI
//

Ice::ObjectAdapterPtr Filesystem::DirectoryI::_adapter;

void
Filesystem::DirectoryI::destroy(const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(_nodeMutex);

    if(!_parent)
    {
        throw Filesystem::PermissionDenied("Cannot destroy root directory");
    }
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }
    if(!_dir->nodes.empty())
    {
        throw Filesystem::PermissionDenied("Cannot destroy non-empty directory");
    }
    _destroyed = true;
    _parent->removeEntry(_dir->name);
    _map->erase(c.id);
    c.adapter->remove(c.id);
}

string
Filesystem::DirectoryI::name(const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(_nodeMutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    return _dir->name;
}

Filesystem::NodeDescSeq
Filesystem::DirectoryI::list(const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(_nodeMutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    NodeDict::const_iterator p;
    NodeDescSeq result;
    for(p = _dir->nodes.begin(); p != _dir->nodes.end(); ++p)
    {
        result.push_back(p->second);
    }
    return result;
}

Filesystem::NodeDesc
Filesystem::DirectoryI::find(const string& name, const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(_nodeMutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    NodeDict::const_iterator p = _dir->nodes.find(name);
    if(p == _dir->nodes.end())
    {
        throw NoSuchName(name);
    }
    return p->second;
}

Filesystem::DirectoryPrx
Filesystem::DirectoryI::createDirectory(const string& name, const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(_nodeMutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    if(name.empty() || _dir->nodes.find(name) != _dir->nodes.end())
    {
        throw NameInUse(name);
    }

    PersistentDirectoryPtr persistentDir = new PersistentDirectory;
    persistentDir->name = name;
    Ice::Identity id = c.adapter->getCommunicator()->stringToIdentity(IceUtil::generateUUID());
    DirectoryIPtr dir = new DirectoryI(id, persistentDir, this);
    _map->put(make_pair(id, persistentDir));

    DirectoryPrx proxy = DirectoryPrx::uncheckedCast(c.adapter->createProxy(id));

    NodeDesc nd;
    nd.name = name;
    nd.type = DirType;
    nd.proxy = proxy;
    _dir->nodes[name] = nd;

    _map->put(make_pair(c.id, _dir));

    c.adapter->add(dir, id);

    return proxy;
}

Filesystem::FilePrx
Filesystem::DirectoryI::createFile(const string& name, const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(_nodeMutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    if(name.empty() || _dir->nodes.find(name) != _dir->nodes.end())
    {
        throw NameInUse(name);
    }

    PersistentFilePtr persistentFile = new PersistentFile;
    persistentFile->name = name;
    FileIPtr file = new FileI(persistentFile, this);
    Ice::Identity id = c.adapter->getCommunicator()->stringToIdentity(IceUtil::generateUUID());
    _map->put(make_pair(id, persistentFile));

    FilePrx proxy = FilePrx::uncheckedCast(c.adapter->createProxy(id));

    NodeDesc nd;
    nd.name = name;
    nd.type = FileType;
    nd.proxy = proxy;
    _dir->nodes[name] = nd;

    _map->put(make_pair(c.id, _dir));

    c.adapter->add(file, id);

    return proxy;
}

//
// Called by the child to remove itself from the parent's node map when the child is destroyed.
//
void
Filesystem::DirectoryI::removeEntry(const string& name)
{
    IceUtil::Mutex::Lock lock(_nodeMutex);

    NodeDict::iterator p = _dir->nodes.find(name);
    assert(p != _dir->nodes.end());
    _dir->nodes.erase(p);
    _map->put(make_pair(_id, _dir));
}

Filesystem::DirectoryI::DirectoryI(const Ice::Identity& pid,
                                   const PersistentDirectoryPtr& pdir,
                                   const DirectoryIPtr& parent)
    : NodeI(parent), _id(pid), _dir(pdir)
{
    // Instantiate the child nodes
    //
    for(NodeDict::iterator p = _dir->nodes.begin(); p != _dir->nodes.end(); ++p)
    {
        Ice::Identity id = p->second.proxy->ice_getIdentity();
        PersistentNodePtr node = _map->find(id)->second;
        if(p->second.type == DirType)
        {
            PersistentDirectoryPtr pDir = PersistentDirectoryPtr::dynamicCast(node);
            assert(pDir);
            DirectoryIPtr d = new DirectoryI(id, pDir, this);
            _adapter->add(d, id);
        }
        else
        {
            PersistentFilePtr pFile = PersistentFilePtr::dynamicCast(node);
            assert(pFile);
            FileIPtr f = new FileI(pFile, this);
            _adapter->add(f, id);
        }
    }
}
