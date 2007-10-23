// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.que
//
// **********************************************************************

#define _WIN32_WINNT 0x0500

#if defined(_MSC_VER) && _MSC_VER >= 1400
#    define _CRT_SECURE_NO_DEPRECATE 1  // C4996 '<C function>' was declared deprecated
#endif

#include <Ice/Application.h>
#include <IceUtil/Options.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <Aclapi.h>
#include <AccCtrl.h>
#include <Sddl.h>

using namespace std;
using namespace Ice;

class Install : public Application
{
public:
    virtual int run(int, char*[]);

    Install();
    virtual ~Install();

private:

    void usage() const;

    void initializeSid(const string&);

    bool fileExists(const string&) const;
    void checkService(const string&, string&, string&) const;
    void grantPermissions(const string& path, SE_OBJECT_TYPE type = SE_FILE_OBJECT,
                     bool inherit = false, bool fullControl = false) const;
    bool mkdir(const string&) const;
    string formatMessage(DWORD) const;

    void uninstallService(const string&) const;
    void installService(const string&, const string&, const string&, const string&,
                        const string&, const string&, const string&) const;

    void addEventLogKey(const string&, const string&) const;
    void removeEventLogKey(const string&) const;
    string mangleKey(const string&) const;
    string createEventLogKey(const string&) const;
    string getIceDLLPath(const string&) const;

    string _icegridInstanceName;
    string _nodeName;
    string _glacier2InstanceName;

    SID* _sid;
    string _sidName;

    bool _debug;
};

int
main(int argc, char* argv[])
{
    Install app;
    int status = app.main(argc, argv);

    system("pause");

    return status;
}

int
Install::run(int argc, char* argv[])
{
    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("u", "uninstall");
    opts.addOpt("", "debug");

    const string propNames[] = { "ImagePath", "DisplayName", "ObjectName", "Password",
                                 "Description", "DependOnRegistry" };
    const int propCount = 6;

    for(int i = 0; i < propCount; ++i)
    {
        opts.addOpt("", propNames[i], IceUtil::Options::NeedArg);
    }

    vector<string> commands;
    try
    {
        commands = opts.parse(argc, argv);
    }
    catch(const IceUtil::BadOptException& e)
    {
        cerr << "Error:" << e.reason << endl;
        usage();
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage();
        return EXIT_SUCCESS;
    }
    if(opts.isSet("version"))
    {
        cout << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    _debug = opts.isSet("debug");

    PropertiesPtr properties = communicator()->getProperties();

    for(int i = 0; i < propCount; ++i)
    {
        if(opts.isSet(propNames[i]))
        {
            properties->setProperty(propNames[i], opts.optArg(propNames[i]));
        }
    }

    if(commands.size() != 2)
    {
        usage();
        return EXIT_FAILURE;
    }

    initializeSid(properties->getPropertyWithDefault("ObjectName",
                                                     "NT Authority\\LocalService"));

    string service = commands[0];
    string configFile = commands[1];

    try
    {
        PropertiesPtr serviceProperties = createProperties();
        serviceProperties->load(configFile);

        string serviceName;
        LocatorPrx defaultLocator = LocatorPrx::uncheckedCast(
            communicator()->stringToProxy(
                serviceProperties->getProperty("Ice.Default.Locator")));

        if(service == "icegridregistry")
        {
            _icegridInstanceName =
                serviceProperties->getPropertyWithDefault("IceGrid.InstanceName", "IceGrid");
            serviceName = service + "." + _icegridInstanceName;
        }
        else
        {
            if(defaultLocator != 0)
            {
                _icegridInstanceName = defaultLocator->ice_getIdentity().category;
            }

            if(service == "icegridnode")
            {
                if(_icegridInstanceName == "")
                {
                    throw string("Ice.Default.Locator must be set in " + configFile);
                }
                _nodeName = serviceProperties->getProperty("IceGrid.Node.Name");
                if(_nodeName == "")
                {
                    throw string("IceGrid.Node.Name must be set in " + configFile);
                }
                serviceName = service + "." + _icegridInstanceName + "." + _nodeName;
            }
            else if(service == "glacier2router")
            {
                _glacier2InstanceName =
                    serviceProperties->getPropertyWithDefault("Glacier2.InstanceName",
                                                              "Glacier2");
                serviceName = service + "." + _glacier2InstanceName;
            }
        }

        string displayName;
        string description;
        checkService(service, displayName, description);
        displayName = properties->getPropertyWithDefault("DisplayName", displayName);
        description = properties->getPropertyWithDefault("Description", description);

        if(opts.isSet("uninstall"))
        {
            uninstallService(serviceName);
            return EXIT_SUCCESS;
        }

        string imagePath = properties->getProperty("ImagePath");
        if(imagePath == "")
        {
            char buffer[MAX_PATH];
            DWORD size = GetModuleFileName(0, buffer, MAX_PATH);
            if(size == 0)
            {
                throw "Can't get full path to self: " + formatMessage(GetLastError());
            }
            imagePath = string(buffer, size);
            imagePath.replace(imagePath.rfind('\\'), string::npos, string("\\")
                              + service + ".exe");
        }
        if(!fileExists(imagePath))
        {
            throw imagePath + ": not found";
        }

        string dependencies = "netprofm";

        if(service == "icegridregistry")
        {
            if(properties->getPropertyAsInt("DependOnRegistry") != 0)
            {
                throw string("The IceGrid registry service can't depend on itself");
            }

            string registryDataDir = serviceProperties->getProperty("IceGrid.Registry.Data");
            if(registryDataDir == "")
            {
                throw string("IceGrid.Registry.Data must be set in " + configFile);
            }
            if(!mkdir(registryDataDir))
            {
                grantPermissions(registryDataDir, SE_FILE_OBJECT, true, true);
            }
        }
        else if(service == "icegridnode")
        {
            string nodeDataDir = serviceProperties->getProperty("IceGrid.Node.Data");
            if(nodeDataDir == "")
            {
                throw string("IceGrid.Node.Data must be set in " + configFile);
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
                dependencies = "icegridregistry." + _icegridInstanceName;
            }
        }
        else if(service == "glacier2router")
        {
            if(properties->getPropertyAsInt("DependOnRegistry") != 0)
            {
                if(_icegridInstanceName == "")
                {
                    throw string("Ice.Default.Locator must be set in " + configFile
                                 + " when DependOnRegistry is not zero");
                }
                dependencies = "icegridregistry." + _icegridInstanceName;
            }
        }
        dependencies += '\0'; // must be double-null terminated

        grantPermissions(configFile);

        installService(serviceName, imagePath, configFile, displayName, description,
                       dependencies, properties->getProperty("Password"));
    }
    catch(const string& msg)
    {
        cerr << "Error: " << msg << endl;
        return EXIT_FAILURE;
    }
    catch(const Ice::Exception& ex)
    {
        cerr << "Error: " << ex << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

Install::Install() :
    _sid(0)
{
}

Install::~Install()
{
    free(_sid);
}

void
Install::usage() const
{
    cerr << "Usage: " << appName()
         << " [options] service config-file [property] [property]\n";
    cerr <<
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "-u, --uninstall      Uninstall the Windows service.\n"
        "\n"
        "service must be icegridregistry, icegridnode or glacier2router.\n"
        "\n"
        "config-file          Path to the Ice configuration file for this service.\n"
        "\n"
        "Valid properties:\n"
        "ImagePath            Full path to <service>.exe. The default value is\n"
        "                     <directory of " << appName() << ">\\<service>.exe\n"
        "DisplayName          Display name for the service\n"
        "Description          Description for the service\n"
        "ObjectName           Account used to run the service. Defaults to\n"
        "                     NT Authority\\LocalService\n"
        "Password             Password for ObjectName\n"
        "DependOnRegistry     When non 0, the service depends on the IceGrid registry\n"
        "                     service (the IceGrid registry service name is computed\n"
        "                     using Ice.Default.Locator in <config-file>).\n"
        ;
}

void
Install::initializeSid(const string& name)
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

                throw string("Could not retrieve Security ID for ") + name + ": "
                    + formatMessage(res);
            }
        }
        free(domainName);
    }

    //
    // Now store in _sidName a 'normalized' name (for the CreateService call)
    //
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
            throw string("Could not retrieve 'normalized' account name for ") + name + ": "
                + formatMessage(res);
        }

        _sidName = string(domainName) + "\\" + accountName;
    }

    if(_debug)
    {
        char* sidString = 0;
        ConvertSidToStringSid(_sid, &sidString);

        Trace trace(communicator()->getLogger(), appName());
        trace << "SID: " << sidString << "; Normalized name: " << _sidName;

        LocalFree(sidString);
    }
}

bool
Install::fileExists(const string& path) const
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
            throw string("Problem with ") + path + ": " + msg;
        }
    }
}

void
Install::checkService(const string& service, string& displayName, string& description) const
{
    if(service == "icegridregistry")
    {
        displayName = string("IceGrid registry (") + _icegridInstanceName + ")";
        description = "Location and deployment service for Ice applications";
    }
    else if(service == "icegridnode")
    {
        displayName = string("IceGrid node (") + _nodeName + " within "
            +  _icegridInstanceName + ")";
        description = "Starts and monitors Ice servers";
    }
    else if(service == "glacier2router")
    {
        displayName = string("Glacier2 router (") + _glacier2InstanceName + ")";
        description = "Ice Firewall traversal service";
    }
    else
    {
        throw string("Invalid service");
    }
}

void
Install::grantPermissions(const string& path, SE_OBJECT_TYPE type, bool inherit, bool fullControl) const
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
        throw string("Could not retrieve securify info for ") + path + ": "
            + formatMessage(res);
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
            throw string("Could not retrieve effective rights for ") + _sidName
                + " on " + path + ": " + formatMessage(res);
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
                Trace trace(communicator()->getLogger(), appName());
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
            PACL newAcl = 0;
            res = SetEntriesInAcl(1, &ea, acl, &newAcl);
            if(res != ERROR_SUCCESS)
            {
                throw string("Could not modify ACL for ") + path + ": " + formatMessage(res);
            }

            res = SetNamedSecurityInfo(const_cast<char*>(path.c_str()), type,
                                       DACL_SECURITY_INFORMATION,
                                       0, 0, newAcl, 0);
            if(res != ERROR_SUCCESS)
            {
                throw string("Could not grant access to ") + _sidName
                    + " on " + path + ": " + formatMessage(res);
            }

            if(_debug)
            {
                Trace trace(communicator()->getLogger(), appName());
                trace << "Granted access on " << path << " to " << _sidName;
            }
        }
    }
    catch(...)
    {
        LocalFree(acl);
        LocalFree(newAcl);
        throw;
    }

    LocalFree(acl);
    LocalFree(newAcl);
}

bool
Install::mkdir(const string& path) const
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
            throw "Could not create directory " + path + ": " + formatMessage(res);
        }
    }
    else
    {
        grantPermissions(path, SE_FILE_OBJECT, true, true);
        return true;
    }
}

string
Install::formatMessage(DWORD err) const
{
    ostringstream os;
    char* msgBuf = 0;
    DWORD ok = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                             FORMAT_MESSAGE_FROM_SYSTEM |
                             FORMAT_MESSAGE_IGNORE_INSERTS,
                             0,
                             err,
                             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                             reinterpret_cast<char*>(&msgBuf),
                             0,
                             0);

    if(ok)
    {
        os << msgBuf;
        LocalFree(msgBuf);
    }
    else
    {
        os << "unknown error";
    }

    return os.str();
}

void
Install::uninstallService(const string& serviceName) const
{
    SC_HANDLE scm = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if(scm == 0)
    {
        DWORD res = GetLastError();
        throw string("Cannot open SCM: ") + formatMessage(res);
    }

    SC_HANDLE service = OpenService(scm, serviceName.c_str(), SERVICE_ALL_ACCESS);
    if(service == 0)
    {
        DWORD res = GetLastError();
        CloseServiceHandle(scm);
        throw string("Cannot open service: ") + formatMessage(res);
    }

    if(!DeleteService(service))
    {
        DWORD res = GetLastError();
        CloseServiceHandle(scm);
        CloseServiceHandle(service);
        throw string("Cannot open service: ") + formatMessage(res);
    }

    CloseServiceHandle(scm);
    CloseServiceHandle(service);

    removeEventLogKey(serviceName);
}

void
Install::installService(const string& serviceName, const string& imagePath,
                        const string& configFile, const string& displayName,
                        const string& description, const string& dependencies,
                        const string& password) const
{

    addEventLogKey(serviceName, getIceDLLPath(imagePath));

    SC_HANDLE scm = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if(scm == 0)
    {
        DWORD res = GetLastError();
        throw string("Cannot open SCM: ") + formatMessage(res);
    }

    string command = "\"" + imagePath + "\" --service " + serviceName
        + " --Ice.Config=\"" + configFile + "\"";

    SC_HANDLE service = CreateService(
        scm,
        serviceName.c_str(),
        displayName.c_str(),
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        command.c_str(),
        0,
        0,
        dependencies.c_str(),
        _sidName.c_str(),
        password.c_str());

    if(service == 0)
    {
        DWORD res = GetLastError();
        CloseServiceHandle(scm);
        throw string("Cannot create service: ") + formatMessage(res);
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
        throw string("Cannot set description: ") + formatMessage(res);
    }


    CloseServiceHandle(scm);
    CloseServiceHandle(service);
}

void
Install::addEventLogKey(const string& serviceName, const string& resourceFile) const
{
    HKEY key = 0;
    DWORD disposition = 0;
    LONG res = RegCreateKeyEx(HKEY_LOCAL_MACHINE, createEventLogKey(serviceName).c_str(),
                              0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0,
                              &key, &disposition);
    if(res != ERROR_SUCCESS)
    {
        throw "Could not create Event Log key in registry: " + formatMessage(res);
    }

    //
    // The event resources are bundled into this DLL, therefore
    // the "EventMessageFile" key should contain the path to this
    // DLL.
    //
    res = RegSetValueEx(key, "EventMessageFile", 0, REG_EXPAND_SZ,
                        reinterpret_cast<const BYTE*>(resourceFile.c_str()),
                        resourceFile.length() + 1);

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
        throw "Could not set registry key: " + formatMessage(res);
    }

    res = RegCloseKey(key);
    if(res != ERROR_SUCCESS)
    {
        throw "Could not close registry key handle: " + formatMessage(res);
    }
}

void
Install::removeEventLogKey(const string& serviceName) const
{
    LONG res = RegDeleteKey(HKEY_LOCAL_MACHINE, createEventLogKey(serviceName).c_str());
    if(res != ERROR_SUCCESS)
    {
        throw "Could not remove registry key: " + formatMessage(res);
    }
}

string
Install::mangleKey(const string& name) const
{
    string result = name;
    //
    // The service name cannot contain backslashes.
    //
    string::size_type pos = 0;
    while((pos = result.find('\\', pos)) != string::npos)
    {
        result[pos] = '/';
    }
    return result;
}

string
Install::createEventLogKey(const string& name) const
{
    //
    // The registry key is:
    //
    // HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\Application.
    //
    return "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + mangleKey(name);
}

string
Install::getIceDLLPath(const string& imagePath) const
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
            throw string("Could not find Ice DLL");
        }
    }
}
