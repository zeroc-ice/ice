// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/EventLoggerI.h>
#include <Ice/EventLoggerMsg.h>
#include <Ice/LocalException.h>
#include <Ice/Network.h> // For errorToString
#include <IceUtil/StaticMutex.h>

using namespace std;

HMODULE Ice::EventLoggerI::_module = 0;

static IceUtil::StaticMutex outputMutex = ICE_STATIC_MUTEX_INITIALIZER;

Ice::EventLoggerI::EventLoggerI(const string& theAppName) : 
    _source(0)
{
    string appName = theAppName;
    if(appName.empty())
    {
        InitializationException ex(__FILE__, __LINE__);
        ex.reason = "event logger requires a value for Ice.ProgramName";
        throw ex;
    }

    //
    // We first need to ensure that there is a registry entry for this application
    // under HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\Application.
    // The application name cannot contain backslashes.
    //
    string::size_type pos = 0;
    while((pos = appName.find('\\', pos)) != string::npos)
    {
        appName[pos] = '/';
    }
    string key = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + appName;

    HKEY hKey;
    DWORD d;

    //
    // Try to create the key and set the values.
    //
    LONG err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, key.c_str(), 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0,
                         &hKey, &d);
    if(err == ERROR_SUCCESS)
    {
        //
        // Get the filename of this DLL.
        //
        char path[_MAX_PATH];
        assert(_module != 0);
        if(!GetModuleFileName(_module, path, _MAX_PATH))
        {
            RegCloseKey(hKey);
            SyscallException ex(__FILE__, __LINE__);
            ex.error = GetLastError();
            throw ex;
        }
        
        //
        // The event resources are bundled into this DLL, therefore the
        // "EventMessageFile" key should contain the path to this DLL.
        //
        err = RegSetValueEx(hKey, "EventMessageFile", 0, REG_EXPAND_SZ, 
                            reinterpret_cast<unsigned char*>(path), static_cast<DWORD>(strlen(path) + 1));
        if(err != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            SyscallException ex(__FILE__, __LINE__);
            ex.error = err;
            throw ex;
        }
        
        //
        // The "TypesSupported" key indicates the supported event types.
        //
        DWORD typesSupported = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
        err = RegSetValueEx(hKey, "TypesSupported", 0, REG_DWORD, reinterpret_cast<unsigned char*>(&typesSupported),
                            sizeof(typesSupported));
        if(err != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            SyscallException ex(__FILE__, __LINE__);
            ex.error = err;
            throw ex;
        }
        RegCloseKey(hKey);
    }


    //
    // The event source must match the registry key.
    //
    _source = RegisterEventSource(0, appName.c_str());
    if(_source == 0)
    {
        SyscallException ex(__FILE__, __LINE__);
        ex.error = GetLastError();
        throw ex;
    }
}

Ice::EventLoggerI::~EventLoggerI()
{
    assert(_source != 0);
    DeregisterEventSource(_source);
}

void
Ice::EventLoggerI::print(const string& message)
{
    const char* str[1];
    str[0] = message.c_str();
    if(!ReportEvent(_source, EVENTLOG_INFORMATION_TYPE, 0, EVENT_LOGGER_MSG, 0, 1, 0, str, 0))
    {
        IceUtil::StaticMutex::Lock sync(outputMutex);
        cerr << "ReportEvent failed `" << IceInternal::errorToString(GetLastError()) << "':\n" << message << endl;
    }
}

void
Ice::EventLoggerI::trace(const string& category, const string& message)
{
    string s;
    if(!category.empty())
    {
        s = category;
        s.append(": ");
    }
    s.append(message);

    const char* str[1];
    str[0] = s.c_str();
    if(!ReportEvent(_source, EVENTLOG_INFORMATION_TYPE, 0, EVENT_LOGGER_MSG, 0, 1, 0, str, 0))
    {
        IceUtil::StaticMutex::Lock sync(outputMutex);
        cerr << "ReportEvent failed `" << IceInternal::errorToString(GetLastError()) << "':\n" << message << endl;
    }
}

void
Ice::EventLoggerI::warning(const string& message)
{
    const char* str[1];
    str[0] = message.c_str();
    if(!ReportEvent(_source, EVENTLOG_WARNING_TYPE, 0, EVENT_LOGGER_MSG, 0, 1, 0, str, 0))
    {
        IceUtil::StaticMutex::Lock sync(outputMutex);
        cerr << "ReportEvent failed `" << IceInternal::errorToString(GetLastError()) << "':\n" << message << endl;
    }
}

void
Ice::EventLoggerI::error(const string& message)
{
    const char* str[1];
    str[0] = message.c_str();
    if(!ReportEvent(_source, EVENTLOG_ERROR_TYPE, 0, EVENT_LOGGER_MSG, 0, 1, 0, str, 0))
    {
        IceUtil::StaticMutex::Lock sync(outputMutex);
        cerr << "ReportEvent failed `" << IceInternal::errorToString(GetLastError()) << "':\n" << message << endl;
    }
}

void
Ice::EventLoggerI::setModuleHandle(HMODULE module)
{
    _module = module;
}
