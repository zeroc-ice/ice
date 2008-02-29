// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceStorm/PersistentTopicMap.h>
#include <IceStorm/SubscriberMap.h>
#include <IceStorm/IceStormInternal.h>
#include <IceStorm/LLUMap.h>
#include <Freeze/Freeze.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;

class Client : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Client app;
    int rc = app.main(argc, argv);
    return rc;
}

void
Client::usage()
{
    cerr << "Usage: " << appName() << " old-env new-env\n";
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
        cerr << argv[0] << ": database environment names must be different" << endl;
        return EXIT_FAILURE;
    }

    Freeze::ConnectionPtr oldCon = Freeze::createConnection(communicator(), oldEnvName);
    Freeze::ConnectionPtr newCon = Freeze::createConnection(communicator(), newEnvName);

    // We should not create the old database.
    PersistentTopicMap topicMap(oldCon, "topics", false);
    // Creating the new database is fine.
    SubscriberMap subscriberMap(newCon, "subscribers");
    LLUMap lluMap(newCon, "llu");

    Freeze::TransactionHolder oldTxn(oldCon);
    Freeze::TransactionHolder newTxn(newCon);
    for(PersistentTopicMap::const_iterator p = topicMap.begin(); p != topicMap.end(); ++p)
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

    // We need to write a record in the LLU map so that if this
    // database is used for a migration this database will be picked
    // as the latest. We use generation 1 since the default is 0.
    IceStormElection::LogUpdate llu;
    llu.generation = 1;
    llu.iteration = 0;
    lluMap.put(LLUMap::value_type("_manager", llu));

    oldTxn.rollback();
    newTxn.commit();

    return 0;
}
