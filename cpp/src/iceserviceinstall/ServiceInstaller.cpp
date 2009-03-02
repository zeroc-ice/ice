// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#define _WIN32_WINNT 0x0500

#if defined(_MSC_VER) && _MSC_VER >= 1400
#    define _CRT_SECURE_NO_DEPRECATE 1  // C4996 '<C function>' was declared deprecated
#endif

#include <ServiceInstaller.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <Aclapi.h>

#if defined(_MSC_VER) && _MSC_VER >= 1300
//
// The VC6 headers don't include Sddl.h
//
#include <Sddl.h>
#endif

using namespace std;
using namespace Ice;

namespace
{

//
// Replace "/" by "\"
//
inline string
fixDirSeparator(const string& path)
{
    string result = path;
    size_t pos = 0;
    while((pos = result.find('/', pos)) != string::npos)
    {
        result[pos] = '\\';
        pos++;
    }

    return result;
}

}

IceServiceInstaller::IceServiceInstaller(int serviceType, const string& configFile,
                                         const CommunicatorPtr& communicator) :
    _serviceType(serviceType),
    _configFile(fixDirSeparator(configFile)),
    _communicator(communicator),
    _serviceProperties(createProperties()),
    _sid(0),
    _debug(false)
{

    _serviceProperties->load(_configFile);

    //
    // Compute _serviceName
    //
    _defaultLocator = LocatorPrx::uncheckedCast(
        _communicator->stringToProxy(
            _serviceProperties->getProperty("Ice.Default.Locator")));

    if(_serviceType == icegridregistry)
    {
        _icegridInstanceName =
            _serviceProperties->getPropertyWithDefault("IceGrid.InstanceName", "IceGrid");
        _serviceName = serviceTypeToLowerString(_serviceType) + "." + _icegridInstanceName;
    }
    else
    {
        if(_defaultLocator != 0)
        {
            _icegridInstanceName = _defaultLocator->ice_getIdentity().category;
        }

        if(_serviceType == icegridnode)
        {
            if(_icegridInstanceName == "")
            {
                throw "Ice.Default.Locator must be set in " + _configFile;
            }
            _nodeName = _serviceProperties->getProperty("IceGrid.Node.Name");
            if(_nodeName == "")
            {
                throw "IceGrid.Node.Name must be set in " + _configFile;
            }
            _serviceName = serviceTypeToLowerString(_serviceType) + "." + _icegridInstanceName + "." + _nodeName;
        }
        else if(_serviceType == glacier2router)
        {
            _glacier2InstanceName =
                _serviceProperties->getPropertyWithDefault("Glacier2.InstanceName",
                                                              "Glacier2");
            _serviceName = serviceTypeToLowerString(_serviceType) + "." + _glacier2InstanceName;
        }
        else
        {
            throw "Unknown service type";
        }
    }
}


IceServiceInstaller::~IceServiceInstaller()
{
    free(_sid);
}

void
IceServiceInstaller::install(const PropertiesPtr& properties)
{
    _debug = properties->getPropertyAsInt("Debug") != 0;

    initializeSid(properties->getPropertyWithDefault("ObjectName",
                                                     "NT Authority\\LocalService"));

    const string defaultDisplayName[] =
    {
        "IceGrid registry (" + _icegridInstanceName + ")",
        "IceGrid node (" + _nodeName + " within " +  _icegridInstanceName + ")",
        "Glacier2 router (" + _glacier2InstanceName + ")"
    };

    const string defaultDescription[] =
    {
        "Location and deployment service for Ice applications",
        "Starts and monitors Ice servers",
        "Ice Firewall traversal service"
    };

    string displayName = properties->getPropertyWithDefault("DisplayName",
                                                            defaultDisplayName[_serviceType]);

    string description = properties->getPropertyWithDefault("Description",
                                                            defaultDescription[_serviceType]);

    string imagePath = properties->getProperty("ImagePath");

    if(imagePath == "")
    {
        char buffer[MAX_PATH];
        DWORD size = GetModuleFileName(0, buffer, MAX_PATH);
        if(size == 0)
        {
            throw "Can't get full path to self: " + IceUtilInternal::errorToString(GetLastError());
        }
        imagePath = string(buffer, size);
        imagePath.replace(imagePath.rfind('\\'), string::npos, "\\"
                          + serviceTypeToLowerString(_serviceType) + ".exe");
    }
    else
    {
        imagePath = fixDirSeparator(imagePath);
    }
    if(!fileExists(imagePath))
    {
        throw imagePath + ": not found";
    }

    string dependency;

    if(_serviceType == icegridregistry)
    {
        if(properties->getPropertyAsInt("DependOnRegistry") != 0)
        {
            throw "The IceGrid registry service can't depend on itself";
        }

        string registryDataDir = fixDirSeparator(_serviceProperties->getProperty("IceGrid.Registry.Data"));
        if(registryDataDir == "")
        {
            throw "IceGrid.Registry.Data must be set in " + _configFile;
        }
        if(!IceUtilInternal::isAbsolutePath(registryDataDir))
        {
            throw "'" + registryDataDir + "' is a relative path; IceGrid.Registry.Data must be an absolute path";
        }

        if(!mkdir(registryDataDir))
        {
            grantPermissions(registryDataDir, SE_FILE_OBJECT, true, true);
        }
    }
    else if(_serviceType == icegridnode)
    {
        string nodeDataDir = fixDirSeparator(_serviceProperties->getProperty("IceGrid.Node.Data"));
        if(nodeDataDir == "")
        {
            throw "IceGrid.Node.Data must be set in " + _configFile;
        }
        if(!IceUtilInternal::isAbsolutePath(nodeDataDir))
        {
            throw "'" + nodeDataDir + "' is a relative path; IceGrid.Node.Data must be an absolute path";
        }

        if(!mkdir(nodeDataDir))
        {
            grantPermissions(nodeDataDir, SE_FILE_OBJECT, true, true);
        }

        grantPermissions(
            "MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib",
            SE_REGISTRY_KEY, true);

        if(properties->getPropertyAsInt("DependOnRegistry") != 0)
        {
            dependency = "icegridregistry." + _icegridInstanceName;
        }
    }
    else if(_serviceType == glacier2router)
    {
        if(properties->getPropertyAsInt("DependOnRegistry") != 0)
        {
            if(_icegridInstanceName == "")
            {
                throw"Ice.Default.Locator must be set in " + _configFile
                             + " when DependOnRegistry is not zero";
            }
            dependency = "icegridregistry." + _icegridInstanceName;
        }
    }

    grantPermissions(_configFile);

    string eventLog = properties->getProperty("EventLog");
    if(eventLog == "")
    {
        eventLog = "Application";
    }
    else
    {
        addLog(eventLog);
    }

    string eventLogSource = _serviceProperties->getPropertyWithDefault("Ice.EventLog.Source", _serviceName);

    addSource(eventLogSource, eventLog, getIceDLLPath(imagePath));

    SC_HANDLE scm = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if(scm == 0)
    {
        DWORD res = GetLastError();
        throw "Cannot open SCM: " + IceUtilInternal::errorToString(res);
    }

    string deps = dependency;

    if(deps.empty())
    {
        const string candidates[] = { "netprofm",  "Nla" };
        const int candidatesLen = 2;

        for(int i = 0; i < candidatesLen; ++i)
        {
            SC_HANDLE service = OpenService(scm, candidates[i].c_str(), GENERIC_READ);
            if(service != 0)
            {
                deps = candidates[i];
                CloseServiceHandle(service);
                break; // for
            }
        }
    }

    deps += '\0'; // must be double-null terminated

    //
    // Get the full path of config file
    //
    char fullPath[MAX_PATH];
    if(GetFullPathName(_configFile.c_str(), MAX_PATH, fullPath, 0) > MAX_PATH)
    {
        throw "Could not compute the full path of " + _configFile;
    }

    string command = "\"" + imagePath + "\" --service " + _serviceName
        + " --Ice.Config=\"" + fullPath + "\"";

    bool autoStart = properties->getPropertyAsIntWithDefault("AutoStart", 1) != 0;
    string password = properties->getProperty("Password");

    SC_HANDLE service = CreateService(
        scm,
        _serviceName.c_str(),
        displayName.c_str(),
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        autoStart ? SERVICE_AUTO_START : SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        command.c_str(),
        0,
        0,
        deps.c_str(),
        _sidName.c_str(),
        password.c_str());

    if(service == 0)
    {
        DWORD res = GetLastError();
        CloseServiceHandle(scm);
        throw "Cannot create service" + _serviceName + ": " + IceUtilInternal::errorToString(res);
    }

    //
    // Set description
    //

    SERVICE_DESCRIPTION sd = { const_cast<char*>(description.c_str()) };

    if(!ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &sd))
    {
        DWORD res = GetLastError();
        CloseServiceHandle(scm);
        CloseServiceHandle(service);
        throw "Cannot set description for service" + _serviceName + ": " + IceUtilInternal::errorToString(res);
    }

    CloseServiceHandle(scm);
    CloseServiceHandle(service);
}

void
IceServiceInstaller::uninstall()
{
    SC_HANDLE scm = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if(scm == 0)
    {
        DWORD res = GetLastError();
        throw "Cannot open SCM: " + IceUtilInternal::errorToString(res);
    }

    SC_HANDLE service = OpenService(scm, _serviceName.c_str(), SERVICE_ALL_ACCESS);
    if(service == 0)
    {
        DWORD res = GetLastError();
        CloseServiceHandle(scm);
        throw "Cannot open service '" + _serviceName + "': " + IceUtilInternal::errorToString(res);
    }

    //
    // Stop service first
    //
    SERVICE_STATUS serviceStatus;
    if(!ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus))
    {
        DWORD res = GetLastError();
        if(res != ERROR_SERVICE_NOT_ACTIVE)
        {
            CloseServiceHandle(scm);
            CloseServiceHandle(service);
            throw "Cannot stop service '" + _serviceName + "': " + IceUtilInternal::errorToString(res);
        }
    }

    if(!DeleteService(service))
    {
        DWORD res = GetLastError();
        CloseServiceHandle(scm);
        CloseServiceHandle(service);
        throw "Cannot delete service '" + _serviceName + "': " + IceUtilInternal::errorToString(res);
    }

    CloseServiceHandle(scm);
    CloseServiceHandle(service);

    string eventLogSource = _serviceProperties->getPropertyWithDefault("Ice.EventLog.Source", _serviceName);
    string eventLog = removeSource(eventLogSource);

    if(eventLog != "Application")
    {
        removeLog(eventLog);
    }
}

/* static */ vector<string>
IceServiceInstaller::getPropertyNames()
{
    static const string propertyNames[] = { "ImagePath", "DisplayName", "ObjectName", "Password",
                                            "Description", "DependOnRegistry", "Debug", "AutoStart", "EventLog" };

    vector<string> result(propertyNames, propertyNames + 9);

    return result;
}

/*static*/ string
IceServiceInstaller::serviceTypeToString(int serviceType)
{
    static const string serviceTypeArray[] = { "IceGridRegistry", "IceGridNode", "Glacier2Router" };

    if(serviceType >=0 && serviceType < serviceCount)
    {
        return serviceTypeArray[serviceType];
    }
    else
    {
        return "Unknown service";
    }
}

/*static*/ string
IceServiceInstaller::serviceTypeToLowerString(int serviceType)
{
    static const string serviceTypeArray[] = { "icegridregistry", "icegridnode", "glacier2router" };

    if(serviceType >=0 && serviceType < serviceCount)
    {
        return serviceTypeArray[serviceType];
    }
    else
    {
        return "Unknown service";
    }
}

void
IceServiceInstaller::initializeSid(const string& name)
{
    {
        DWORD sidSize = 32;
        _sid = static_cast<SID*>(malloc(sidSize));
        memset(_sid, 0, sidSize);

        DWORD domainNameSize = 32;
        char* domainName = static_cast<char*>(malloc(domainNameSize));
        memset(domainName, 0, domainNameSize);

        SID_NAME_USE nameUse;
        while(LookupAccountName(0, name.c_str(), _sid, &sidSize, domainName, &domainNameSize, &nameUse) == false)
        {
            DWORD res = GetLastError();

            if(res == ERROR_INSUFFICIENT_BUFFER)
            {
                _sid = static_cast<SID*>(realloc(_sid, sidSize));
                memset(_sid, 0, sidSize);
                domainName = static_cast<char*>(realloc(domainName, domainNameSize));
                memset(domainName, 0, domainNameSize);
            }
            else
            {
                free(_sid);
                _sid = 0;
                free(domainName);

                throw "Could not retrieve Security ID for " + name + ": " + IceUtilInternal::errorToString(res);
            }
        }
        free(domainName);
    }

    //
    // Now store in _sidName a 'normalized' name (for the CreateService call)
    //

    if(name.find('\\') != string::npos)
    {
        //
        // Keep this name; otherwise on XP, the localized name
        // ("NT AUTHORITY\LOCAL SERVICE" in English) shows up in the Services
        // snap-in instead of 'Local Service' (which is also a localized name,
        // but looks nicer).
        //
        _sidName = name;
    }
    else
    {
        char accountName[1024];
        DWORD accountNameLen = 1024;

        char domainName[1024];
        DWORD domainLen = 1024;

        SID_NAME_USE nameUse;
        if(LookupAccountSid(0, _sid, accountName, &accountNameLen, domainName,
                            &domainLen, &nameUse) == false)
        {
            DWORD res = GetLastError();
            throw "Could not retrieve full account name for " + name + ": " + IceUtilInternal::errorToString(res);
        }

        _sidName = string(domainName) + "\\" + accountName;
    }

    if(_debug)
    {
        Trace trace(_communicator->getLogger(), "IceServiceInstaller");

#if defined(_MSC_VER) && _MSC_VER >= 1300
	char* sidString = 0;
        ConvertSidToStringSid(_sid, &sidString);
	trace << "SID: " << sidString << "; ";
	LocalFree(sidString);
#endif
	trace << "Full name: " << _sidName;
    }
}

bool
IceServiceInstaller::fileExists(const string& path) const
{
    struct _stat buffer = { 0 };
    int err = _stat(path.c_str(), &buffer);

    if(err == 0)
    {
        if((buffer.st_mode & _S_IFREG) == 0)
        {
            throw path + " is not a regular file";
        }
        return true;
    }
    else
    {
        if(errno == ENOENT)
        {
            return false;
        }
        else
        {
            const char* msg = strerror(errno);
            throw "Problem with " + path + ": " + msg;
        }
    }
}

void
IceServiceInstaller::grantPermissions(const string& path, SE_OBJECT_TYPE type, bool inherit, bool fullControl) const
{
    //
    // First retrieve the ACL for our file/directory/key
    //
    PACL acl = 0;
    PACL newAcl = 0;
    PSECURITY_DESCRIPTOR sd = 0;
    DWORD res = GetNamedSecurityInfo(const_cast<char*>(path.c_str()), type,
                                     DACL_SECURITY_INFORMATION,
                                     0, 0, &acl, 0, &sd);
    if(res != ERROR_SUCCESS)
    {
        throw "Could not retrieve securify info for " + path + ": " + IceUtilInternal::errorToString(res);
    }

    try
    {
        //
        // Now check if _sid can read this file/dir/key
        //
        TRUSTEE trustee;
        BuildTrusteeWithSid(&trustee, _sid);

        ACCESS_MASK accessMask = 0;
        res = GetEffectiveRightsFromAcl(acl, &trustee, &accessMask);

        if(res != ERROR_SUCCESS)
        {
            throw "Could not retrieve effective rights for " + _sidName + " on " + path + ": " + IceUtilInternal::errorToString(res);
        }

        bool done = false;

        if(type == SE_FILE_OBJECT)
        {
            if(fullControl)
            {
                done = (accessMask & READ_CONTROL) && (accessMask & SYNCHRONIZE)
                    && (accessMask & 0x1F) == 0x1F;
            }
            else
            {
                done = (accessMask & READ_CONTROL) && (accessMask & SYNCHRONIZE);
            }
        }
        else
        {
            done = (accessMask & READ_CONTROL);
        }

        if(done)
        {
            if(_debug)
            {
                Trace trace(_communicator->getLogger(), "IceServiceInstaller");
                trace << _sidName << " had already the desired permissions on " << path;
            }
        }
        else
        {
            EXPLICIT_ACCESS ea = { 0 };

            if(type == SE_FILE_OBJECT && fullControl)
            {
                ea.grfAccessPermissions = (accessMask | FILE_ALL_ACCESS);
            }
            else
            {
                ea.grfAccessPermissions = (accessMask | GENERIC_READ);
            }
            ea.grfAccessMode = GRANT_ACCESS;
            if(inherit)
            {
                ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
            }
            else
            {
                ea.grfInheritance = NO_INHERITANCE;
            }
            ea.Trustee = trustee;

            //
            // Create new ACL
            //
            res = SetEntriesInAcl(1, &ea, acl, &newAcl);
            if(res != ERROR_SUCCESS)
            {
                throw "Could not modify ACL for " + path + ": " + IceUtilInternal::errorToString(res);
            }

            res = SetNamedSecurityInfo(const_cast<char*>(path.c_str()), type,
                                       DACL_SECURITY_INFORMATION,
                                       0, 0, newAcl, 0);
            if(res != ERROR_SUCCESS)
            {
                throw "Could not grant access to " + _sidName + " on " + path + ": " + IceUtilInternal::errorToString(res);
            }

            if(_debug)
            {
                Trace trace(_communicator->getLogger(), "IceServiceInstaller");
                trace << "Granted access on " << path << " to " << _sidName;
            }
        }
    }
    catch(...)
    {
        LocalFree(sd);
        LocalFree(newAcl);
        throw;
    }

    LocalFree(sd);
    LocalFree(newAcl);
}

bool
IceServiceInstaller::mkdir(const string& path) const
{
    if(CreateDirectory(path.c_str(), 0) == 0)
    {
        DWORD res = GetLastError();
        if(res == ERROR_ALREADY_EXISTS)
        {
            return false;
        }
        else if(res == ERROR_PATH_NOT_FOUND)
        {
            string parentPath = path;
            parentPath.erase(parentPath.rfind('\\'));
            mkdir(parentPath);
            return mkdir(path);
        }
        else
        {
            throw "Could not create directory " + path + ": " + IceUtilInternal::errorToString(res);
        }
    }
    else
    {
        grantPermissions(path, SE_FILE_OBJECT, true, true);
        return true;
    }
}

void
IceServiceInstaller::addLog(const string& log) const
{
    //
    // Create or open the corresponding registry key
    //

    HKEY key = 0;
    DWORD disposition = 0;
    LONG res = RegCreateKeyEx(HKEY_LOCAL_MACHINE, createLog(log).c_str(),
                              0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0,
                              &key, &disposition);

    if(res != ERROR_SUCCESS)
    {
        throw "Could not create new Event Log '" + log + "': " + IceUtilInternal::errorToString(res);
    }

    res = RegCloseKey(key);
    if(res != ERROR_SUCCESS)
    {
        throw "Could not close registry key handle: " + IceUtilInternal::errorToString(res);
    }
}

void
IceServiceInstaller::removeLog(const string& log) const
{
    LONG res = RegDeleteKey(HKEY_LOCAL_MACHINE, createLog(log).c_str());

    //
    // We get ERROR_ACCESS_DENIED when the log is shared by several sources
    //
    if(res != ERROR_SUCCESS && res != ERROR_ACCESS_DENIED)
    {
        throw "Could not remove registry key '" + createLog(log) + "': " + IceUtilInternal::errorToString(res);
    }
}

void
IceServiceInstaller::addSource(const string& source, const string& log, const string& resourceFile) const
{
    HKEY key = 0;
    DWORD disposition = 0;
    LONG res = RegCreateKeyEx(HKEY_LOCAL_MACHINE, createSource(source, log).c_str(),
                              0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0,
                              &key, &disposition);
    if(res != ERROR_SUCCESS)
    {
        throw "Could not create Event Log source in registry: " + IceUtilInternal::errorToString(res);
    }

    //
    // The event resources are bundled into this DLL, therefore
    // the "EventMessageFile" key should contain the path to this
    // DLL.
    //
    res = RegSetValueEx(key, "EventMessageFile", 0, REG_EXPAND_SZ,
                        reinterpret_cast<const BYTE*>(resourceFile.c_str()),
                        static_cast<DWORD>(resourceFile.length() + 1));

    if(res == ERROR_SUCCESS)
    {
        //
        // The "TypesSupported" key indicates the supported event
        // types.
        //
        DWORD typesSupported = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE
            | EVENTLOG_INFORMATION_TYPE;
        res = RegSetValueEx(key, "TypesSupported", 0, REG_DWORD,
                            reinterpret_cast<BYTE*>(&typesSupported), sizeof(typesSupported));
    }

    if(res != ERROR_SUCCESS)
    {
        RegCloseKey(key);
        throw "Could not set registry key: " + IceUtilInternal::errorToString(res);
    }

    res = RegCloseKey(key);
    if(res != ERROR_SUCCESS)
    {
        throw "Could not close registry key handle: " + IceUtilInternal::errorToString(res);
    }
}

string
IceServiceInstaller::removeSource(const string& source) const
{
    //
    // Find the source and return the log
    //

    HKEY key = 0;

    LONG res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\EventLog", 0,
                            KEY_ENUMERATE_SUB_KEYS, &key);

    if(res != ERROR_SUCCESS)
    {
        throw "Could not open EventLog key: " + IceUtilInternal::errorToString(res);
    }

    DWORD index = 0;
    do
    {
        char subkey[4096];
        DWORD subkeySize = 4096;

        res = RegEnumKeyEx(key, index, subkey, &subkeySize, 0, 0, 0, 0);

        if(res == ERROR_SUCCESS)
        {
            //
            // Check if we can delete the source sub-key
            //
            LONG delRes = RegDeleteKey(HKEY_LOCAL_MACHINE, createSource(source, subkey).c_str());
            if(delRes == ERROR_SUCCESS)
            {
                res = RegCloseKey(key);
                if(res != ERROR_SUCCESS)
                {
                    throw "Could not close registry key handle: " + IceUtilInternal::errorToString(res);
                }
                return string(subkey);
            }

            ++index;
        }
    } while(res == ERROR_SUCCESS);


    if(res == ERROR_NO_MORE_ITEMS)
    {
        RegCloseKey(key);
        throw "Could not locate EventLog with source '" + source + "'";
    }
    else
    {
        RegCloseKey(key);
        throw "Error while searching EventLog with source '" + source + "': " + IceUtilInternal::errorToString(res);
    }

    res = RegCloseKey(key);
    if(res != ERROR_SUCCESS)
    {
        throw "Could not close registry key handle: " + IceUtilInternal::errorToString(res);
    }

    return ""; // To keep compilers happy.
}

string
IceServiceInstaller::mangleSource(const string& name) const
{
    string result = name;
    //
    // The source cannot contain backslashes.
    //
    string::size_type pos = 0;
    while((pos = result.find('\\', pos)) != string::npos)
    {
        result[pos] = '/';
    }
    return result;
}

string
IceServiceInstaller::createLog(const string& log) const
{
    //
    // The registry key is:
    //
    // HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\<log>.
    //
    return "SYSTEM\\CurrentControlSet\\Services\\EventLog\\" + log;
}

string
IceServiceInstaller::createSource(const string& name, const string& log) const
{
    return createLog(log) + "\\" + mangleSource(name);
}

string
IceServiceInstaller::getIceDLLPath(const string& imagePath) const
{
    string imagePathDir = imagePath;
    imagePathDir.erase(imagePathDir.rfind('\\'));

    //
    // Get current 'DLL' version
    //
    int majorVersion = (ICE_INT_VERSION / 10000);
    int minorVersion = (ICE_INT_VERSION / 100) - majorVersion * 100;
    ostringstream os;
    os << majorVersion * 10 + minorVersion;

    int patchVersion = ICE_INT_VERSION % 100;
    if(patchVersion > 50)
    {
        os << 'b';
        if(patchVersion >= 52)
        {
            os << (patchVersion - 50);
        }
    }
    string version = os.str();

    string result = imagePathDir + '\\' + "ice" + version + ".dll";

    if(fileExists(result))
    {
        return result;
    }
    else
    {
        result = imagePathDir + '\\' + "ice" +  version + "d.dll";
        if(fileExists(result))
        {
            return result;
        }
        else
        {
            throw "Could not find Ice DLL";
        }
    }
}
