// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_COLLECTOR_H
#define ICE_COLLECTOR_H

#include <Ice/CollectorF.h>
#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/AcceptorF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/EndpointF.h>
#include <Ice/EventHandler.h>
#include <list>
#include <map>

namespace Ice
{

class LocalException;

}

namespace __Ice
{

class Incoming;

class ICE_API CollectorI : public EventHandlerI, public JTCRecursiveMutex
{
public:

    Instance instance() const;
    void prepareReply(Incoming*);
    void sendReply(Incoming*);
    bool destroyed() const;

    //
    // Operations from EventHandlerI
    //
    virtual int fd();
    virtual void close();
    virtual void receive();

private:

    CollectorI(const CollectorI&);
    void operator=(const CollectorI&);

    CollectorI(const ::Ice::ObjectAdapter&, const Transceiver&);
    virtual ~CollectorI();
    void destroy();
    friend class CollectorFactoryI; // May create and destroy CollectorIs

    void warning(const Ice::LocalException&) const;

    ::Ice::ObjectAdapter adapter_;
    ThreadPool threadPool_;
    Transceiver transceiver_;
    int fd_;
};

class ICE_API CollectorFactoryI : public EventHandlerI, public JTCMutex
{
public:

    //
    // Operations from EventHandlerI
    //
    virtual int fd();
    virtual void close();
    virtual void receive();

private:

    CollectorFactoryI(const CollectorFactoryI&);
    void operator=(const CollectorFactoryI&);

    CollectorFactoryI(const Instance&, const ::Ice::ObjectAdapter&,
		      const Endpoint&);
    virtual ~CollectorFactoryI();
    void destroy();
    // May create and destroy CollectorFactoryIs
    friend class CollectorFactoryFactoryI;

    void warning(const Ice::LocalException&) const;

    Instance instance_;
    ::Ice::ObjectAdapter adapter_;
    ThreadPool threadPool_;
    Acceptor acceptor_;
    int fd_;
    std::list<Collector> collectors_;
};

class ICE_API CollectorFactoryFactoryI : public Shared, public JTCMutex
{
public:

    CollectorFactory create(const ::Ice::ObjectAdapter&,
			    const Endpoint&);

private:

    CollectorFactoryFactoryI(const CollectorFactoryFactoryI&);
    void operator=(const CollectorFactoryFactoryI&);

    CollectorFactoryFactoryI(const Instance&);
    virtual ~CollectorFactoryFactoryI();
    void destroy();
    friend class InstanceI; // May create and destroy CollectorFactoryFactoryIs

    Instance instance_;
};

}

#endif
