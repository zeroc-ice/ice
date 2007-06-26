// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <IceGrid/Query.h>

#include <Clock.h>

#include <set>

using namespace std;
using namespace Demo;

class InstanceCheckThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    InstanceCheckThread(const IceGrid::QueryPrx& query, const string& topicName, const Ice::ObjectPrx& clock,
                       const Ice::ObjectPrx& managerReplica, const set<IceStorm::TopicManagerPrx>& managers,
                       const Ice::ObjectPrx& topicReplica, const set<IceStorm::TopicPrx>& topics) :
        _query(query),
        _topicName(topicName),
        _clock(clock),
        _managerReplica(managerReplica),
        _managers(managers),
        _topicReplica(topicReplica),
        _topics(topics),
        _timeout(IceUtil::Time::seconds(10)),
        _terminated(false)
    {
    }

    virtual void
    run()
    {
        Lock sync(*this);
        while(!_terminated)
        {
            timedWait(_timeout);
            if(!_terminated)
            {
                try
                {
                    //
                    // Check if there are any new topic managers in the replicas list.
                    //
                    Ice::ObjectProxySeq managers = _query->findAllReplicas(_managerReplica);
                    Ice::ObjectProxySeq::const_iterator p;
                    for(p = managers.begin(); p != managers.end(); ++p)
                    {
                        IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::uncheckedCast(*p);
                        if(_managers.find(manager) == _managers.end())
                        {
                            //
                            // Create the topic on the new topic manager if it does
                            // not already exist.
                            //
                            try
                            {
                                manager->retrieve(_topicName);
                            }
                            catch(const IceStorm::NoSuchTopic&)
                            {
                                manager->create(_topicName);
                            }

                            //
                            // Since the topic proxy returned by the retrieval/creation
                            // is a replicated proxy we cannot use it to subscribe to the
                            // new instance. Instead we ahve to retrive all the topics and
                            // find the new one.
                            //
                            Ice::ObjectProxySeq topics = _query->findAllReplicas(_topicReplica);
                            Ice::ObjectProxySeq::const_iterator q;
                            for(q = topics.begin(); q != topics.end(); ++q)
                            {
                                IceStorm::TopicPrx topic = IceStorm::TopicPrx::uncheckedCast(*q);;
                                if(_topics.find(topic) == _topics.end())
                                {
                                    try
                                    {
                                        topic->subscribeAndGetPublisher(IceStorm::QoS(), _clock);
                                    }
                                    catch(const IceStorm::AlreadySubscribed&)
                                    {
                                        // Ignore
                                    }
                                    _topics.insert(topic);
                                    _managers.insert(manager);
                                    break;
                                }
                            }
                        }
                    }
                }
                catch(const Ice::Exception& ex)
                {
                    cerr << "warning: exception while checking for new IceStorm instances: " << ex << endl;
                }
            }
        }
    }

    void
    terminate()
    {
        Lock sync(*this);
        _terminated = true;
        notify();
    }

private:

    const IceGrid::QueryPrx _query;
    const string _topicName;
    const Ice::ObjectPrx _clock;
    const Ice::ObjectPrx _managerReplica;
    set<IceStorm::TopicManagerPrx> _managers;
    const Ice::ObjectPrx _topicReplica;
    set<IceStorm::TopicPrx> _topics;
    const IceUtil::Time _timeout;
    bool _terminated;
};

typedef IceUtil::Handle<InstanceCheckThread> InstanceCheckThreadPtr;

class ClockI : public Clock
{
public:

    virtual void
    tick(const string& time, const Ice::Current&)
    {
        cout << time << endl;
    }
};

class Subscriber : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Subscriber app;
    return app.main(argc, argv, "config.sub");
}

int
Subscriber::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();

    IceGrid::QueryPrx query = IceGrid::QueryPrx::uncheckedCast(communicator()->stringToProxy("DemoIceGrid/Query"));
    Ice::ObjectPrx managerReplica = communicator()->stringToProxy("DemoIceStorm/TopicManager");
    Ice::ObjectProxySeq objSeq = query->findAllReplicas(managerReplica);
    if(objSeq.size() == 0)
    {
        cerr << appName() << ": no topic managers found, make sure application was deployed." << endl;
        return EXIT_FAILURE;
    }

    string topicName = "time";
    if(argc != 1)
    {
        topicName = argv[1];
    }

    //
    // Create the servant to receive the events.
    //
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Clock.Subscriber");

    //
    // We want to use oneway batch messages.
    //
    Ice::ObjectPrx clock = adapter->addWithUUID(new ClockI)->ice_batchOneway();

    //
    // Get all the topic managers and create the topic if necessary.
    //
    IceStorm::TopicPrx topicReplica;
    set<IceStorm::TopicManagerPrx> managers;
    Ice::ObjectProxySeq::const_iterator p;
    for(p = objSeq.begin(); p != objSeq.end(); ++p)
    {
        IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(*p);
        managers.insert(manager);

        try
        {
            topicReplica = manager->retrieve(topicName);
        }
        catch(const IceStorm::NoSuchTopic&)
        {
            try
            {
                topicReplica = manager->create(topicName);
            }
            catch(const IceStorm::TopicExists&)
            {
                cerr << appName() << ": temporary failure. try again." << endl;
                return EXIT_FAILURE;
            }
        }
    }

    //
    // Get all the topics and subscribe. We can't use the proxies returned by
    // the topic creation above because they are repicated proxies, and not 
    // specific proxies for each individual topic.
    //
    IceStorm::TopicPrx topic;
    objSeq = query->findAllReplicas(topicReplica);
    set<IceStorm::TopicPrx> topics;
    for(p = objSeq.begin(); p != objSeq.end(); ++p)
    {
        topic = IceStorm::TopicPrx::uncheckedCast(*p);
        topic->subscribeAndGetPublisher(IceStorm::QoS(), clock);
        topics.insert(topic);
    }

    //
    // Create and start thread to check for new IceStorm instances coming online.
    //
    InstanceCheckThreadPtr instanceCheck = 
        new InstanceCheckThread(query, topicName, clock, managerReplica, managers, topicReplica, topics);
    instanceCheck->start();

    adapter->activate();
    shutdownOnInterrupt();
    communicator()->waitForShutdown();

    instanceCheck->terminate();
    instanceCheck->getThreadControl().join();

    //
    // Unsubscribe from all topics.
    //
    objSeq = query->findAllReplicas(topicReplica);
    for(p = objSeq.begin(); p != objSeq.end(); ++p)
    {
        topic = IceStorm::TopicPrx::uncheckedCast(*p);
        topic->unsubscribe(clock);
    }

    return EXIT_SUCCESS;
}
