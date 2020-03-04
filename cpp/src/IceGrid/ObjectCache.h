//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_OBJECTCACHE_H
#define ICE_GRID_OBJECTCACHE_H

#include <Ice/CommunicatorF.h>
#include <IceGrid/Cache.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class ObjectCache;

class ObjectEntry
{
public:

    ObjectEntry(const ObjectInfo&, const std::string&, const std::string&);
    std::shared_ptr<Ice::ObjectPrx> getProxy() const;
    std::string getType() const;
    std::string getApplication() const;
    std::string getServer() const;
    const ObjectInfo& getObjectInfo() const;

    bool canRemove();

private:

    const ObjectInfo _info;
    const std::string _application;
    const std::string _server;
};

class ObjectCache : public Cache<Ice::Identity, ObjectEntry>
{
public:

    ObjectCache(const std::shared_ptr<Ice::Communicator>&);

    void add(const ObjectInfo&, const std::string&, const std::string&);
    std::shared_ptr<ObjectEntry> get(const Ice::Identity&) const;
    void remove(const Ice::Identity&);

    std::vector<std::shared_ptr<ObjectEntry>> getObjectsByType(const std::string&);

    ObjectInfoSeq getAll(const std::string&);
    ObjectInfoSeq getAllByType(const std::string&);

    const std::shared_ptr<Ice::Communicator>& getCommunicator() const { return _communicator; }

private:

    class TypeEntry
    {
    public:

        void add(const std::shared_ptr<ObjectEntry>&);
        bool remove(const std::shared_ptr<ObjectEntry>&);

        const std::vector<std::shared_ptr<ObjectEntry>>& getObjects() const { return _objects; }

    private:

        std::vector<std::shared_ptr<ObjectEntry>> _objects;
    };

    const std::shared_ptr<Ice::Communicator> _communicator;
    std::map<std::string, TypeEntry> _types;

};

};

#endif
