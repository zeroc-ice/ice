// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SERVICE_MANAGER_I_H
#define SERVICE_MANAGER_I_H

#include <IceBox/IceBox.h>
#include <Ice/LoggerF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/DynamicLibraryF.h>
#include <map>

namespace IceBox
{

class ServiceManagerI : public ServiceManager, public IceUtil::Mutex
{
public:

    ServiceManagerI(Ice::CommunicatorPtr, int&, char*[]);
    virtual ~ServiceManagerI();

    virtual Ice::SliceChecksumDict getSliceChecksums(const Ice::Current&) const;

    virtual void startService(const std::string&, const ::Ice::Current&);
    virtual void stopService(const std::string&, const ::Ice::Current&);

    virtual void addObserver(const ServiceObserverPrx&, const Ice::Current&);

    virtual void shutdown(const ::Ice::Current&);

    int run();

    struct ServiceInfo
    {
        ::std::string name;
        ServicePtr service;
        ::IceInternal::DynamicLibraryPtr library;
        ::Ice::CommunicatorPtr communicator;
        ::std::string envName;
        bool active;
        Ice::StringSeq args;
    };

    bool start();
    void stop();

    void removeObserver(const ServiceObserverPrx&, const Ice::Exception&);

private:

    void load(const std::string&, const std::string&);
    void start(const std::string&, const std::string&, const ::Ice::StringSeq&);
    void stopAll();

    void servicesStarted(const std::vector<std::string>&, const std::set<ServiceObserverPrx>&);
    void servicesStopped(const std::vector<std::string>&, const std::set<ServiceObserverPrx>&);
    void observerRemoved(const ServiceObserverPrx&, const std::exception&);

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::LoggerPtr _logger;
    ::Ice::StringSeq _argv; // Filtered server argument vector, not including program name
    std::vector<ServiceInfo> _services;

    std::set<ServiceObserverPrx> _observers;
    int _traceServiceObserver;
};

typedef IceUtil::Handle<ServiceManagerI> ServiceManagerIPtr;

}

#endif
