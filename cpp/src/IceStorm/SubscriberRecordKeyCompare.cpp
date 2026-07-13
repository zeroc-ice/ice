// Copyright (c) ZeroC, Inc.

#include "Util.h"

using namespace IceStorm;

namespace
{
    // Marshaling context used to decode keys inside compareSubscriberRecordKey. An LMDB comparator is
    // a plain function pointer with no user data, so the communicator is installed here (via
    // setKeyComparatorCommunicator) before the subscribers database is opened.
    IceDB::IceContext keyContext;
}

void
IceStormInternal::setKeyComparatorCommunicator(const Ice::CommunicatorPtr& communicator)
{
    keyContext.communicator = communicator;
}

int
IceStormInternal::compareSubscriberRecordKey(const MDB_val* v1, const MDB_val* v2)
{
    SubscriberRecordKey k1, k2;
    IceDB::Codec<SubscriberRecordKey, IceDB::IceContext, Ice::OutputStream>::read(k1, *v1, keyContext);
    IceDB::Codec<SubscriberRecordKey, IceDB::IceContext, Ice::OutputStream>::read(k2, *v2, keyContext);
    if (k1 < k2)
    {
        return -1;
    }
    else if (k1 == k2)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
