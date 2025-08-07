// Copyright (c) ZeroC, Inc.

#include "Util.h"
#include "Instance.h"
#include "SubscriberRecord.h"

using namespace IceStorm;
using namespace std;

namespace IceStormInternal
{
    IceDB::IceContext dbContext;
}

string
IceStormInternal::identityToTopicName(const Ice::Identity& id)
{
    // Work out the topic name. If the category is empty then we're in backwards compatibility mode and the name is
    // just identity.name. Otherwise identity.name is topic.<topicname>.
    if (id.category.empty())
    {
        return id.name;
    }

    assert(id.name.length() > 6 && id.name.compare(0, 6, "topic.") == 0);
    return id.name.substr(6);
}

Ice::Identity
IceStormInternal::nameToIdentity(const shared_ptr<Instance>& instance, const string& name)
{
    // Identity is <instanceName>/topic.<topicname>
    return {"topic." + name, instance->instanceName()};
}

string
IceStormInternal::describeEndpoints(const optional<Ice::ObjectPrx>& proxy)
{
    ostringstream os;
    if (proxy)
    {
        Ice::EndpointSeq endpoints = proxy->ice_getEndpoints();
        for (auto i = endpoints.cbegin(); i != endpoints.cend(); ++i)
        {
            if (i != endpoints.begin())
            {
                os << ", ";
            }
            os << "\"" << (*i)->toString() << "\"";
        }
    }
    else
    {
        os << "subscriber proxy is null";
    }
    return os.str();
}

int
IceStormInternal::compareSubscriberRecordKey(const MDB_val* v1, const MDB_val* v2)
{
    SubscriberRecordKey k1, k2;
    IceDB::Codec<SubscriberRecordKey, IceDB::IceContext, Ice::OutputStream>::read(k1, *v1, dbContext);
    IceDB::Codec<SubscriberRecordKey, IceDB::IceContext, Ice::OutputStream>::read(k2, *v2, dbContext);
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

IceStormElection::LogUpdate
IceStormInternal::getIncrementedLLU(const IceDB::ReadWriteTxn& txn, LLUMap& lluMap)
{
    IceStormElection::LogUpdate llu{0, 0};
    [[maybe_unused]] bool ok = lluMap.get(txn, lluDbKey, llu);
    assert(ok);

    llu.iteration++;
    lluMap.put(txn, lluDbKey, llu);
    return llu;
}
