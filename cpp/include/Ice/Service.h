// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    virtual bool shutdown();
    virtual void interrupt();

    int main(int, char*[]);
    Ice::CommunicatorPtr communicator() const;

    static Service* instance();

    void handleInterrupt(int);

protected:

    virtual bool start(int, char*[]) = 0;
    virtual void waitForShutdown();
    virtual bool stop();
    virtual Ice::CommunicatorPtr initializeCommunicator(int&, char*[]);

    void enableInterrupt();
    void disableInterrupt();

    void syserror(const std::string&) const;
    void error(const std::string&) const;
    void warning(const std::string&) const;
    void trace(const std::string&) const;

#ifdef _WIN32

    bool win9x() const;

private:

    bool checkService(int, char*[], int&);
    int installService(const std::string&, int, char*[]);
    int uninstallService(const std::string&, int, char*[]);
    int startService(const std::string&, int, char*[]);
    int stopService(const std::string&, int, char*[]);

public:

    void serviceMain(int, char*[]);
    void control(int);

private:

    bool _service;
    bool _win9x; // Are we running on Windows 9x/ME?
    SERVICE_STATUS_HANDLE _statusHandle;
    SERVICE_STATUS _status;
    std::vector<std::string> _serviceArgs;

#else

private:

    bool checkDaemon(int, char*[], int&);

#endif

private:

    std::string _prog;
    Ice::LoggerPtr _logger;
    Ice::CommunicatorPtr _communicator;
    bool _nohup;
    static Service* _instance;
};

} // End of namespace Ice

#endif
