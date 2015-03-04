// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SQL_SUBSCRIBER_MAP_H
#define SQL_SUBSCRIBER_MAP_H

#include <Ice/CommunicatorF.h>
#include <IceStorm/SubscriberRecord.h>
#include <IceDB/SqlTypes.h>

namespace IceStorm
{

typedef std::map<SubscriberRecordKey, SubscriberRecord> SubscriberMap;

class SqlSubscriberMap : public IceUtil::Shared
{
public:

    SqlSubscriberMap(const SqlDB::DatabaseConnectionPtr&, const std::string&, const Ice::CommunicatorPtr&);

    void put(const SqlDB::DatabaseConnectionPtr&, const SubscriberRecordKey&, const SubscriberRecord&);

    SubscriberRecord find(const SqlDB::DatabaseConnectionPtr&, const SubscriberRecordKey&);
    void getMap(const SqlDB::DatabaseConnectionPtr&, SubscriberMap&);

    void erase(const SqlDB::DatabaseConnectionPtr&, const SubscriberRecordKey&);
    void eraseTopic(const SqlDB::DatabaseConnectionPtr&, const Ice::Identity&);
    void clear(const SqlDB::DatabaseConnectionPtr&);

private:

    class QoSMap
    {
    public:

        QoSMap(const SqlDB::DatabaseConnectionPtr&, const std::string&, const Ice::CommunicatorPtr&);

        void put(const SqlDB::DatabaseConnectionPtr&, const SubscriberRecordKey&, const QoS&);
        QoS find(const SqlDB::DatabaseConnectionPtr&, const SubscriberRecordKey&);

        void erase(const SqlDB::DatabaseConnectionPtr&, const SubscriberRecordKey&);
        void eraseTopic(const SqlDB::DatabaseConnectionPtr&, const Ice::Identity&);

        void clear(const SqlDB::DatabaseConnectionPtr&);

    private:

        const std::string _table;
        const Ice::CommunicatorPtr _communicator;
    };

    const std::string _table;
    const Ice::CommunicatorPtr _communicator;
    QoSMap _qosMap;
};

typedef IceUtil::Handle<SqlSubscriberMap> SqlSubscriberMapPtr;

}

#endif
