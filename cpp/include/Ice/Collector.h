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
#include <Ice/EndpointDataF.h>
#include <Ice/EventHandler.h>
#include <list>
#include <map>

namespace Ice
{

class LocalException;

}

namespace _Ice
{

class ICE_API CollectorI : public EventHandlerI, public JTCRecursiveMutex
{
public:

    Instance instance() const;
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

    CollectorI(const Instance&, const Transceiver&);
    virtual ~CollectorI();
    void destroy();
    friend class CollectorFactoryI; // May create and destroy CollectorIs

    void warning(const Ice::LocalException&) const;

    Instance instance_;
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

    CollectorFactoryI(const Instance&, const EndpointData&);
    virtual ~CollectorFactoryI();
    void destroy();
    // May create and destroy CollectorFactoryIs
    friend class CollectorFactoryFactoryI;

    void warning(const Ice::LocalException&) const;

    Instance instance_;
    ThreadPool threadPool_;
    Acceptor acceptor_;
    int fd_;
    std::list<Collector> collectors_;
};

class ICE_API CollectorFactoryFactoryI : public Shared, public JTCMutex
{
public:

    CollectorFactory create(const EndpointData&);

private:

    CollectorFactoryFactoryI(const CollectorFactoryFactoryI&);
    void operator=(const CollectorFactoryFactoryI&);

    CollectorFactoryFactoryI(const Instance&);
    virtual ~CollectorFactoryFactoryI();
    void destroy();
    friend class InstanceI; // May create and destroy CollectorFactoryFactoryIs

    Instance instance_;
    std::map<EndpointData, CollectorFactory> factories_;
};

}

#endif
