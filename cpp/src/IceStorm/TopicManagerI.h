// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TOPIC_MANAGER_I_H
#define TOPIC_MANAGER_I_H

#include <IceStorm/IceStorm.h>
#include <IceStorm/TopicIF.h>
#include <IceStorm/FlusherF.h>
#include <IceStorm/TraceLevelsF.h>
#include <IceStorm/StringBoolDict.h>

namespace IceStorm
{
 
typedef std::map<std::string, TopicIPtr> TopicIMap;

class TopicManagerI : public TopicManager, public JTCMutex
{
public:

    TopicManagerI(const Ice::CommunicatorPtr&, const Ice::ObjectAdapterPtr&, const TraceLevelsPtr&,
		  const Freeze::DBPtr&);
    ~TopicManagerI();

    virtual TopicPrx create(const std::string&, const Ice::Current&);
    virtual TopicPrx retrieve(const std::string&, const Ice::Current&);
    virtual TopicDict retrieveAll(const Ice::Current&);
    virtual void subscribe(const std::string&, const QoS&, const StringSeq&, const Ice::ObjectPrx&,
			   const Ice::Current&);
    virtual void unsubscribe(const std::string&, const StringSeq&, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    void reap();

    void installTopic(const std::string&, const std::string&);

    Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _adapter;
    TraceLevelsPtr _traceLevels;
    TopicIMap _topicIMap;
    FlusherPtr _flusher;
    StringBoolDict _topics;
};

} // End namespace IceStorm

#endif
