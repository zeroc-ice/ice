//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <ServiceInstaller.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>

#include <Aclapi.h>
#include <Sddl.h>
#include <authz.h>

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

    if(_serviceType == icegridregistry)
    {
        _icegridInstanceName = _serviceProperties->getPropertyWithDefault("IceGrid.InstanceName", "IceGrid");
        _serviceName = serviceTypeToLowerString(_serviceType) + "." + _icegridInstanceName;
    }
    else
    {
        Ice::LocatorPrx defaultLocator = LocatorPrx::uncheckedCast(
            _communicator->stringToProxy(_serviceProperties->getProperty("Ice.Default.Locator")));
        if(defaultLocator != 0)
        {
            _icegridInstanceName = defaultLocator->ice_getIdentity().category;
        }

        if(_serviceType == icegridnode)
        {
            if(_icegridInstanceName == "")
            {
                throw logic_error("Ice.Default.Locator must be set in " + _configFile);
            }
            _nodeName = _serviceProperties->getProperty("IceGrid.Node.Name");
            if(_nodeName == "")
            {
                throw logic_error("IceGrid.Node.Name must be set in " + _configFile);
            }
            _serviceName = serviceTypeToLowerString(_serviceType) + "." + _icegridInstanceName + "." + _nodeName;
        }
        else if(_serviceType == glacier2router)
        {
            _glacier2InstanceName = _serviceProperties->getPropertyWithDefault("Glacier2.InstanceName", "Glacier2");
            _serviceName = serviceTypeToLowerString(_serviceType) + "." + _glacier2InstanceName;
        }
        else
        {
            throw invalid_argument("Unknown service type");
        }
    }
}

void
IceServiceInstaller::install(const PropertiesPtr& properties)
{
    _debug = properties->getPropertyAsInt("Debug") != 0;

    initializeSid(properties->getPropertyWithDefault("ObjectName", "NT Authority\\LocalService"));

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

    string displayName = properties->getPropertyWithDefault("DisplayName", defaultDisplayName[_serviceType]);
    string description = properties->getPropertyWithDefault("Description", defaultDescription[_serviceType]);
    string imagePath = fixDirSeparator(properties->getPropertyWithDefault("ImagePath",
                                                                          getServiceInstallerPath() + '\\' +
                                                                          serviceTypeToLowerString(_serviceType) +
                                                                          ".exe"));
    if(!IceUtilInternal::fileExists(imagePath))
    {
        throw runtime_error(imagePath + ": not found");
    }
    else
    {
        string imageDir = imagePath;
        imageDir.erase(imageDir.rfind('\\'));
        if(imageDir != "")
        {
            grantPermissions(imageDir.c_str(), SE_FILE_OBJECT, true, GENERIC_READ | GENERIC_EXECUTE);
        }
    }

    string dependency;

    if(_serviceType == icegridregistry)
    {
        if(properties->getPropertyAsInt("DependOnRegistry") != 0)
        {
            throw logic_error("The IceGrid registry service can't depend on itself");
        }

        string registryDataDir = fixDirSeparator(_serviceProperties->getProperty("IceGrid.Registry.LMDB.Path"));
        if(registryDataDir == "")
        {
            throw logic_error("IceGrid.Registry.LMDB.Path must be set in " + _configFile);
        }
        if(!IceUtilInternal::isAbsolutePath(registryDataDir))
        {
            throw logic_error("'" + registryDataDir +
                              "' is a relative path; IceGrid.Registry.LMDB.Path must be an absolute path");
        }

        if(!mkdir(registryDataDir))
        {
            grantPermissions(registryDataDir, SE_FILE_OBJECT, true, FILE_ALL_ACCESS);
        }
    }
    else if(_serviceType == icegridnode)
    {
        string nodeDataDir = fixDirSeparator(_serviceProperties->getProperty("IceGrid.Node.Data"));
        if(nodeDataDir == "")
        {
            throw logic_error("IceGrid.Node.Data must be set in " + _configFile);
        }
        if(!IceUtilInternal::isAbsolutePath(nodeDataDir))
        {
            throw logic_error("'" + nodeDataDir + "' is a relative path; IceGrid.Node.Data must be an absolute path");
        }

        if(!mkdir(nodeDataDir))
        {
            grantPermissions(nodeDataDir, SE_FILE_OBJECT, true, FILE_ALL_ACCESS);
        }

        grantPermissions("MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib", SE_REGISTRY_KEY, true);

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
                throw logic_error("Ice.Default.Locator must be set in " + _configFile + " when DependOnRegistry is not zero");
            }
            dependency = "icegridregistry." + _icegridInstanceName;
        }
    }

    if(!_configFile.find("HKLM\\") == 0)
    {
        grantPermissions(_configFile);
    }

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
        throw runtime_error("Cannot open SCM: " + IceUtilInternal::errorToString(res));
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

    string command = "\"" + imagePath + "\" --service " + _serviceName + " --Ice.Config=\"";
    //
    // Get the full path of config file.
    //
    if(!_configFile.find("HKLM\\") == 0)
    {
        char fullPath[MAX_PATH];
        if(GetFullPathName(_configFile.c_str(), MAX_PATH, fullPath, 0) > MAX_PATH)
        {
            throw runtime_error("Could not compute the full path of " + _configFile);
        }
        command += string(fullPath) + "\"";
    }
    else
    {
        command += _configFile + "\"";
    }

    int autoStartVal = properties->getPropertyAsIntWithDefault("AutoStart", 1);
    bool autoStart = autoStartVal != 0;
    bool delayedAutoStart = autoStartVal == 2;

    string password = properties->getProperty("Password");

    //
    // We don't support to use a string converter with this tool, so don't need to
    // use string converters in calls to stringToWstring.
    //
    SC_HANDLE service = CreateServiceW(
        scm,
        stringToWstring(_serviceName).c_str(),
        stringToWstring(displayName).c_str(),
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        autoStart ? SERVICE_AUTO_START : SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        stringToWstring(command).c_str(),
        0,
        0,
        stringToWstring(deps).c_str(),
        stringToWstring(_sidName).c_str(),
        stringToWstring(password).c_str());

    if(service == 0)
    {
        DWORD res = GetLastError();
        CloseServiceHandle(scm);
        throw runtime_error("Cannot create service" + _serviceName + ": " + IceUtilInternal::errorToString(res));
    }

    //
    // Set description
    //
    wstring uDescription = stringToWstring(description);
    SERVICE_DESCRIPTIONW sd = { const_cast<wchar_t*>(uDescription.c_str()) };

    if(!ChangeServiceConfig2W(service, SERVICE_CONFIG_DESCRIPTION, &sd))
    {
        DWORD res = GetLastError();
        CloseServiceHandle(scm);
        CloseServiceHandle(service);
        throw runtime_error("Cannot set description for service" + _serviceName + ": " + IceUtilInternal::errorToString(res));
    }

    //
    // Set delayed auto-start
    //
    if(delayedAutoStart)
    {
        SERVICE_DELAYED_AUTO_START_INFO info = { true };

        if(!ChangeServiceConfig2W(service, SERVICE_CONFIG_DELAYED_AUTO_START_INFO, &info))
        {
            DWORD res = GetLastError();
            CloseServiceHandle(scm);
            CloseServiceHandle(service);
            throw runtime_error("Cannot set delayed auto start for service" + _serviceName + ": " + IceUtilInternal::errorToString(res));
        }
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
        throw runtime_error("Cannot open SCM: " + IceUtilInternal::errorToString(res));
    }

    //
    // We don't support to use a string converter with this tool, so don't need to
    // use string converters in calls to stringToWstring.
    //
    SC_HANDLE service = OpenServiceW(scm, stringToWstring(_serviceName).c_str(), SERVICE_ALL_ACCESS);
    if(service == 0)
    {
        DWORD res = GetLastError();
        CloseServiceHandle(scm);
        throw runtime_error("Cannot open service '" + _serviceName + "': " + IceUtilInternal::errorToString(res));
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
            throw runtime_error("Cannot stop service '" + _serviceName + "': " + IceUtilInternal::errorToString(res));
        }
    }

    if(!DeleteService(service))
    {
        DWORD res = GetLastError();
        CloseServiceHandle(scm);
        CloseServiceHandle(service);
        throw runtime_error("Cannot delete service '" + _serviceName + "': " + IceUtilInternal::errorToString(res));
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

vector<string>
IceServiceInstaller::getPropertyNames()
{
    static const string propertyNames[] = { "ImagePath", "DisplayName", "ObjectName", "Password",
                                            "Description", "DependOnRegistry", "Debug", "AutoStart", "EventLog" };

    vector<string> result(propertyNames, propertyNames + 9);

    return result;
}

string
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

string
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

string
IceServiceInstaller::getServiceInstallerPath()
{
    wchar_t buffer[MAX_PATH];
    if(!GetModuleFileNameW(0, buffer, MAX_PATH))
    {
        throw runtime_error("Can not get full path to service installer:\n" + IceUtilInternal::lastErrorToString());
    }

    string path = wstringToString(buffer);
    assert(path.find_last_of("/\\"));
    return path.substr(0, path.find_last_of("/\\"));
}

void
IceServiceInstaller::initializeSid(const string& name)
{
    {
        DWORD sidSize = 32;
        _sidBuffer.resize(sidSize);

        DWORD domainNameSize = 32;
        wstring domainName(domainNameSize, wchar_t());

        //
        // We don't support to use a string converter with this tool, so don't need to
        // use string converters in calls to stringToWstring.
        //
        SID_NAME_USE nameUse;
        while(LookupAccountNameW(0, stringToWstring(name).c_str(),
                                 _sidBuffer.data(), &sidSize,
                                 const_cast<wchar_t*>(domainName.data()), &domainNameSize, &nameUse) == false)
        {
            DWORD res = GetLastError();

            if(res == ERROR_INSUFFICIENT_BUFFER)
            {
                _sidBuffer.resize(sidSize);
                domainName.resize(domainNameSize);
                continue;
            }
            throw runtime_error("Could not retrieve Security ID for " + name + ": " +
                                IceUtilInternal::errorToString(res));
        }
        _sid = reinterpret_cast<SID*>(_sidBuffer.data());
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
        wchar_t accountName[1024];
        DWORD accountNameLen = 1024;

        wchar_t domainName[1024];
        DWORD domainLen = 1024;

        SID_NAME_USE nameUse;
        if(LookupAccountSidW(0, _sid, accountName, &accountNameLen, domainName, &domainLen, &nameUse) == false)
        {
            DWORD res = GetLastError();
            throw runtime_error("Could not retrieve full account name for " + name + ": " +
                                IceUtilInternal::errorToString(res));
        }

        _sidName = wstringToString(domainName) + "\\" + wstringToString(accountName);
    }

    if(_debug)
    {
        Trace trace(_communicator->getLogger(), "IceServiceInstaller");
        wchar_t* sidString = 0;
        ConvertSidToStringSidW(_sid, &sidString);
        trace << "SID: " << wstringToString(sidString) << "; ";
        LocalFree(sidString);
        trace << "Full name: " << _sidName;
    }
}

void
IceServiceInstaller::grantPermissions(const string& path, SE_OBJECT_TYPE type, bool inherit, DWORD desiredAccess) const
{
    if(_debug)
    {
        Trace trace(_communicator->getLogger(), "IceServiceInstaller");
        trace << "Granting access on " << path << " to " << _sidName;
    }

    //
    // First retrieve the ACL for our file/directory/key
    //
    PACL acl = 0; // will point to memory in sd
    PACL newAcl = 0;
    PSECURITY_DESCRIPTOR sd = 0;

    AUTHZ_RESOURCE_MANAGER_HANDLE manager = 0;
    AUTHZ_CLIENT_CONTEXT_HANDLE clientContext = 0;

    SECURITY_INFORMATION flags = DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION;

    DWORD res = GetNamedSecurityInfoW(const_cast<wchar_t*>(stringToWstring(path).c_str()), type,
                                      flags, 0, 0, &acl, 0, &sd);
    if(res != ERROR_SUCCESS)
    {
        throw runtime_error("Could not retrieve securify info for " + path + ": " + IceUtilInternal::errorToString(res));
    }

    //
    // Now check if _sid can access this file/dir/key
    //
    try
    {
        if(!AuthzInitializeResourceManager(AUTHZ_RM_FLAG_NO_AUDIT, 0, 0, 0, 0, &manager))
        {
            throw runtime_error("AutzInitializeResourceManager failed: " + IceUtilInternal::lastErrorToString());
        }

        LUID unusedId = { 0 };

        if(!AuthzInitializeContextFromSid(0, _sid, manager, 0, unusedId, 0, &clientContext))
        {
            throw runtime_error("AuthzInitializeContextFromSid failed: " + IceUtilInternal::lastErrorToString());
        }

        AUTHZ_ACCESS_REQUEST accessRequest = { 0 };
        accessRequest.DesiredAccess = MAXIMUM_ALLOWED;
        accessRequest.PrincipalSelfSid = 0;
        accessRequest.ObjectTypeList = 0;
        accessRequest.ObjectTypeListLength = 0;
        accessRequest.OptionalArguments = 0;

        ACCESS_MASK accessMask = 0;
        DWORD accessUnused = 0;
        DWORD accessError = 0;
        AUTHZ_ACCESS_REPLY accessReply = { 0 };
        accessReply.ResultListLength = 1;
        accessReply.GrantedAccessMask = &accessMask;
        accessReply.SaclEvaluationResults = &accessUnused;
        accessReply.Error = &accessError;

        if(!AuthzAccessCheck(0, clientContext, &accessRequest, 0, sd, 0, 0, &accessReply, 0))
        {
            throw runtime_error("AuthzAccessCheck failed: " + IceUtilInternal::lastErrorToString());
        }

        bool done = false;

        if(type == SE_FILE_OBJECT)
        {
            if(desiredAccess == FILE_ALL_ACCESS)
            {
                done = (accessMask & READ_CONTROL) && (accessMask & SYNCHRONIZE) && (accessMask & 0x1F) == 0x1F;
            }
            else
            {
                done = (accessMask & READ_CONTROL) && (accessMask & SYNCHRONIZE);
            }
        }
        else
        {
            done = (accessMask & READ_CONTROL) != 0;
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
            EXPLICIT_ACCESS_W ea = { 0 };
            ea.grfAccessPermissions = (accessMask | desiredAccess);
            ea.grfAccessMode = GRANT_ACCESS;
            if(inherit)
            {
                ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
            }
            else
            {
                ea.grfInheritance = NO_INHERITANCE;
            }

            TRUSTEE_W trustee;
            BuildTrusteeWithSidW(&trustee, _sid);
            ea.Trustee = trustee;

            //
            // Create new ACL
            //
            res = SetEntriesInAclW(1, &ea, acl, &newAcl);
            if(res != ERROR_SUCCESS)
            {
                throw runtime_error("Could not modify ACL for " + path + ": " + IceUtilInternal::errorToString(res));
            }

            res = SetNamedSecurityInfoW(const_cast<wchar_t*>(stringToWstring(path).c_str()), type,
                                        DACL_SECURITY_INFORMATION, 0, 0, newAcl, 0);
            if(res != ERROR_SUCCESS)
            {
                throw runtime_error("Could not grant access to " + _sidName + " on " + path + ": " +
                                    IceUtilInternal::errorToString(res));
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
        AuthzFreeResourceManager(manager);
        AuthzFreeContext(clientContext);
        LocalFree(sd);
        LocalFree(newAcl);
        throw;
    }

    AuthzFreeResourceManager(manager);
    AuthzFreeContext(clientContext);
    LocalFree(sd);
    LocalFree(newAcl);
}

bool
IceServiceInstaller::mkdir(const string& path) const
{
    //
    // We don't support to use a string converter with this tool, so don't need to
    // use string converters in calls to stringToWstring.
    //
    if(CreateDirectoryW(stringToWstring(path).c_str(), 0) == 0)
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
            throw runtime_error("Could not create directory " + path + ": " + IceUtilInternal::errorToString(res));
        }
    }
    else
    {
        grantPermissions(path, SE_FILE_OBJECT, true, FILE_ALL_ACCESS);
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
    //
    // We don't support to use a string converter with this tool, so don't need to
    // use string converters in calls to stringToWstring.
    //
    LONG res = RegCreateKeyExW(HKEY_LOCAL_MACHINE, stringToWstring(createLog(log)).c_str(), 0, L"REG_SZ",
                               REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key, &disposition);

    if(res != ERROR_SUCCESS)
    {
        throw runtime_error("Could not create new Event Log '" + log + "': " + IceUtilInternal::errorToString(res));
    }

    res = RegCloseKey(key);
    if(res != ERROR_SUCCESS)
    {
        throw runtime_error("Could not close registry key handle: " + IceUtilInternal::errorToString(res));
    }
}

void
IceServiceInstaller::removeLog(const string& log) const
{
    //
    // We don't support to use a string converter with this tool, so don't need to
    // use string converters in calls to stringToWstring.
    //
    LONG res = RegDeleteKeyW(HKEY_LOCAL_MACHINE, stringToWstring(createLog(log)).c_str());

    //
    // We get ERROR_ACCESS_DENIED when the log is shared by several sources
    //
    if(res != ERROR_SUCCESS && res != ERROR_ACCESS_DENIED)
    {
        throw runtime_error("Could not remove registry key '" + createLog(log) + "': " + IceUtilInternal::errorToString(res));
    }
}

void
IceServiceInstaller::addSource(const string& source, const string& log, const string& resourceFile) const
{
    //
    // We don't support to use a string converter with this tool, so don't need to
    // use string converters in calls to stringToWstring.
    //
    HKEY key = 0;
    DWORD disposition = 0;
    LONG res = RegCreateKeyExW(HKEY_LOCAL_MACHINE, stringToWstring(createSource(source, log)).c_str(),
                               0, L"REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key, &disposition);
    if(res != ERROR_SUCCESS)
    {
        throw runtime_error("Could not create Event Log source in registry: " + IceUtilInternal::errorToString(res));
    }

    //
    // The event resources are bundled into this DLL, therefore
    // the "EventMessageFile" key should contain the path to this
    // DLL.
    //
    res = RegSetValueExW(key, L"EventMessageFile", 0, REG_EXPAND_SZ,
                         reinterpret_cast<const BYTE*>(stringToWstring(resourceFile).c_str()),
                         static_cast<DWORD>(resourceFile.length() + 1) * sizeof(wchar_t));

    if(res == ERROR_SUCCESS)
    {
        //
        // The "TypesSupported" key indicates the supported event
        // types.
        //
        DWORD typesSupported = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
        res = RegSetValueExW(key, L"TypesSupported", 0, REG_DWORD, reinterpret_cast<BYTE*>(&typesSupported),
                             sizeof(typesSupported));
    }

    if(res != ERROR_SUCCESS)
    {
        RegCloseKey(key);
        throw runtime_error("Could not set registry key: " + IceUtilInternal::errorToString(res));
    }

    res = RegCloseKey(key);
    if(res != ERROR_SUCCESS)
    {
        throw runtime_error("Could not close registry key handle: " + IceUtilInternal::errorToString(res));
    }
}

string
IceServiceInstaller::removeSource(const string& source) const
{
    //
    // Find the source and return the log
    //

    HKEY key = 0;

    LONG res = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\EventLog", 0,
                             KEY_ENUMERATE_SUB_KEYS, &key);

    if(res != ERROR_SUCCESS)
    {
        throw runtime_error("Could not open EventLog key: " + IceUtilInternal::errorToString(res));
    }

    DWORD index = 0;
    do
    {
        DWORD subkeySize = 4096;
        wchar_t subkey[4096];

        res = RegEnumKeyExW(key, index, subkey, &subkeySize, 0, 0, 0, 0);

        if(res == ERROR_SUCCESS)
        {
            //
            // Check if we can delete the source sub-key
            //
            // We don't support to use a string converter with this tool, so don't need to
            // use string converters in calls to stringToWstring.
            //
            LONG delRes = RegDeleteKeyW(HKEY_LOCAL_MACHINE,
                                        stringToWstring(createSource(source,
                                            wstringToString(subkey))).c_str());
            if(delRes == ERROR_SUCCESS)
            {
                res = RegCloseKey(key);
                if(res != ERROR_SUCCESS)
                {
                    throw runtime_error("Could not close registry key handle: " + IceUtilInternal::errorToString(res));
                }
                return wstringToString(subkey);
            }

            ++index;
        }
    } while(res == ERROR_SUCCESS);

    if(res == ERROR_NO_MORE_ITEMS)
    {
        RegCloseKey(key);
        throw runtime_error("Could not locate EventLog with source '" + source + "'");
    }
    else
    {
        RegCloseKey(key);
        throw runtime_error("Error while searching EventLog with source '" + source + "': " + IceUtilInternal::errorToString(res));
    }
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
    int patchVersion = ICE_INT_VERSION % 100;

    ostringstream os;
    os << majorVersion * 10 + minorVersion;
    if(patchVersion >= 60)
    {
        os << 'b' << (patchVersion - 60);
    }
    else if(patchVersion >= 50)
    {
        os << 'a' << (patchVersion - 50);
    }

    const string version = os.str();

    string result = imagePathDir + '\\' + "ice" + version + ".dll";
    if(!IceUtilInternal::fileExists(result))
    {
        result = imagePathDir + '\\' + "ice" +  version + "d.dll";
        if(!IceUtilInternal::fileExists(result))
        {
            throw runtime_error("Could not find Ice DLL");
        }
    }
    return result;
}
