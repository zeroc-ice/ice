// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STORM_ICE
#define ICE_STORM_ICE

module IceStorm
{

interface Topic
{
    nonmutating string getName();
    
    Object* getPublisher();

    void destroy();
};

dictionary<string, Topic*> TopicDict;

exception TopicExists
{
};

exception NoSuchTopic
{
};

sequence<string> StringSeq;
dictionary<string, string> QoS;

interface TopicManager
{
    Topic* create(string name) throws TopicExists;

    Topic* retrieve(string name) throws NoSuchTopic;

    nonmutating TopicDict retrieveAll();

    void subscribe(Object* tmpl, string id, QoS qos, StringSeq topics);

    void unsubscribe(string id, StringSeq topics);

    void shutdown();
};

};

#endif
