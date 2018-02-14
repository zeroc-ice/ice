// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/Freeze.h>

#include <IceStorm/IceStormInternal.h>

#include <IceStorm/SubscriberMap.h>
#include <IceStorm/LLUMap.h>
#include <IceStorm/V32FormatDB.h>
#include <IceStorm/V31FormatDB.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;

class Client : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);

private:

    void v32migrate(const Freeze::ConnectionPtr&, SubscriberMap&);
    void v31migrate(const Freeze::ConnectionPtr&, SubscriberMap&);
};

#ifdef _WIN32

int
wmain(int argc, wchar_t* argv[])

#else

int
main(int argc, char* argv[])

#endif
{
    Client app;
    int rc = app.main(argc, argv);
    return rc;
}

void
Client::usage()
{
    cerr << "Usage: " << appName() << " old-env new-env\n";
    cerr << "\n";
    cerr << "This utility upgrades a 3.1 or 3.2 IceStorm database environment\n";
    cerr << "to a 3.3 or superior IceStorm database environment.\n";
}

string
identityToTopicName(const Ice::Identity& id)
{
    //
    // Work out the topic name. If the category is empty then we're in
    // backwards compatibility mode and the name is just
    // identity.name. Otherwise identity.name is topic.<topicname>.
    //
    if(id.category.empty())
    {
        return id.name;
    }

    assert(id.name.length() > 6 && id.name.compare(0, 6, "topic.") == 0);
    return id.name.substr(6);
}

void
Client::v32migrate(const Freeze::ConnectionPtr& oldCon, SubscriberMap& subscriberMap)
{
    // We should not create the old database.
    V32Format topicMap(oldCon, "topics", false);
    Freeze::TransactionHolder oldTxn(oldCon);

    for(V32Format::const_iterator p = topicMap.begin(); p != topicMap.end(); ++p)
    {
        // First the placeholder record for the topic.
        SubscriberRecordKey key;
        key.topic = p->first;
        SubscriberRecord rec;
        rec.link = false;
        rec.cost = 0;
        subscriberMap.put(SubscriberMap::value_type(key, rec));

        string topicName = identityToTopicName(key.topic);

        // Next each link.
        for(LinkRecordSeq::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
        {
            Ice::Identity id = q->theTopic->ice_getIdentity();
            key.id = id;

            rec.id = id;
            rec.obj = q->obj;
            rec.theTopic = q->theTopic;
            rec.topicName = topicName;
            rec.link = true;
            rec.cost = q->cost;

            subscriberMap.put(SubscriberMap::value_type(key, rec));
        }
    }

    oldTxn.rollback();
}

void
Client::v31migrate(const Freeze::ConnectionPtr& oldCon, SubscriberMap& subscriberMap)
{

    // We should not create the old database.
    V31Format topicMap(oldCon, "topics", false);
    Freeze::TransactionHolder oldTxn(oldCon);

    for(V31Format::const_iterator p = topicMap.begin(); p != topicMap.end(); ++p)
    {
        // First the placeholder record for the topic.
        SubscriberRecordKey key;
        key.topic.name = p->first;
        SubscriberRecord rec;
        rec.link = false;
        rec.cost = 0;
        subscriberMap.put(SubscriberMap::value_type(key, rec));

        string topicName = identityToTopicName(key.topic);

        // Next each link.
        for(LinkRecordDict::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
        {
            Ice::Identity id = q->second.theTopic->ice_getIdentity();
            key.id = id;

            rec.id = id;
            rec.obj = q->second.obj;
            rec.theTopic = q->second.theTopic;
            rec.topicName = topicName;
            rec.link = true;
            rec.cost = q->second.cost;

            subscriberMap.put(SubscriberMap::value_type(key, rec));
        }
    }

    oldTxn.rollback();
}

int
Client::run(int argc, char* argv[])
{
    if(argc != 3)
    {
        usage();
        return EXIT_FAILURE;
    }

    string oldEnvName = argv[1];
    string newEnvName = argv[2];

    if(oldEnvName == newEnvName)
    {
        cerr << argv[0] << ": The database environment names must be different" << endl;
        return EXIT_FAILURE;
    }

    bool migrate31 = false;

    Freeze::ConnectionPtr oldCon = Freeze::createConnection(communicator(), oldEnvName);
    Freeze::Catalog catalog(oldCon, Freeze::catalogName());
    if(catalog.size() != 1 || catalog.begin()->first != "topics")
    {
        cerr << argv[0] << ": The old database environment does not contain an IceStorm database." << endl;
        return EXIT_FAILURE;
    }
    Freeze::CatalogData data = catalog.begin()->second;
    if(!data.evictor && data.key == "string" && data.value == "::IceStorm::LinkRecordDict")
    {
        migrate31 = true;
    }
    else if(!data.evictor && data.key == "::Ice::Identity" && data.value == "::IceStorm::LinkRecordSeq")
    {
        migrate31 = false;
    }
    else
    {
        cerr << argv[0] << ": The old environment contains an unrecognized IceStorm database version." << endl;
        return EXIT_FAILURE;
    }

    // Creating the new database is fine.
    Freeze::ConnectionPtr newCon = Freeze::createConnection(communicator(), newEnvName);
    SubscriberMap subscriberMap(newCon, "subscribers");
    LLUMap lluMap(newCon, "llu");
    Freeze::TransactionHolder newTxn(newCon);

    if(migrate31)
    {
        v31migrate(oldCon, subscriberMap);
    }
    else
    {
        v32migrate(oldCon, subscriberMap);
    }

    // We need to write a record in the LLU map so that if this
    // database is used for a migration this database will be picked
    // as the latest. We use generation 1 since the default is 0.
    IceStormElection::LogUpdate llu;
    llu.generation = 1;
    llu.iteration = 0;
    lluMap.put(LLUMap::value_type("_manager", llu));

    newTxn.commit();

    return 0;
}
