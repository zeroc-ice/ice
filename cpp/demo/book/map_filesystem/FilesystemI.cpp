// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FilesystemI.h>
#include <IdentityFileEntryMap.h>
#include <IdentityDirectoryEntryMap.h>
#include <IceUtil/IceUtil.h> // For generateUUID.

using namespace std;
using namespace Filesystem;
using namespace FilesystemDB;

FileI::FileI(const Ice::CommunicatorPtr& communicator, const string& envName) :
    _communicator(communicator),
    _envName(envName)
{
}

string
FileI::name(const Ice::Current& c)
{
    const Freeze::ConnectionPtr connection(Freeze::createConnection(_communicator, _envName));
    IdentityFileEntryMap fileDB(connection, filesDB());

    for(;;)
    {
        try
        {
            IdentityFileEntryMap::iterator p = fileDB.find(c.id);
            if(p == fileDB.end())
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }
            return p->second.name;
        }
        catch(const Freeze::DeadlockException&)
        {
            continue;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            halt(ex);
        }
    }
}

Lines
FileI::read(const Ice::Current& c)
{
    const Freeze::ConnectionPtr connection(Freeze::createConnection(_communicator, _envName));
    IdentityFileEntryMap fileDB(connection, filesDB());

    for(;;)
    {
        try
        {
            IdentityFileEntryMap::iterator p = fileDB.find(c.id);
            if(p == fileDB.end())
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }
            return p->second.text;
        }
        catch(const Freeze::DeadlockException&)
        {
            continue;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            halt(ex);
        }
    }
}

void
FileI::write(const Filesystem::Lines& text, const Ice::Current& c)
{
    const Freeze::ConnectionPtr connection(Freeze::createConnection(_communicator, _envName));
    IdentityFileEntryMap fileDB(connection, filesDB());

    for(;;)
    {
        try
        {
            IdentityFileEntryMap::iterator p = fileDB.find(c.id);
            if(p == fileDB.end())
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }
            FileEntry entry = p->second;
            entry.text = text;
            p.set(entry);
            break;
        }
        catch(const Freeze::DeadlockException&)
        {
            continue;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            halt(ex);
        }
    }
}

void
FileI::destroy(const Ice::Current& c)
{
    const Freeze::ConnectionPtr connection(Freeze::createConnection(_communicator, _envName));
    IdentityFileEntryMap fileDB(connection, filesDB());
    IdentityDirectoryEntryMap dirDB(connection, DirectoryI::directoriesDB());

    for(;;)
    {
        try
        {
            //
            // The transaction is necessary since we are altering two
            // records in one atomic action.
            //
            Freeze::TransactionHolder txn(connection);

            IdentityFileEntryMap::iterator p = fileDB.find(c.id);
            if(p == fileDB.end())
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }
            FileEntry entry = p->second;

            IdentityDirectoryEntryMap::iterator pp = dirDB.find(entry.parent);
            if(pp == dirDB.end())
            {
                halt(Freeze::DatabaseException(__FILE__, __LINE__, "consistency error: file without parent"));
            }

            DirectoryEntry dirEntry = pp->second;
            dirEntry.nodes.erase(entry.name);
            pp.set(dirEntry);

            fileDB.erase(p);
            txn.commit();
            break;
        }
        catch(const Freeze::DeadlockException&)
        {
            continue;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            halt(ex);
        }
    }
}

string
FileI::filesDB()
{
    return "files";
}

void
FileI::halt(const Freeze::DatabaseException& ex) const
{
    Ice::Error error(_communicator->getLogger());
    error << "fatal exception: " << ex << "\n*** Aborting application ***";

    abort();
}

DirectoryI::DirectoryI(const Ice::CommunicatorPtr& communicator, const string& envName) :
    _communicator(communicator), _envName(envName)
{
    const Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityDirectoryEntryMap dirDB(connection, directoriesDB());

    //
    // Create the record for the root directory if necessary.
    //
    for(;;)
    {
        try
        {
            Ice::Identity rootId;
            rootId.name = "RootDir";
            IdentityDirectoryEntryMap::const_iterator p = dirDB.find(rootId);
            if(p == dirDB.end())
            {
                DirectoryEntry d;
                d.name = "/";
#ifdef __SUNPRO_CC
                dirDB.put(IdentityDirectoryEntryMap::value_type(rootId, d));
#else
                dirDB.put(make_pair(rootId, d));
#endif
            }
            break;
        }
        catch(const Freeze::DeadlockException&)
        {
            continue;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            halt(ex);
        }
    }
}

string
DirectoryI::name(const Ice::Current& c)
{
    const Freeze::ConnectionPtr connection(Freeze::createConnection(_communicator, _envName));
    IdentityDirectoryEntryMap directoryDB(connection, directoriesDB());

    for(;;)
    {
        try
        {
            IdentityDirectoryEntryMap::iterator p = directoryDB.find(c.id);
            if(p == directoryDB.end())
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }
            return p->second.name;
        }
        catch(const Freeze::DeadlockException&)
        {
            continue;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            halt(ex);
        }
    }
}

NodeDescSeq
DirectoryI::list(const Ice::Current& c)
{
    const Freeze::ConnectionPtr connection(Freeze::createConnection(_communicator, _envName));
    IdentityDirectoryEntryMap directoryDB(connection, directoriesDB());

    for(;;)
    {
        try
        {
            IdentityDirectoryEntryMap::iterator p = directoryDB.find(c.id);
            if(p == directoryDB.end())
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }
            NodeDescSeq result;
            for(StringNodeDescDict::const_iterator q = p->second.nodes.begin(); q != p->second.nodes.end(); ++q)
            {
                result.push_back(q->second);
            }
            return result;
        }
        catch(const Freeze::DeadlockException&)
        {
            continue;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            halt(ex);
        }
    }
}

NodeDesc
DirectoryI::find(const string& name, const Ice::Current& c)
{
    const Freeze::ConnectionPtr connection(Freeze::createConnection(_communicator, _envName));
    IdentityDirectoryEntryMap directoryDB(connection, directoriesDB());

    for(;;)
    {
        try
        {
            IdentityDirectoryEntryMap::iterator p = directoryDB.find(c.id);
            if(p == directoryDB.end())
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }
            StringNodeDescDict::const_iterator q = p->second.nodes.find(name);
            if(q == p->second.nodes.end())
            {
                throw NoSuchName(name);
            }
            return q->second;
        }
        catch(const Freeze::DeadlockException&)
        {
            continue;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            halt(ex);
        }
    }
}

DirectoryPrx
DirectoryI::createDirectory(const string& name, const Ice::Current& c)
{
    const Freeze::ConnectionPtr connection(Freeze::createConnection(_communicator, _envName));
    IdentityDirectoryEntryMap directoryDB(connection, directoriesDB());

    for(;;)
    {
        try
        {
            //
            // The transaction is necessary since we are altering two
            // records in one atomic action.
            //
            Freeze::TransactionHolder txn(connection);

            IdentityDirectoryEntryMap::iterator p = directoryDB.find(c.id);
            if(p == directoryDB.end())
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }

            DirectoryEntry entry = p->second;
            if(name.empty() || entry.nodes.find(name) != entry.nodes.end())
            {
                throw NameInUse(name);
            }

            DirectoryEntry d;
            d.name = name;
            d.parent = c.id;

            Ice::Identity id;
            id.name = IceUtil::generateUUID();
            DirectoryPrx proxy = DirectoryPrx::uncheckedCast(c.adapter->createProxy(id));

            NodeDesc nd;
            nd.name = name;
            nd.type = DirType;
            nd.proxy = proxy;
            entry.nodes.insert(make_pair(name, nd));

            p.set(entry);
#ifdef __SUNPRO_CC
            directoryDB.put(IdentityDirectoryEntryMap::value_type(id, d));
#else
            directoryDB.put(make_pair(id, d));
#endif

            txn.commit();

            return proxy;
        }
        catch(const Freeze::DeadlockException&)
        {
            continue;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            halt(ex);
        }
    }
}

FilePrx
DirectoryI::createFile(const string& name, const Ice::Current& c)
{
    const Freeze::ConnectionPtr connection(Freeze::createConnection(_communicator, _envName));
    IdentityFileEntryMap fileDB(connection, FileI::filesDB());
    IdentityDirectoryEntryMap dirDB(connection, directoriesDB());

    for(;;)
    {
        try
        {
            //
            // The transaction is necessary since we are altering two
            // records in one atomic action.
            //
            Freeze::TransactionHolder txn(connection);

            IdentityDirectoryEntryMap::iterator p = dirDB.find(c.id);
            if(p == dirDB.end())
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }

            DirectoryEntry entry = p->second;
            if(name.empty() || entry.nodes.find(name) != entry.nodes.end())
            {
                throw NameInUse(name);
            }

            FileEntry d;
            d.name = name;
            d.parent = c.id;

            Ice::Identity id;
            id.name = IceUtil::generateUUID();
            id.category = "file";
            FilePrx proxy = FilePrx::uncheckedCast(c.adapter->createProxy(id));

            NodeDesc nd;
            nd.name = name;
            nd.type = FileType;
            nd.proxy = proxy;
            entry.nodes.insert(make_pair(name, nd));

            p.set(entry);

#ifdef __SUNPRO_CC
            fileDB.put(IdentityFileEntryMap::value_type(id, d));
#else
            fileDB.put(make_pair(id, d));
#endif

            txn.commit();

            return proxy;
        }
        catch(const Freeze::DeadlockException&)
        {
            continue;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            halt(ex);
        }
    }
}

void
DirectoryI::destroy(const Ice::Current& c)
{
    const Freeze::ConnectionPtr connection(Freeze::createConnection(_communicator, _envName));
    IdentityDirectoryEntryMap directoryDB(connection, directoriesDB());

    for(;;)
    {
        try
        {
            //
            // The transaction is necessary since we are altering two
            // records in one atomic action.
            //
            Freeze::TransactionHolder txn(connection);

            IdentityDirectoryEntryMap::iterator p = directoryDB.find(c.id);
            if(p == directoryDB.end())
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }
            DirectoryEntry entry = p->second;
            if(entry.parent.name.empty())
            {
                throw PermissionDenied("Cannot destroy root directory");
            }
            if(!entry.nodes.empty())
            {
                throw PermissionDenied("Cannot destroy non-empty directory");
            }
            IdentityDirectoryEntryMap::iterator pp = directoryDB.find(entry.parent);
            if(pp == directoryDB.end())
            {
                halt(Freeze::DatabaseException(__FILE__, __LINE__, "consistency error: directory without parent"));
            }

            DirectoryEntry dirEntry = pp->second;
            dirEntry.nodes.erase(entry.name);
            pp.set(dirEntry);

            directoryDB.erase(p);
            txn.commit();
            break;
        }
        catch(const Freeze::DeadlockException&)
        {
            continue;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            halt(ex);
        }
    }
}

string
DirectoryI::directoriesDB()
{
    return "directories";
}

void
DirectoryI::halt(const Freeze::DatabaseException& ex) const
{
    Ice::Error error(_communicator->getLogger());
    error << "fatal exception: " << ex << "\n*** Aborting application ***";

    abort();
}
