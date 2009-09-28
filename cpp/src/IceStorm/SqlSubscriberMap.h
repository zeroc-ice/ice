// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SQL_SUBSCRIBER_MAP_H
#define SQL_SUBSCRIBER_MAP_H

#include <Ice/CommunicatorF.h>
#include <IceStorm/SubscriberRecord.h>
#include <IceSQL/SqlTypes.h>

namespace IceStorm
{

typedef std::map<SubscriberRecordKey, SubscriberRecord> SubscriberMap;

class SqlSubscriberMap : public IceUtil::Shared
{
public:

    SqlSubscriberMap(const IceSQL::DatabaseConnectionPtr&, const std::string&, const Ice::CommunicatorPtr&);

    void put(const IceSQL::DatabaseConnectionPtr&, const SubscriberRecordKey&, const SubscriberRecord&);

    SubscriberRecord find(const IceSQL::DatabaseConnectionPtr&, const SubscriberRecordKey&);
    void getMap(const IceSQL::DatabaseConnectionPtr&, SubscriberMap&);

    void erase(const IceSQL::DatabaseConnectionPtr&, const SubscriberRecordKey&);
    void eraseTopic(const IceSQL::DatabaseConnectionPtr&, const Ice::Identity&);
    void clear(const IceSQL::DatabaseConnectionPtr&);

private:

    class QoSMap
    {
    public:

        QoSMap(const IceSQL::DatabaseConnectionPtr&, const std::string&, const Ice::CommunicatorPtr&);

        void put(const IceSQL::DatabaseConnectionPtr&, const SubscriberRecordKey&, const QoS&);
        QoS find(const IceSQL::DatabaseConnectionPtr&, const SubscriberRecordKey&);

        void erase(const IceSQL::DatabaseConnectionPtr&, const SubscriberRecordKey&);
        void eraseTopic(const IceSQL::DatabaseConnectionPtr&, const Ice::Identity&);
        void clear(const IceSQL::DatabaseConnectionPtr&);

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
