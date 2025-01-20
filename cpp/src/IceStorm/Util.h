// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_UTIL_H
#define ICESTORM_UTIL_H

#include "../IceDB/IceDB.h"
#include "Ice/Ice.h"
#include "LLURecord.h"
#include "SubscriberRecord.h"

namespace IceStorm
{
    //
    // Forward declarations.
    //
    class Instance;

    using SubscriberMap =
        IceDB::Dbi<IceStorm::SubscriberRecordKey, IceStorm::SubscriberRecord, IceDB::IceContext, Ice::OutputStream>;
    using LLUMap = IceDB::Dbi<std::string, IceStormElection::LogUpdate, IceDB::IceContext, Ice::OutputStream>;

    const char* const lluDbKey = "_manager";
}

namespace IceStormInternal
{
    std::string identityToTopicName(const Ice::Identity&);

    Ice::Identity nameToIdentity(const std::shared_ptr<IceStorm::Instance>&, const std::string&);

    std::string describeEndpoints(const std::optional<Ice::ObjectPrx>&);

    int compareSubscriberRecordKey(const MDB_val* v1, const MDB_val* v2);

    IceStormElection::LogUpdate getIncrementedLLU(const IceDB::ReadWriteTxn&, IceStorm::LLUMap&);
}

#endif
