// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DATABASE_WRAPPER_H
#define DATABASE_WRAPPER_H

#include <IceGrid/DatabaseCache.h>

namespace IceGrid
{

#ifdef QTSQL

class ApplicationsDictWrapper
{
public:

    ApplicationsDictWrapper(const DatabaseCachePtr& database, const IceSQL::DatabaseConnectionPtr& connection) :
        _table(database->applications),
        _connection(connection)
    {
    }

    StringApplicationInfoDict getMap()
    {
        StringApplicationInfoDict dict;
        _table->getMap(_connection, dict);
        return dict;
    }

    void
    put(const std::string& name, const ApplicationInfo& info)
    {
        _table->put(_connection, name, info);
    }

    ApplicationInfo
    find(const std::string& name)
    {
        return _table->find(_connection, name);
    }

    void
    erase(const std::string& name)
    {
        _table->erase(_connection, name);
    }

    void
    clear()
    {
        _table->clear(_connection);
    }

private:

    const SqlStringApplicationInfoDictPtr _table;
    const IceSQL::DatabaseConnectionPtr _connection;
};

class AdaptersDictWrapper
{
public:

    AdaptersDictWrapper(const DatabaseCachePtr& database, const IceSQL::DatabaseConnectionPtr& connection) :
        _table(database->adapters),
        _connection(connection)
    {
    }

    StringAdapterInfoDict getMap()
    {
        StringAdapterInfoDict dict;
        _table->getMap(_connection, dict);
        return dict;
    }

    void
    put(const AdapterInfo& info)
    {
        _table->put(_connection, info);
    }

    AdapterInfo
    find(const std::string& name)
    {
        return _table->find(_connection, name);
    }

    std::vector<AdapterInfo>
    findByReplicaGroupId(const std::string& name)
    {
        return _table->findByReplicaGroupId(_connection, name);
    }

    void
    erase(const std::string& name)
    {
        _table->erase(_connection, name);
    }

    void
    clear()
    {
        _table->clear(_connection);
    }

private:

    const SqlStringAdapterInfoDictPtr _table;
    const IceSQL::DatabaseConnectionPtr _connection;
};

class ObjectsDictBaseWrapper
{
public:

    IdentityObjectInfoDict getMap()
    {
        IdentityObjectInfoDict dict;
        _table->getMap(_connection, dict);
        return dict;
    }

    void
    put(const Ice::Identity& id, const ObjectInfo& info)
    {
        _table->put(_connection, id, info);
    }

    ObjectInfo
    find(const Ice::Identity& id)
    {
        return _table->find(_connection, id);
    }

    std::vector<ObjectInfo>
    findByType(const std::string& type)
    {
        return _table->findByType(_connection, type);
    }

    void
    erase(const Ice::Identity& id)
    {
        _table->erase(_connection, id);
    }

    void
    clear()
    {
        _table->clear(_connection);
    }

protected:

    ObjectsDictBaseWrapper(const SqlIdentityObjectInfoDictPtr& table, const IceSQL::DatabaseConnectionPtr& connection) :
        _table(table),
        _connection(connection)
    {
    }

private:

    const SqlIdentityObjectInfoDictPtr _table;
    const IceSQL::DatabaseConnectionPtr _connection;
};

class ObjectsDictWrapper : public ObjectsDictBaseWrapper
{
public:

    ObjectsDictWrapper(const DatabaseCachePtr& database, const IceSQL::DatabaseConnectionPtr& connection) :
        ObjectsDictBaseWrapper(database->objects, connection)
    {
    }
};

class InternalObjectsDictWrapper : public ObjectsDictBaseWrapper
{
public:

    InternalObjectsDictWrapper(const DatabaseCachePtr& database, const IceSQL::DatabaseConnectionPtr& connection) :
        ObjectsDictBaseWrapper(database->internalObjects, connection)
    {
    }
};

#else

class NotFoundException : public std::exception
{
};

class ApplicationsDictWrapper
{
public:

    ApplicationsDictWrapper(const DatabaseCachePtr& database, DatabaseConnectionPtr& connection) :
        _applications(connection, "applications")
    {
    }

    StringApplicationInfoDict& getMap()
    {
        return _applications;
    }

    void
    put(const std::string& name, const ApplicationInfo& info)
    {
        _applications.put(StringApplicationInfoDict::value_type(name, info));
    }

    ApplicationInfo
    find(const std::string& name)
    {
        StringApplicationInfoDict::const_iterator p = _applications.find(name);
        if(p == _applications.end())
        {
            throw NotFoundException();
        }
        return p->second;
    }

    void
    erase(const std::string& name)
    {
        _applications.erase(name);
    }

    void
    clear()
    {
        _applications.clear();
    }

private:

    StringApplicationInfoDict _applications;
};

class AdaptersDictWrapper
{
public:

    AdaptersDictWrapper(const DatabaseCachePtr& database, DatabaseConnectionPtr& connection) :
        _adapters(connection, "adapters")
    {
    }

    StringAdapterInfoDict& getMap()
    {
        return _adapters;
    }

    void
    put(const AdapterInfo& info)
    {
        _adapters.put(StringAdapterInfoDict::value_type(info.id, info));
    }

    AdapterInfo
    find(const std::string& name)
    {
        StringAdapterInfoDict::const_iterator p = _adapters.find(name);
        if(p == _adapters.end())
        {
            throw NotFoundException();
        }
        return p->second;
    }

    std::vector<AdapterInfo>
    findByReplicaGroupId(const std::string& name)
    {
        std::vector<AdapterInfo> result;
        StringAdapterInfoDict::const_iterator p;
        for(p = _adapters.findByReplicaGroupId(name, true); p != _adapters.end(); ++p)
        {
            result.push_back(p->second);
        }
        return result;
    }

    void
    erase(const std::string& name)
    {
        _adapters.erase(name);
    }

    void
    clear()
    {
        _adapters.clear();
    }

private:

    StringAdapterInfoDict _adapters;
};

class ObjectsDictBaseWrapper
{
public:

    IdentityObjectInfoDict& getMap()
    {
        return _objects;
    }

    void
    put(const Ice::Identity& id, const ObjectInfo& info)
    {
        _objects.put(IdentityObjectInfoDict::value_type(id, info));
    }

    ObjectInfo
    find(const Ice::Identity& id)
    {
        IdentityObjectInfoDict::const_iterator p = _objects.find(id);
        if(p == _objects.end())
        {
            throw NotFoundException();
        }
        return p->second;
    }

    std::vector<ObjectInfo>
    findByType(const std::string& type)
    {
        std::vector<ObjectInfo> result;
        IdentityObjectInfoDict::const_iterator p;
        for(p = _objects.findByType(type); p != _objects.end(); ++p)
        {
            result.push_back(p->second);
        }
        return result;
    }

    void
    erase(const Ice::Identity& id)
    {
        _objects.erase(id);
    }

    void
    clear()
    {
        _objects.clear();
    }

protected:

    ObjectsDictBaseWrapper(const DatabaseConnectionPtr& connection, const std::string& name) :
        _objects(connection, name)
    {
    }

private:

    IdentityObjectInfoDict _objects;
};

class ObjectsDictWrapper : public ObjectsDictBaseWrapper
{
public:

    ObjectsDictWrapper(const DatabaseCachePtr& database, const DatabaseConnectionPtr& connection) :
        ObjectsDictBaseWrapper(connection, "objects")
    {
    }
};

class InternalObjectsDictWrapper : public ObjectsDictBaseWrapper
{
public:

    InternalObjectsDictWrapper(const DatabaseCachePtr& database, const DatabaseConnectionPtr& connection) :
        ObjectsDictBaseWrapper(connection, "internal-objects")
    {
    }
};

#endif

}

#endif
