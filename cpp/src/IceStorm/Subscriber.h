// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <Ice/LoggerF.h>

#include <IceStorm/IceStorm.h> // For QoS (nasty!)
#include <IceStorm/SubscriberF.h>
#include <IceStorm/TraceLevelsF.h>
#include <IceStorm/FlusherF.h>

#include <list>

namespace IceStorm
{

class Subscriber : public IceUtil::Shared
{
public:

    Subscriber(const Ice::LoggerPtr&, const TraceLevelsPtr&, const FlusherPtr&, const QoS&, const Ice::ObjectPrx&);
    ~Subscriber();

    bool invalid() const;

    void unsubscribe();

    void flush();
    void publish(const std::string&, bool, const std::vector< ::Ice::Byte>&);

    Ice::Identity id() const;

    // TODO: should there be a global operator==?
    bool operator==(const Subscriber&) const;

private:

    // Immutable
    Ice::LoggerPtr _logger;
    TraceLevelsPtr _traceLevels; 

    JTCMutex _invalidMutex;
    bool _invalid;

    //
    // This id is the full id of the subscriber for a particular topic
    // (that is <prefix>#<topicname>
    //
    // Immutable
    std::string _id;

    // Immutable
    Ice::ObjectPrx _obj;

    FlusherPtr _flusher;
};


typedef std::list<SubscriberPtr> SubscriberList;

} // End namespace IceStorm

#endif
