// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FilesystemI.h>
#include <IceUtil/UUID.h>
#include <Freeze/Initialize.h>

using namespace std;
using namespace Ice;
using namespace Filesystem;
using namespace FilesystemI;

// Static members

string FilesystemI::NodeI::_envName;
string FilesystemI::NodeI::_dbName;
IceUtil::StaticMutex FilesystemI::DirectoryI::_lcMutex = ICE_STATIC_MUTEX_INITIALIZER;
FilesystemI::DirectoryI::ReapMap FilesystemI::DirectoryI::_reapMap;

// Slice Node::name() operation

std::string
FilesystemI::NodeI::name(const Current& c)
{
    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
	throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    return getPersistentNode()->name;
}

// NodeI id() member

Ice::Identity
FilesystemI::NodeI::id() const
{
    return _id;
}

// NodeI activate() member. Adds a servant to the ASM.

ObjectPrx
FilesystemI::NodeI::activate(const ObjectAdapterPtr& a)
{
    return a->add(this, _id);
}

// NodeI constructor

FilesystemI::NodeI::NodeI(const CommunicatorPtr& communicator, const Identity& id, const DirectoryIPtr& parent)
    : _map(Freeze::createConnection(communicator, _envName), _dbName),
      _parent(parent), _destroyed(false), _id(id)
{
}

// Look for the node in the map and take care of deadlocks

PersistentNodePtr 
FilesystemI::NodeI::findNode(const Identity& id) const 
{
    for(;;)
    {
	try
        {
	    IdentityNodeMap::const_iterator p = _map.find(id);
	    if(p == _map.end())
		return 0;
	    else
		return p->second;
	}
	catch(const Freeze::DeadlockException&)
        {
	    // Try again
	    // 
	}
    }
}

// Slice File::read() operation

Lines
FilesystemI::FileI::read(const Current& c)
{
    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
	throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    return _file->text;
}

// Slice File::write() operation

void
FilesystemI::FileI::write(const Lines& text, const Current& c)
{
    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
	throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    _file->text = text;
    _map.put(IdentityNodeMap::value_type(_id, _file));
}

// Slice File::destroy() operation

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

    c.adapter->remove(c.id);
    _parent->addReapEntry(_file->name);
    _map.erase(c.id);
}

// FileI constructor

FilesystemI::FileI::FileI(const ObjectAdapterPtr& adapter, const Identity& id,
                          const PersistentFilePtr& file, const DirectoryIPtr& parent) :
    NodeI(adapter->getCommunicator(), id, parent), _file(file)
{
}

PersistentNodePtr
FilesystemI::FileI::getPersistentNode() const
{
    return _file;
}

// Slice Directory::list() operation

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

    NodeDescSeq result;
    for(NodeDict::const_iterator i = _dir->nodes.begin(); i != _dir->nodes.end(); ++i)
    {
        result.push_back(i->second);
    }
    return result;
}

// Slice Directory::find() operation

NodeDesc
FilesystemI::DirectoryI::find(const std::string& name, const Current& c)
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
    
    NodeDict::const_iterator i = _dir->nodes.find(name);
    if(i == _dir->nodes.end())
    {
        throw NoSuchName(name);
    }
    return i->second;
}

// Slice Directory::createFile() operation

FilePrx
FilesystemI::DirectoryI::createFile(const std::string& name, const Current& c)
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

    if(name.empty() || _dir->nodes.find(name) != _dir->nodes.end())
    {
	throw NameInUse(name);
    }

    PersistentFilePtr persistentFile = new PersistentFile;
    persistentFile->name = name;
    CommunicatorPtr communicator = c.adapter->getCommunicator();
    FileIPtr file = new FileI(c.adapter, communicator->stringToIdentity(IceUtil::generateUUID()),
                              persistentFile, this);
    assert(findNode(file->id()) == 0);
    _map.put(IdentityNodeMap::value_type(file->id(), persistentFile));

    FilePrx proxy = FilePrx::uncheckedCast(c.adapter->createProxy(file->id()));

    NodeDesc nd;
    nd.name = name;
    nd.type = FileType;
    nd.proxy = proxy;
    _dir->nodes[name] = nd;
    _map.put(IdentityNodeMap::value_type(_id, _dir));

    file->activate(c.adapter);

    return proxy;
}

// Slice Directory::createDirectory() operation

DirectoryPrx
FilesystemI::DirectoryI::createDirectory(const std::string& name, const Current& c)
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

    if(name.empty() || _dir->nodes.find(name) != _dir->nodes.end())
    {
	throw NameInUse(name);
    }

    PersistentDirectoryPtr persistentDir = new PersistentDirectory;
    persistentDir->name = name;
    CommunicatorPtr communicator = c.adapter->getCommunicator();
    DirectoryIPtr dir = new DirectoryI(c.adapter, communicator->stringToIdentity(IceUtil::generateUUID()),
                                       persistentDir, this);
    assert(findNode(dir->_id) == 0);
    _map.put(IdentityNodeMap::value_type(dir->_id, persistentDir));

    DirectoryPrx proxy = DirectoryPrx::uncheckedCast(c.adapter->createProxy(dir->_id));

    NodeDesc nd;
    nd.name = name;
    nd.type = DirType;
    nd.proxy = proxy;
    _dir->nodes[name] = nd;
    _map.put(IdentityNodeMap::value_type(_id, _dir));

    dir->activate(c.adapter);

    return proxy;
}

// Slice Directory::destroy() operation (inherited from Node)

void
FilesystemI::DirectoryI::destroy(const Current& c)
{
    if(!_parent)
    {
        throw Filesystem::PermissionDenied("Cannot destroy root directory");
    }

    IceUtil::Mutex::Lock lock(_m);

    if(_destroyed)
    {
        throw ObjectNotExistException(__FILE__, __LINE__, c.id, c.facet, c.operation);
    }

    IceUtil::StaticMutex::Lock lcLock(DirectoryI::_lcMutex);

    reap();

    if(!_dir->nodes.empty())
    {
        throw PermissionDenied("Cannot destroy non-empty directory");
    }

    c.adapter->remove(c.id);
    _parent->addReapEntry(_dir->name);
    _map.erase(c.id);
    _destroyed = true;
}

// DirectoryI constructor

FilesystemI::DirectoryI::DirectoryI(const ObjectAdapterPtr& adapter, const Identity& id,
                                    const PersistentDirectoryPtr& dir, const DirectoryIPtr& parent)
    : NodeI(adapter->getCommunicator(), id, parent), _dir(dir)
{
    //
    // Instantiate the child nodes
    //
    vector<string> staleEntries;

    for(NodeDict::iterator p = dir->nodes.begin(); p != dir->nodes.end(); ++p)
    {
	Identity id = p->second.proxy->ice_getIdentity();
	PersistentNodePtr node = findNode(id);
        NodeIPtr servant;
        if(node)
        {
            if(p->second.type == DirType)
            {
                PersistentDirectoryPtr pDir = PersistentDirectoryPtr::dynamicCast(node);
                assert(pDir);
                servant = new DirectoryI(adapter, id, pDir, this);
            }
            else
            {
                PersistentFilePtr pFile = PersistentFilePtr::dynamicCast(node);
                assert(pFile);
                servant = new FileI(adapter, id, pFile, this);
            }
            servant->activate(adapter);
        }
        else
        {
            staleEntries.push_back(p->first);
        }
    }

    for(vector<string>::const_iterator i = staleEntries.begin(); i != staleEntries.end(); ++i)
    {
        dir->nodes.erase(*i);
    }
    if(!staleEntries.empty())
    {
        _map.put(IdentityNodeMap::value_type(id, dir));
    }
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
            i->first->_dir->nodes.erase(*j);
        }
   }
   _reapMap.clear();
}

PersistentNodePtr
FilesystemI::DirectoryI::getPersistentNode() const
{
    return _dir;
}
