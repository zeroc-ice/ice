// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/ProtocolPluginFacade.h> // Just to get the hostname

#include <IceGrid/PlatformInfo.h>
#include <IceGrid/TraceLevels.h>

#if !defined(_WIN32)
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

PlatformInfo::PlatformInfo(const Ice::CommunicatorPtr& communicator, const TraceLevelsPtr& traceLevels) : 
    _traceLevels(traceLevels),
    _hostname(IceInternal::getProtocolPluginFacade(communicator)->getDefaultHost())
{
    //
    // Initialization of the necessary data structures to get the load average.
    //
#if defined(_WIN32)
    PDH_STATUS err = PdhOpenQuery(0, 0, &_query);
    if(err != ERROR_SUCCESS)
    {
        Ice::SyscallException ex(__FILE__, __LINE__);
        ex.error = err;
	Ice::Warning out(_traceLevels->logger);
	out << "can't open performance data query:\n" << ex;
    }
    err = PdhAddCounter(_query, "\\Processor(_Total)\\% Processor Time", 0, &_counter);
    if(err != ERROR_SUCCESS)
    {
        Ice::SyscallException ex(__FILE__, __LINE__);
        ex.error = err;
	Ice::Warning out(_traceLevels->logger);
	out << "can't add performance counter:\n" << ex;
    }
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
    _info.machine = ""; // TODO?
#else
    struct utsname utsinfo;
    uname(&utsinfo);
    _info.os = utsinfo.sysname;
    _info.hostname = utsinfo.nodename;
    _info.release = utsinfo.release;
    _info.version = utsinfo.version;
    _info.machine = utsinfo.machine;
#endif
}

PlatformInfo::~PlatformInfo()
{
#ifdef _WIN32
    PdhCloseQuery(_query);
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
    info.load1 = -1.0f;
    info.load5 = -1.0f;
    info.load15 = -1.0f;

#if defined(_WIN32)
    int usage = 100;
    if(PdhCollectQueryData(_query) == ERROR_SUCCESS)
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

    info.load1 = static_cast<float>(_last1Total) / _usages1.size() / 100.0f;
    info.load5 = static_cast<float>(_last5Total) / _usages5.size() / 100.0f;
    info.load15 = static_cast<float>(_last15Total) / _usages15.size() / 100.0f;
#elif defined(__sun) || defined(__linux) || defined(__APPLE__)
    //
    // We use the load average divided by the number of
    // processors to figure out if the machine is busy or
    // not. The result is capped at 1.0f.
    //
    double loadAvg[3];
    if(getloadavg(loadAvg, 3) != -1)
    {
	info.load1 = static_cast<float>(loadAvg[0]);
	info.load5 = static_cast<float>(loadAvg[1]);
	info.load15 = static_cast<float>(loadAvg[2]);
    }
#elif defined(__hpux)
    struct pst_dynamic dynInfo;
    if(pstat_getdynamic(&dynInfo, sizeof(dynInfo), 1, 0) >= 0)
    {
	info.load1 = dynInfo.psd_avg_1_min;
	info.load5 = dynInfo.psd_avg_5_min;
	info.load15 = dynInfo.psd_avg_15_min;
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
		info.load1 = avenrun[0] / 65536.0f;
		info.load5 = avenrun[1] / 65536.0f;
		info.load15 = avenrun[2] / 65536.0f;
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
