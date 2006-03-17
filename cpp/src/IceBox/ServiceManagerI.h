// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

class ServiceManagerI : public ServiceManager
{
public:

    ServiceManagerI(Ice::CommunicatorPtr, int&, char*[]);
    virtual ~ServiceManagerI();

    virtual Ice::SliceChecksumDict getSliceChecksums(const Ice::Current&) const;

    virtual void shutdown(const ::Ice::Current&);

    int run();

    struct ServiceInfo
    {
        ServicePtr service;
        ::IceInternal::DynamicLibraryPtr library;
	::Ice::CommunicatorPtr communicator;
	::std::string envName;
    };

    bool start();
    void stop();

private:

    void load(const std::string&, const std::string&);
    void start(const std::string&, const std::string&, const ::Ice::StringSeq&);
    void stopAll();

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::LoggerPtr _logger;
    ::Ice::StringSeq _argv; // Filtered server argument vector, not including program name
    std::map<std::string, ServiceInfo> _services;
};

typedef IceUtil::Handle<ServiceManagerI> ServiceManagerIPtr;

}

#endif
