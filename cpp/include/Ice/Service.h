// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_SERVICE_H
#define ICE_SERVICE_H

#include <Ice/CommunicatorF.h>
#include <Ice/LoggerF.h>
#include <fstream>

namespace Ice
{

class ICE_API Service
{
public:

    Service();
    virtual ~Service();

    virtual bool start(int, char**) = 0;
    virtual bool shutdown();
    virtual void waitForShutdown();
    virtual bool stop();
    virtual void interrupt();
    virtual void initializeCommunicator(int&, char**);

    int main(int, char*[]);
    void enableInterrupt();
    void disableInterrupt();

    static Service* instance();

protected:

    void syserror(const std::string&);
    void error(const std::string&);
    void warning(const std::string&);
    void trace(const std::string&);

    Ice::CommunicatorPtr _communicator;

#ifdef _WIN32

    bool _win9x; // Are we running on Windows 9x/ME?

private:

    int installService(const std::string&, int, char*[]);
    int uninstallService(const std::string&, int, char*[]);
    int startService(const std::string&, int, char*[]);
    int stopService(const std::string&, int, char*[]);

public:

    void serviceMain(int, char*[]);
    void control(int);

private:

    bool _service;
    SERVICE_STATUS_HANDLE _statusHandle;
    SERVICE_STATUS _status;
    std::vector<std::string> _serviceArgs;
#endif

private:

    std::string _prog;
    Ice::LoggerPtr _logger;
    static Service* _instance;
};

} // End of namespace Ice

#endif
