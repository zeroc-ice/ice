//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

using SubscriberMap = IceDB::Dbi<IceStorm::SubscriberRecordKey,
                                 IceStorm::SubscriberRecord,
                                 IceDB::IceContext,
                                 Ice::OutputStream>;
using LLUMap = IceDB::Dbi<std::string, IceStormElection::LogUpdate, IceDB::IceContext, Ice::OutputStream>;

const std::string lluDbKey = "_manager";

}

namespace IceStormInternal
{

std::string
identityToTopicName(const Ice::Identity&);

Ice::Identity
nameToIdentity(const std::shared_ptr<IceStorm::Instance>&, const std::string&);

std::string
describeEndpoints(const std::shared_ptr<Ice::ObjectPrx>&);

int
compareSubscriberRecordKey(const MDB_val* v1, const MDB_val* v2);

IceStormElection::LogUpdate
getIncrementedLLU(const IceDB::ReadWriteTxn&, IceStorm::LLUMap&);

}

#endif
