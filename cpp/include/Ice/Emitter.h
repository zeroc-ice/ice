// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_EMITTER_H
#define ICE_EMITTER_H

#include <Ice/EmitterF.h>
#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/ConnectorF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/EndpointDataF.h>
#include <Ice/EventHandler.h>
#include <map>

namespace _Ice
{

class Buffer;
class Outgoing;

class ICE_API EmitterI : public EventHandlerI, public JTCMutex
{
public:

    void request(Outgoing*);

    //
    // Operations from EventHandlerI
    //
    virtual int fd();
    virtual void close();
    virtual void receive();

private:

    EmitterI(const EmitterI&);
    void operator=(const EmitterI&);

    EmitterI(const Instance&, const Transceiver&);
    virtual ~EmitterI();
    void destroy();
    friend class EmitterFactoryI; // May create and destroy EmitterIs

    Instance instance_;
    ThreadPool threadPool_;
    Transceiver transceiver_;
};

class ICE_API EmitterFactoryI : public Shared, public JTCMutex
{
public:

    Emitter create();

private:

    EmitterFactoryI(const EmitterFactoryI&);
    void operator=(const EmitterFactoryI&);

    EmitterFactoryI(const Instance&, const EndpointData&);
    virtual ~EmitterFactoryI();
    void destroy();
    // May create and destroy EmitterFactoryIs
    friend class EmitterFactoryFactoryI;

    Instance instance_;
    Connector connector_;
    Emitter emitter_;
};

class ICE_API EmitterFactoryFactoryI : public Shared, public JTCMutex
{
public:

    EmitterFactory create(const EndpointData&);

private:

    EmitterFactoryFactoryI(const EmitterFactoryFactoryI&);
    void operator=(const EmitterFactoryFactoryI&);

    EmitterFactoryFactoryI(const Instance&);
    virtual ~EmitterFactoryFactoryI();
    void destroy();
    friend class InstanceI; // May create and destroy EmitterFactoryFactoryIs

    Instance instance_;
    std::map<EndpointData, EmitterFactory> factories_;
};

}

#endif
