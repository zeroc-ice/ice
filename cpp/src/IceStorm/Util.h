// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef UTIL_H
#define UTIL_H

#include <Ice/Ice.h>
#include <IceDB/IceDB.h>
#include <IceStorm/LLURecord.h>
#include <IceStorm/SubscriberRecord.h>

namespace IceStorm
{

//
// Forward declarations.
//
class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

typedef IceDB::Dbi<IceStorm::SubscriberRecordKey, IceStorm::SubscriberRecord, IceDB::IceContext, Ice::OutputStream>
        SubscriberMap;
typedef IceDB::Dbi<std::string, IceStormElection::LogUpdate, IceDB::IceContext, Ice::OutputStream> LLUMap;

const std::string lluDbKey = "_manager";

}

namespace IceStormInternal
{

std::string
identityToTopicName(const Ice::Identity&);

Ice::Identity
nameToIdentity(const IceStorm::InstancePtr&, const std::string&);

std::string
describeEndpoints(const Ice::ObjectPrx&);

int
compareSubscriberRecordKey(const MDB_val* v1, const MDB_val* v2);

IceStormElection::LogUpdate
getIncrementedLLU(const IceDB::ReadWriteTxn&, IceStorm::LLUMap&);

}

#endif
