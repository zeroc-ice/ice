// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ServiceInstaller.h>
#include <Msiquery.h>

//
// Provides the iceServiceInstall and iceServiceUninstall functions,
// to be called from MSI installers
//
// They both read MSI properties with the following format:
// <service>.<name>
// where <service> is one of the service types (see ServiceInstaller.h)
// and <name> is either "ConfigFile" or a property name (as provided
// by the IceServiceInstaller class). If there is no <service>.ConfigFile
// property, the service is skipped.
//
// Errors are logged using a custom MSI logger, that writes to the MSI
// log file
//

using namespace std;
using namespace Ice;

namespace
{

UINT serviceInstall(MSIHANDLE handle, bool install);

}

extern "C"
{

ICE_SERVICE_INSTALLER_API
UINT __stdcall iceServiceInstall(MSIHANDLE handle)
{
    return serviceInstall(handle, true);
}

ICE_SERVICE_INSTALLER_API
UINT __stdcall iceServiceUninstall(MSIHANDLE handle)
{
    return serviceInstall(handle, false);
}

}

namespace
{
//
// Custom MSI logger
//

class MsiLogger : public Logger
{
public:

    MsiLogger(MSIHANDLE);

    virtual void print(const string&);
    virtual void trace(const string&, const string&);
    virtual void warning(const string&);
    virtual void error(const string&);

private:

    void log(const string&, INSTALLMESSAGE);

    MSIHANDLE _handle;
};


MsiLogger::MsiLogger(MSIHANDLE handle) :
    _handle(handle)
{
}

void
MsiLogger::print(const string& msg)
{
    log(msg, INSTALLMESSAGE_INFO);
}

void
MsiLogger::trace(const string& category, const string& msg)
{
    string s = "[ ";
    if(!category.empty())
    {
        s += category + ": ";
    }
    s += msg + " ]";

    string::size_type idx = 0;
    while((idx = s.find("\n", idx)) != string::npos)
    {
        s.insert(idx + 1, "  ");
        ++idx;
    }
    log(s, INSTALLMESSAGE_INFO);
}

void
MsiLogger::warning(const string& msg)
{
    log(msg, INSTALLMESSAGE_WARNING);
}

void
MsiLogger::error(const string& msg)
{
    log(msg, INSTALLMESSAGE_ERROR);
}

void
MsiLogger::log(const string& msg, INSTALLMESSAGE installMessage)
{
    PMSIHANDLE rec = MsiCreateRecord(1); // would 0 be more correct?

    if(rec != 0)
    {
        if(MsiRecordSetString(rec, 0, msg.c_str()) == ERROR_SUCCESS)
        {
            MsiProcessMessage(_handle, installMessage, rec);
        }
    }
}

UINT serviceInstall(MSIHANDLE handle, bool install)
{
    //
    // Create an custom logger
    //
    LoggerPtr msiLogger = new MsiLogger(handle);
    setProcessLogger(msiLogger);
    CommunicatorPtr communicator = 0;

    UINT result = ERROR_SUCCESS;

    try
    {
        communicator = initialize();
        char buffer[4096];
        const DWORD maxBufferSize = 4096;

        UINT res = ERROR_SUCCESS;
        string propName;

        if(install)
        {
            //
            // Let's see if the xxx.ConfigFile property is defined
            //
            const vector<string> propertyNames = IceServiceInstaller::getPropertyNames();

            for(int st = 0; st <  IceServiceInstaller::serviceCount; ++st)
            {
                const string prefix = IceServiceInstaller::serviceTypeToString(st) + ".";
                propName = prefix + "ConfigFile";

                DWORD bufferSize = maxBufferSize;
                res = MsiGetProperty(handle, propName.c_str(), buffer, &bufferSize);

                if(res == ERROR_SUCCESS)
                {
                    if(bufferSize > 0)
                    {
                        IceServiceInstaller installer(st, buffer, communicator);

                        //
                        // Retrieve properties
                        //

                        PropertiesPtr properties = createProperties();

                        for(size_t i = 0; i < propertyNames.size(); ++i)
                        {
                            const string propName = prefix + propertyNames[i];

                            bufferSize = maxBufferSize;
                            res = MsiGetProperty(handle, propName.c_str(), buffer, &bufferSize);

                            if(res != ERROR_SUCCESS)
                            {
                                {
                                    Error error(msiLogger);
                                    error << "Can't read MSI property " << propName;
                                }
                                communicator->destroy();
                                return ERROR_INSTALL_FAILURE;
                            }

                            if(bufferSize > 0)
                            {
                                properties->setProperty(propName, buffer);
                            }
                        }

                        installer.install(properties);
                    }
                }
                else
                {
                    break; // for
                }
            }
        }
        else
        {
            //
            // uninstall in reverse order
            //

            //
            // Let's see if the xxx.ConfigFile property is defined
            //

            for(int st = IceServiceInstaller::serviceCount; st >= 0; --st)
            {
                const string prefix = IceServiceInstaller::serviceTypeToString(st) + ".";
                propName = prefix + "ConfigFile";

                DWORD bufferSize = maxBufferSize;
                res = MsiGetProperty(handle, propName.c_str(), buffer, &bufferSize);

                if(res == ERROR_SUCCESS)
                {
                    if(bufferSize > 0)
                    {
                        IceServiceInstaller installer(st, buffer, communicator);
                        installer.uninstall();
                    }
                }
            }
        }

        if(res != ERROR_SUCCESS)
        {
            //
            // Report error, fail
            //
            {
                Error error(msiLogger);
                error << "Cant' read MSI property " << propName;
            }

            result = ERROR_INSTALL_FAILURE;
        }
    }
    catch(const string& msg)
    {
        Error error(msiLogger);
        error << msg;
    }
    catch(const Exception& ex)
    {
        Error error(msiLogger);
        error << ex;
    }

    if(communicator != 0)
    {
        communicator->destroy();
    }
    return result;
}
}
