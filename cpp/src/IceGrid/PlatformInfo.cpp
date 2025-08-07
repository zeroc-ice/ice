// Copyright (c) ZeroC, Inc.

#include "PlatformInfo.h"
#include "../Ice/FileUtil.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "Ice/StringUtil.h"
#include "TraceLevels.h"

#include <climits>
#include <fstream>
#include <set>

#if defined(_WIN32)
#    include <pdhmsg.h> // For PDH_MORE_DATA
#else
#    include <sys/utsname.h>
#    include <unistd.h>
#    if defined(__APPLE__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#        include <sys/sysctl.h>
#    endif
#endif

using namespace std;
using namespace IceGrid;

namespace
{
#ifdef _WIN32

    string pdhErrorToString(PDH_STATUS err)
    {
        return IceInternal::errorToStringWithSource(err, GetModuleHandle(TEXT("PDH.DLL")));
    }

    static string getLocalizedPerfName(int idx, const Ice::LoggerPtr& logger)
    {
        vector<char> localized;
        unsigned long size = 256;
        localized.resize(size);
        PDH_STATUS err;
        while ((err = PdhLookupPerfNameByIndex(0, idx, &localized[0], &size)) == PDH_MORE_DATA)
        {
            size += 256;
            localized.resize(size);
        }

        if (err != ERROR_SUCCESS)
        {
            Ice::Warning out(logger);
            out << "Unable to lookup the performance counter name:\n";
            out << pdhErrorToString(err);
            out << "\nThis usually occurs when you do not have sufficient privileges";

            throw Ice::SyscallException{__FILE__, __LINE__, "PdhLookupPerfNameByIndex failed", static_cast<DWORD>(err)};
        }
        return string(&localized[0]);
    }

    typedef BOOL(WINAPI* LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

    int getSocketCount(const Ice::LoggerPtr& logger)
    {
        LPFN_GLPI glpi;
        glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
        if (!glpi)
        {
            Ice::Warning out(logger);
            out << "Unable to figure out the number of process sockets:\n";
            out << "GetLogicalProcessInformation not supported on this OS;";
            return 0;
        }

        vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(1);
        DWORD returnLength = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) * static_cast<int>(buffer.size());
        while (true)
        {
            DWORD rc = glpi(&buffer[0], &returnLength);
            if (!rc)
            {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                    buffer.resize(returnLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) + 1);
                    continue;
                }
                else
                {
                    Ice::Warning out(logger);
                    out << "Unable to figure out the number of process sockets:\n";
                    out << IceInternal::lastErrorToString();
                    return 0;
                }
            }
            buffer.resize(returnLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
            break;
        }

        int socketCount = 0;
        for (vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION>::const_iterator p = buffer.begin(); p != buffer.end(); ++p)
        {
            if (p->Relationship == RelationProcessorPackage)
            {
                socketCount++;
            }
        }
        return socketCount;
    }
#endif
}

namespace IceGrid
{
    RegistryInfo toRegistryInfo(const shared_ptr<InternalReplicaInfo>& replica)
    {
        RegistryInfo info;
        info.name = replica->name;
        info.hostname = replica->hostname;
        return info;
    }

    NodeInfo toNodeInfo(const shared_ptr<InternalNodeInfo>& node)
    {
        NodeInfo info;
        info.name = node->name;
        info.os = node->os;
        info.hostname = node->hostname;
        info.release = node->release;
        info.version = node->version;
        info.machine = node->machine;
        info.nProcessors = node->nProcessors;
        info.dataDir = node->dataDir;
        return info;
    }
}

PlatformInfo::PlatformInfo(
    const string& prefix,
    const shared_ptr<Ice::Communicator>& communicator,
    const shared_ptr<TraceLevels>& traceLevels)
    : _traceLevels(traceLevels)
{
    // Initialization of the necessary data structures to get the load average.
#if defined(_WIN32)
    _terminated = false;
    _usages1.insert(_usages1.end(), 1 * 60 / 5, 0);    // 1 sample every 5 seconds during 1 minutes.
    _usages5.insert(_usages5.end(), 5 * 60 / 5, 0);    // 1 sample every 5 seconds during 5 minutes.
    _usages15.insert(_usages15.end(), 15 * 60 / 5, 0); // 1 sample every 5 seconds during 15 minutes.
    _last1Total = 0;
    _last5Total = 0;
    _last15Total = 0;
#endif

    // Get the number of cores/threads. E.g. a quad-core CPU with 2 threads per core will return 8.
#if defined(_WIN32)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    _nProcessorThreads = sysInfo.dwNumberOfProcessors;
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
    static int ncpu[2] = {CTL_HW, HW_NCPU};
    size_t sz = sizeof(_nProcessorThreads);
    if (sysctl(ncpu, 2, &_nProcessorThreads, &sz, 0, 0) == -1)
    {
        throw Ice::SyscallException(__FILE__, __LINE__);
    }
#else
    _nProcessorThreads = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
#endif

    // Get the rest of the node information.
#ifdef _WIN32
    _os = "Windows";
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    unsigned long hsize = sizeof(hostname);
    if (GetComputerName(hostname, &hsize))
    {
        _hostname = hostname;
    }
    OSVERSIONINFO osInfo;
    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

// GetVersionEx will return the Windows 8 OS version value (6.2) for applications not manifested for Windows 8.1 or
// Windows 10. We read the OS version info from a system file resource and if that fail we just return whatever
// GetVersionEx returns.

// GetVersionEx deprecated in Windows 8.1
#    if defined(_MSC_VER)
#        pragma warning(disable : 4996)
#    endif
    GetVersionEx(&osInfo);
#    if defined(_MSC_VER)
#        pragma warning(default : 4996)
#    endif

    DWORD major = osInfo.dwMajorVersion;
    DWORD minor = osInfo.dwMinorVersion;
    DWORD build = osInfo.dwBuildNumber;

    HMODULE handle = GetModuleHandleW(L"kernel32.dll");
    if (handle)
    {
        wchar_t path[MAX_PATH];
        if (GetModuleFileNameW(handle, path, MAX_PATH))
        {
            DWORD size = GetFileVersionInfoSizeW(path, 0);
            if (size)
            {
                vector<unsigned char> buffer;
                buffer.resize(size);

                if (GetFileVersionInfoW(path, 0, size, &buffer[0]))
                {
                    VS_FIXEDFILEINFO* ffi;
                    unsigned int ffiLen;
                    if (VerQueryValueW(&buffer[0], L"", (LPVOID*)&ffi, &ffiLen))
                    {
                        major = HIWORD(ffi->dwProductVersionMS);
                        minor = LOWORD(ffi->dwProductVersionMS);
                        build = HIWORD(ffi->dwProductVersionLS);
                    }
                }
            }
        }
    }

    ostringstream os;
    os << major << "." << minor;
    _release = os.str();
    os << "." << build;
    _version = os.str();

    switch (sysInfo.wProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_AMD64:
            _machine = "x64";
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            _machine = "IA64";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            _machine = "x86";
            break;
        default:
            _machine = "unknown";
            break;
    };
#else
    struct utsname utsinfo;
    uname(&utsinfo);
    _os = utsinfo.sysname;
    _hostname = utsinfo.nodename;
    _release = utsinfo.release;
    _version = utsinfo.version;
    _machine = utsinfo.machine;
#endif

    auto properties = communicator->getProperties();

    // Try to obtain the number of processor sockets.
    _nProcessorSockets = properties->getIcePropertyAsInt("IceGrid.Node.ProcessorSocketCount");
    if (_nProcessorSockets == 0)
    {
#if defined(_WIN32)
        _nProcessorSockets = getSocketCount(_traceLevels->logger);
#elif defined(__linux__)
        ifstream is("/proc/cpuinfo");
        set<string> ids;

        int nprocessor = 0;
        while (is)
        {
            string line;
            getline(is, line);
            if (line.find("processor") == 0)
            {
                nprocessor++;
            }
            else if (line.find("physical id") == 0)
            {
                nprocessor--;
                ids.insert(line);
            }
        }
        _nProcessorSockets = nprocessor + ids.size();
#else
        // Not supported
        _nProcessorSockets = 1;
#endif
    }

    string endpointsPrefix;
    if (prefix == "IceGrid.Registry")
    {
        _name = properties->getIceProperty("IceGrid.Registry.ReplicaName");
        endpointsPrefix = prefix + ".Client";
    }
    else
    {
        _name = properties->getIceProperty(prefix + ".Name");
        endpointsPrefix = prefix;
    }

    Ice::PropertyDict props = properties->getPropertiesForPrefix(endpointsPrefix);
    auto p = props.find(endpointsPrefix + ".PublishedEndpoints");
    if (p != props.end())
    {
        _endpoints = p->second;
    }
    else
    {
        _endpoints = properties->getProperty(endpointsPrefix + ".Endpoints");
    }

    string cwd;
    if (IceInternal::getcwd(cwd) != 0)
    {
        throw runtime_error("cannot get the current directory:\n" + IceInternal::lastErrorToString());
    }
    _cwd = string(cwd);

    _dataDir = properties->getProperty(prefix + ".Data");
    if (!IceInternal::isAbsolutePath(_dataDir))
    {
        _dataDir = _cwd + '/' + _dataDir;
    }
    if (_dataDir[_dataDir.length() - 1] == '/')
    {
        _dataDir = _dataDir.substr(0, _dataDir.length() - 1);
    }
}

void
PlatformInfo::start()
{
#if defined(_WIN32)
    _updateUtilizationThread = std::thread([this] { runUpdateLoadInfo(); });
#endif
}

void
PlatformInfo::stop()
{
#if defined(_WIN32)
    {
        lock_guard lock(_utilizationMutex);
        _terminated = true;
        _utilizationCondVar.notify_one();
    }
    _updateUtilizationThread.join();
#endif
}

NodeInfo
PlatformInfo::getNodeInfo() const
{
    return toNodeInfo(getInternalNodeInfo());
}

RegistryInfo
PlatformInfo::getRegistryInfo() const
{
    return toRegistryInfo(getInternalReplicaInfo());
}

shared_ptr<InternalNodeInfo>
PlatformInfo::getInternalNodeInfo() const
{
    return make_shared<InternalNodeInfo>(
        _name,
        _os,
        _hostname,
        _release,
        _version,
        _machine,
        _nProcessorThreads,
        _dataDir,
        ICE_SO_VERSION);
}

shared_ptr<InternalReplicaInfo>
PlatformInfo::getInternalReplicaInfo() const
{
    return make_shared<InternalReplicaInfo>(_name, _hostname);
}

LoadInfo
PlatformInfo::getLoadInfo() const
{
    LoadInfo info = {-1.0f, -1.0f, -1.0f};

#if defined(_WIN32)
    lock_guard lock(_utilizationMutex);
    info.avg1 = static_cast<float>(_last1Total) / _usages1.size() / 100.0f;
    info.avg5 = static_cast<float>(_last5Total) / _usages5.size() / 100.0f;
    info.avg15 = static_cast<float>(_last15Total) / _usages15.size() / 100.0f;
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
    // We use the load average divided by the number of processors to figure out if the machine is busy or not. The
    // result is capped at 1.0f.
    double loadAvg[3];
    if (getloadavg(loadAvg, 3) != -1)
    {
        info.avg1 = static_cast<float>(loadAvg[0]);
        info.avg5 = static_cast<float>(loadAvg[1]);
        info.avg15 = static_cast<float>(loadAvg[2]);
    }
#endif
    return info;
}

int
PlatformInfo::getProcessorSocketCount() const
{
    return _nProcessorSockets;
}

std::string
PlatformInfo::getHostname() const
{
    return _hostname;
}

std::string
PlatformInfo::getDataDir() const
{
    return _dataDir;
}

std::string
PlatformInfo::getCwd() const
{
    return _cwd;
}

#ifdef _WIN32
void
PlatformInfo::runUpdateLoadInfo()
{
    // NOTE: We shouldn't initialize the performance counter from the PlatformInfo constructor because it might be
    // called when IceGrid is started on boot as a Windows service with the Windows service control manager (SCM)
    // locked. The query initialization would fail (hang) because it requires to start the "WMI Windows Adapter" service
    // (which can't be started because the SCM is locked...).

    // Open the query.
    HQUERY query;
    PDH_STATUS err = PdhOpenQuery(0, 0, &query);
    if (err != ERROR_SUCCESS)
    {
        Ice::Warning out(_traceLevels->logger);
        out << "Cannot open performance data query:\n" << pdhErrorToString(err);
        return;
    }

    // Add the counter for \\Processor(_Total)\\"%Processor Time". We have to look up the localized names for these.
    // "Processor" is index 238 and "%Processor Time" is index 6. If either lookup fails, close the query system, and
    // we're done.

    string processor;
    string percentProcessorTime;
    try
    {
        processor = getLocalizedPerfName(238, _traceLevels->logger);
        percentProcessorTime = getLocalizedPerfName(6, _traceLevels->logger);
    }
    catch (const Ice::LocalException&)
    {
        // No need to print a warning, it's taken care of by getLocalizedPerfName
        PdhCloseQuery(query);
        return;
    }

    const string name = "\\" + processor + "(_Total)\\" + percentProcessorTime;
    HCOUNTER _counter;
    err = PdhAddCounter(query, name.c_str(), 0, &_counter);
    if (err != ERROR_SUCCESS)
    {
        Ice::Warning out(_traceLevels->logger);
        out << "Cannot add performance counter '" + name + "' (expected ";
        out << "if you have insufficient privileges to monitor performance counters):\n";
        out << pdhErrorToString(err);
        PdhCloseQuery(query);
        return;
    }

    while (true)
    {
        unique_lock lock(_utilizationMutex);
        _utilizationCondVar.wait_for(lock, 5s);
        if (_terminated)
        {
            break;
        }

        int usage = 100;
        err = PdhCollectQueryData(query);
        if (err == ERROR_SUCCESS)
        {
            DWORD type;
            PDH_FMT_COUNTERVALUE value;
            PdhGetFormattedCounterValue(_counter, PDH_FMT_LONG, &type, &value);
            usage = static_cast<int>(value.longValue);
        }
        else
        {
            Ice::Warning out(_traceLevels->logger);
            out << "Could not collect performance counter data:\n" << pdhErrorToString(err);
        }

        _last1Total += usage - _usages1.back();
        _last5Total += usage - _usages5.back();
        _last15Total += usage - _usages15.back();

        _usages1.pop_back();
        _usages5.pop_back();
        _usages15.pop_back();
        _usages1.push_front(usage);
        _usages5.push_front(usage);
        _usages15.push_front(usage);
    }

    PdhCloseQuery(query);
}
#endif
