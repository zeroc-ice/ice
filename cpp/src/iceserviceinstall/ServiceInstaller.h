// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SERVICE_INSTALLER_H
#define ICE_SERVICE_INSTALLER_H

#include <Ice/Ice.h>
#include <AccCtrl.h>

class IceServiceInstaller
{
public:

#if defined(_MSC_VER) && _MSC_VER < 1300
    enum { 
        icegridregistry = 0, 
        icegridnode = 1,
        glacier2router = 2,
        serviceCount = 3
    };
#else
    static const int icegridregistry = 0;
    static const int icegridnode = 1;
    static const int glacier2router = 2;
    static const int serviceCount = 3;
#endif

    IceServiceInstaller(int, const std::string&, const Ice::CommunicatorPtr&);

    void install(const Ice::PropertiesPtr&);

    void uninstall();

    static std::vector<std::string> getPropertyNames();
    static std::string serviceTypeToString(int);
    static std::string serviceTypeToLowerString(int);

private:

    void initializeSid(const std::string&);

    bool fileExists(const std::string&) const;
    void grantPermissions(const std::string& path, SE_OBJECT_TYPE type = SE_FILE_OBJECT,
                     bool inherit = false, bool fullControl = false) const;
    bool mkdir(const std::string&) const;

    void addLog(const std::string&) const;
    void removeLog(const std::string&) const;
    std::string createLog(const std::string&) const;
    void addSource(const std::string&, const std::string&, const std::string&) const;
    std::string removeSource(const std::string&) const;
    std::string mangleSource(const std::string&) const;
    std::string createSource(const std::string&, const std::string&) const;

    std::string getIceDLLPath(const std::string&) const;

    int _serviceType;
    std::string _configFile;
    Ice::CommunicatorPtr _communicator;

    Ice::PropertiesPtr _serviceProperties;
    std::string _serviceName;
    Ice::LocatorPrx _defaultLocator;

    std::string _icegridInstanceName;
    std::string _nodeName;
    std::string _glacier2InstanceName;

    std::auto_ptr<SID> _sid;
    std::string _sidName;

    bool _debug;
};

#endif
