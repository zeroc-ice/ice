// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/ProtocolPluginFacade.h> // Just to get the hostname

#include <IceGrid/PlatformInfo.h>
#include <IceGrid/TraceLevels.h>

#include <IcePatch2/Util.h>

#if defined(_WIN32)
#   include <direct.h> // For _getcwd
#   include <pdhmsg.h> // For PDH_MORE_DATA
#else
#   include <sys/utsname.h>
#   if defined(__APPLE__)
#      include <sys/sysctl.h>
#   elif defined(__sun)
#      include <sys/loadavg.h>
#   elif defined(__hpux)
#      include <sys/pstat.h>
#   elif defined(_AIX)
#      include <nlist.h>
#      include <fcntl.h>
#   endif
#endif

using namespace std;
using namespace IceGrid;

#ifdef _WIN32
namespace IceGrid
{

string
getLocalizedPerfName(const map<string, string>& perfNames, const string& name)
{
    unsigned long idx;
    map<string, string>::const_iterator p = perfNames.find(name);
    if(p == perfNames.end())
    {
	return "";
    }
    istringstream is(p->second);
    is >> idx;
    
    vector<char> localized;
    unsigned long size = 256;
    localized.resize(size);
    while(PdhLookupPerfNameByIndex(0, idx, &localized[0], &size) == PDH_MORE_DATA)
    {
	size += 256;
	localized.resize(size);
    }
    return string(&localized[0]);
}

};

#endif

PlatformInfo::PlatformInfo(const Ice::CommunicatorPtr& communicator, const TraceLevelsPtr& traceLevels) : 
    _traceLevels(traceLevels),
    _hostname(IceInternal::getProtocolPluginFacade(communicator)->getDefaultHost())
{
    //
    // Initialization of the necessary data structures to get the load average.
    //
#if defined(_WIN32)
    //
    // The performance counter query is lazy initialized. We can't
    // initialize it in the constructor because it might be called
    // when IceGrid is started on boot as a Windows service with the
    // Windows service control manager (SCM) locked. The query
    // initialization would fail (hang) because it requires to start
    // the "WMI Windows Adapter" service (which can't be started
    // because the SCM is locked...).
    //
    _query = NULL;
    _counter = NULL;
    _usages1.insert(_usages1.end(), 1 * 60 / 5, 0); // 1 sample every 5 seconds during 1 minutes.
    _usages5.insert(_usages5.end(), 5 * 60 / 5, 0); // 1 sample every 5 seconds during 5 minutes.
    _usages15.insert(_usages15.end(), 15 * 60 / 5, 0); // 1 sample every 5 seconds during 15 minutes.
    _last1Total = 0;
    _last5Total = 0;
    _last15Total = 0;
#elif defined(_AIX)
    struct nlist nl;
    nl.n_name = "avenrun";
    nl.n_value = 0;
    if(knlist(&nl, 1, sizeof(nl)) == 0)
    {
	_kmem = open("/dev/kmem", O_RDONLY);

	//
	// Give up root permissions to minimize security risks, it's
	// only needed to access /dev/kmem.
	//
        setuid(getuid());
        setgid(getgid());
    }
    else
    {
	_kmem = -1;
    }
#endif

    //
    // Get the number of processors.
    //
#if defined(_WIN32)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    _info.nProcessors = sysInfo.dwNumberOfProcessors;
#elif defined(__APPLE__)
    static int ncpu[2] = { CTL_HW, HW_NCPU };
    size_t sz = sizeof(_info.nProcessors);
    if(sysctl(ncpu, 2, &_info.nProcessors, &sz, 0, 0) == -1)
    {
	Ice::SyscallException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
#elif defined(__hpux)
    struct pst_dynamic dynInfo;
    if(pstat_getdynamic(&dynInfo, sizeof(dynInfo), 1, 0) >= 0)
    {
        _info.nProcessors = dynInfo.psd_proc_cnt;
    }
    else
    {
        _info.nProcessors = 1;
    }
#else
    _info.nProcessors = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
#endif

    //
    // Get the rest of the node information.
    //
#ifdef _WIN32
    _info.os = "Windows";
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    unsigned long size = sizeof(hostname);
    if(GetComputerName(hostname, &size))
    {
	_info.hostname = hostname;
    }
    OSVERSIONINFO osInfo;
    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osInfo);
    ostringstream os;
    os << osInfo.dwMajorVersion << "." << osInfo.dwMinorVersion;
    _info.release = os.str();
    _info.version = osInfo.szCSDVersion;
    _info.machine = "x86"; // TODO?
#else
    struct utsname utsinfo;
    uname(&utsinfo);
    _info.os = utsinfo.sysname;
    _info.hostname = utsinfo.nodename;
    _info.release = utsinfo.release;
    _info.version = utsinfo.version;
    _info.machine = utsinfo.machine;
#endif

    _info.dataDir = communicator->getProperties()->getProperty("IceGrid.Node.Data");
    if(!IcePatch2::isAbsolute(_info.dataDir))
    {
#ifdef _WIN32
	char cwd[_MAX_PATH];
	if(_getcwd(cwd, _MAX_PATH) == NULL)
#else
	char cwd[PATH_MAX];
	if(getcwd(cwd, PATH_MAX) == NULL)
#endif
	{
	    throw "cannot get the current directory:\n" + IcePatch2::lastError();
	}
	
	_info.dataDir = string(cwd) + '/' + _info.dataDir;
    }
    if(_info.dataDir[_info.dataDir.length() - 1] == '/')
    {
	_info.dataDir = _info.dataDir.substr(0, _info.dataDir.length() - 1);
    }
}

PlatformInfo::~PlatformInfo()
{
#ifdef _WIN32
    if(_query != NULL)
    {
	PdhCloseQuery(_query);
    }
#elif defined(_AIX)
    if(_kmem > 0)
    {
	close(_kmem);
    }
#endif
}

NodeInfo
PlatformInfo::getNodeInfo() const
{
    return _info;
}

LoadInfo
PlatformInfo::getLoadInfo()
{
    LoadInfo info;
    info.avg1 = -1.0f;
    info.avg5 = -1.0f;
    info.avg15 = -1.0f;

#if defined(_WIN32)
    int usage = 100;
    if(_query == NULL)
    {
	initQuery();
    }
    if(_query != NULL && _counter != NULL && PdhCollectQueryData(_query) == ERROR_SUCCESS)
    {
	DWORD type;
	PDH_FMT_COUNTERVALUE value;
	PdhGetFormattedCounterValue(_counter, PDH_FMT_LONG, &type, &value);
	usage = static_cast<int>(value.longValue);
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

    info.avg1 = static_cast<float>(_last1Total) / _usages1.size() / 100.0f;
    info.avg5 = static_cast<float>(_last5Total) / _usages5.size() / 100.0f;
    info.avg15 = static_cast<float>(_last15Total) / _usages15.size() / 100.0f;
#elif defined(__sun) || defined(__linux) || defined(__APPLE__)
    //
    // We use the load average divided by the number of
    // processors to figure out if the machine is busy or
    // not. The result is capped at 1.0f.
    //
    double loadAvg[3];
    if(getloadavg(loadAvg, 3) != -1)
    {
	info.avg1 = static_cast<float>(loadAvg[0]);
	info.avg5 = static_cast<float>(loadAvg[1]);
	info.avg15 = static_cast<float>(loadAvg[2]);
    }
#elif defined(__hpux)
    struct pst_dynamic dynInfo;
    if(pstat_getdynamic(&dynInfo, sizeof(dynInfo), 1, 0) >= 0)
    {
	info.avg1 = dynInfo.psd_avg_1_min;
	info.avg5 = dynInfo.psd_avg_5_min;
	info.avg15 = dynInfo.psd_avg_15_min;
    }
#elif defined(_AIX)
    if(_kmem > 1)
    {
	long long avenrun[3];
	struct nlist nl;
	nl.n_name = "avenrun";
	nl.n_value = 0;
	if(knlist(&nl, 1, sizeof(nl)) == 0)
	{
	    if(pread(_kmem, avenrun, sizeof(avenrun), nl.n_value) >= sizeof(avenrun))
	    {
		info.avg1 = avenrun[0] / 65536.0f;
		info.avg5 = avenrun[1] / 65536.0f;
		info.avg15 = avenrun[2] / 65536.0f;
	    }
	}
    }
#endif
    return info;
}

std::string
PlatformInfo::getHostname() const
{
    return _hostname;
}

std::string
PlatformInfo::getDataDir() const
{
    return _info.dataDir;
}

#ifdef _WIN32
void
PlatformInfo::initQuery()
{
    //
    // Open the query
    //
    PDH_STATUS err = PdhOpenQuery(0, 0, &_query);
    if(err != ERROR_SUCCESS)
    {
	Ice::SyscallException ex(__FILE__, __LINE__);
	ex.error = err;
	Ice::Warning out(_traceLevels->logger);
	out << "can't open performance data query:\n" << ex;
	return;
    }

    //
    // Load the english perf name table.
    //
    vector<unsigned char> buffer;
    unsigned long size = 32768; 
    buffer.resize(size);
    while(RegQueryValueEx(HKEY_PERFORMANCE_DATA, "Counter 09", 0, 0, &buffer[0], &size) == ERROR_MORE_DATA)
    {
	size += 8192;
	buffer.resize(size);
    }

    map<string, string> perfNames;
    const char* buf = reinterpret_cast<const char*>(&buffer[0]);
    unsigned int i = 0;
    while(i < buffer.size() && buf[i])
    {
	string index(&buf[i]);
	i += static_cast<int>(index.size()) + 1;
	if(i >= buffer.size())
	{
	    break;
	}
	string name(&buf[i]);
	i += static_cast<int>(name.size()) + 1;
	perfNames.insert(make_pair(name, index));
    }

    //
    // Get the localized version of "Processor" and "%Processor Time"
    //
    string proc = getLocalizedPerfName(perfNames, "Processor");
    string proctime = getLocalizedPerfName(perfNames, "% Processor Time");

    //
    // Add the counter
    //
    const string name = "\\" + proc + "(_Total)\\" + proctime;
    err = PdhAddCounter(_query, name.c_str(), 0, &_counter);
    if(err != ERROR_SUCCESS)
    {
	Ice::SyscallException ex(__FILE__, __LINE__);
	ex.error = err;
	Ice::Warning out(_traceLevels->logger);
	out << "can't add performance counter `" << name << "':\n" << ex;
    }
}
#endif
