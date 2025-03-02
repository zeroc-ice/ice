// Copyright (c) ZeroC, Inc.

#include "Activator.h"
#include "../Ice/ArgVector.h"
#include "../Ice/FileUtil.h"
#include "Ice/Ice.h"
#include "IceGrid/Admin.h"
#include "Internal.h"
#include "ServerI.h"
#include "TraceLevels.h"
#include "Util.h"

#include <chrono>
#include <stdexcept>
#include <thread>

#include <climits>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef _WIN32
#    include <csignal>
#    include <pwd.h> // for getpwuid
#    include <sys/wait.h>
#    include <unistd.h>
#else
#    ifndef SIGKILL
#        define SIGKILL 9
#    endif
#    ifndef SIGTERM
#        define SIGTERM 15
#    endif
#endif

#if defined(__linux__) || defined(__GLIBC__)
#    include <grp.h> // for setgroups
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

#define ICE_STRING(X) #X

namespace
{
#ifdef _WIN32
    int getSystemErrno() { return GetLastError(); }
#else
    int getSystemErrno() { return errno; }
#endif
}

namespace IceGrid
{
#ifndef _WIN32
    //
    // Helper function for async-signal safe error reporting
    //
    void
    reportChildError(int err, int fd, const char* cannot, const char* name, const shared_ptr<TraceLevels>& traceLevels)
    {
        //
        // Send any errors to the parent process, using the write
        // end of the pipe.
        //
        ostringstream os;
        os << cannot << " '" << name << "'";
        if (err)
        {
            os << ": " << IceInternal::errorToString(err);
        }
        const string msg = os.str();
        ssize_t sz = write(fd, msg.c_str(), msg.size());
        if (sz == -1)
        {
            Ice::Warning out(traceLevels->logger);
            out << "error rerporting child error msg: '" << msg << "'";
        }
        close(fd);

        //
        // _exit instead of exit to avoid interferences with the parent
        // process.
        //
        _exit(EXIT_FAILURE);
    }

#endif

    string signalToString(int signal)
    {
        switch (signal)
        {
#ifndef _WIN32
            case SIGHUP:
            {
                return ICE_STRING(SIGHUP);
            }
            case SIGINT:
            {
                return ICE_STRING(SIGINT);
            }
            case SIGQUIT:
            {
                return ICE_STRING(SIGQUIT);
            }
            case SIGILL:
            {
                return ICE_STRING(SIGILL);
            }
            case SIGTRAP:
            {
                return ICE_STRING(SIGTRAP);
            }
            case SIGABRT:
            {
                return ICE_STRING(SIGABRT);
            }
            case SIGBUS:
            {
                return ICE_STRING(SIGBUS);
            }
            case SIGFPE:
            {
                return ICE_STRING(SIGFPE);
            }
            case SIGUSR1:
            {
                return ICE_STRING(SIGUSR1);
            }
            case SIGSEGV:
            {
                return ICE_STRING(SIGSEGV);
            }
            case SIGPIPE:
            {
                return ICE_STRING(SIGPIPE);
            }
            case SIGALRM:
            {
                return ICE_STRING(SIGALRM);
            }
            case SIGCONT:
            {
                return ICE_STRING(SIGCONT);
            }
            case SIGSTOP:
            {
                return ICE_STRING(SIGSTOP);
            }
#endif
            case SIGKILL:
            {
                return ICE_STRING(SIGKILL);
            }
            case SIGTERM:
            {
                return ICE_STRING(SIGTERM);
            }
            default:
            {
                ostringstream os;
                os << "signal " << signal;
                return os.str();
            }
        }
    }

    int stringToSignal(const string& str)
    {
#ifndef _WIN32
        if (str == ICE_STRING(SIGHUP))
        {
            return SIGHUP;
        }
        else if (str == ICE_STRING(SIGINT))
        {
            return SIGINT;
        }
        else if (str == ICE_STRING(SIGQUIT))
        {
            return SIGQUIT;
        }
        else if (str == ICE_STRING(SIGILL))
        {
            return SIGILL;
        }
        else if (str == ICE_STRING(SIGTRAP))
        {
            return SIGTRAP;
        }
        else if (str == ICE_STRING(SIGABRT))
        {
            return SIGABRT;
        }
        else if (str == ICE_STRING(SIGBUS))
        {
            return SIGBUS;
        }
        else if (str == ICE_STRING(SIGFPE))
        {
            return SIGFPE;
        }
        else if (str == ICE_STRING(SIGUSR1))
        {
            return SIGUSR1;
        }
        else if (str == ICE_STRING(SIGSEGV))
        {
            return SIGSEGV;
        }
        else if (str == ICE_STRING(SIGUSR2))
        {
            return SIGUSR2;
        }
        else if (str == ICE_STRING(SIGPIPE))
        {
            return SIGPIPE;
        }
        else if (str == ICE_STRING(SIGALRM))
        {
            return SIGALRM;
        }
        else if (str == ICE_STRING(SIGCONT))
        {
            return SIGCONT;
        }
        else if (str == ICE_STRING(SIGSTOP))
        {
            return SIGSTOP;
        }
        else
#endif
            if (str == ICE_STRING(SIGKILL))
        {
            return SIGKILL;
        }
        else if (str == ICE_STRING(SIGTERM))
        {
            return SIGTERM;
        }
        else
        {
            if (str != "")
            {
                char* end;
                long int signal = strtol(str.c_str(), &end, 10);
                if (*end == '\0' && signal > 0 && signal < 64)
                {
#ifdef _WIN32
                    if (signal == SIGKILL || signal == SIGTERM)
                    {
                        return static_cast<int>(signal);
                    }
#else
                    return static_cast<int>(signal);
#endif
                }
            }
            throw BadSignalException("unknown signal '" + str + "'");
        }
    }

#ifdef _WIN32
    struct UnicodeStringLess
    {
        bool operator()(const wstring& lhs, const wstring& rhs) const
        {
            int r = CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, lhs.c_str(), -1, rhs.c_str(), -1);
            assert(r > 0);
            return r == CSTR_LESS_THAN;
        }
    };
#endif
}

#ifdef _WIN32
extern "C" void CALLBACK
activatorWaitCallback(PVOID data, BOOLEAN)
{
    Activator::Process* process = reinterpret_cast<Activator::Process*>(data);
    process->activator->processTerminated(process);
}
#endif

Activator::Activator(const shared_ptr<TraceLevels>& traceLevels) : _traceLevels(traceLevels)
{
#ifdef _WIN32
    _hIntr = CreateEvent(
        nullptr, // Security attributes
        TRUE,    // Manual reset
        FALSE,   // Initial state is nonsignaled
        nullptr  // Unnamed
    );

    if (_hIntr == nullptr)
    {
        throw SyscallException{__FILE__, __LINE__, "CreateEvent failed", GetLastError()};
    }
#else
    int fds[2];
    if (pipe(fds) != 0)
    {
        throw SyscallException{__FILE__, __LINE__, "pipe failed", errno};
    }
    _fdIntrRead = fds[0];
    _fdIntrWrite = fds[1];
    int flags = fcntl(_fdIntrRead, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(_fdIntrRead, F_SETFL, flags);
#endif
}

Activator::~Activator()
{
#ifdef _WIN32
    if (_hIntr != nullptr)
    {
        CloseHandle(_hIntr);
    }
#else
    close(_fdIntrRead);
    close(_fdIntrWrite);
#endif
}

int
Activator::activate(
    const string& name,
    const string& exePath,
    const string& pwdPath,
#ifndef _WIN32
    uid_t uid,
    gid_t gid,
#endif
    const Ice::StringSeq& options,
    const Ice::StringSeq& envs,
    const shared_ptr<ServerI>& server)
{
    lock_guard lock(_mutex);

    if (_deactivating)
    {
        throw runtime_error("The node is being shutdown.");
    }

    string path{exePath}; // NOLINT(performance-unnecessary-copy-initialization)
    if (path.empty())
    {
        throw invalid_argument("The server executable path is empty.");
    }

    string pwd = simplify(pwdPath);
#ifdef _WIN32
    if (!IceInternal::isAbsolutePath(path))
    {
        if (path.find('/') == string::npos)
        {
            //
            // Get the absolute pathname of the executable.
            //
            wchar_t absbuf[_MAX_PATH];
            wchar_t* fPart;
            wstring ext = path.size() <= 4 || path[path.size() - 4] != '.' ? L".exe" : L"";

            //
            // IceGrid doesn't support to use string converters, so don't need to use
            // any string converter in wstringToString conversions.
            //
            if (SearchPathW(nullptr, stringToWstring(path).c_str(), ext.c_str(), _MAX_PATH, absbuf, &fPart) == 0)
            {
                if (_traceLevels->activator > 0)
                {
                    Trace out(_traceLevels->logger, _traceLevels->activatorCat);
                    out << "couldn't find '" << path << "' executable.";
                }
                throw runtime_error("Couldn't find '" + path + "' executable.");
            }
            path = wstringToString(absbuf);
        }
        else if (!pwd.empty())
        {
            path = pwd + "/" + path;
        }
    }

    //
    // Get the absolute pathname of the working directory.
    //
    // IceGrid doesn't support to use string converters, so
    // don't need to use any string converter in stringToWstring
    // conversions.
    //
    if (!pwd.empty())
    {
        wchar_t absbuf[_MAX_PATH];
        if (_wfullpath(absbuf, stringToWstring(pwd).c_str(), _MAX_PATH) == nullptr)
        {
            if (_traceLevels->activator > 0)
            {
                Trace out(_traceLevels->logger, _traceLevels->activatorCat);
                out << "cannot convert '" << pwd << "' into an absolute path";
            }
            throw runtime_error(
                "The server working directory path '" + pwd + "' can't be converted into an absolute path.");
        }
        pwd = wstringToString(absbuf);
    }
#endif

    //
    // Setup arguments.
    //
    StringSeq args;
    args.push_back(path);
    args.insert(args.end(), options.begin(), options.end());

    if (_traceLevels->activator > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
        out << "activating server '" << name << "'";
        if (_traceLevels->activator > 1)
        {
            out << "\n";
            out << "path = " << path << "\n";
            if (pwd.empty())
            {
                string cwd;
                if (IceInternal::getcwd(cwd) == 0)
                {
                    out << "pwd = " << cwd << "\n";
                }
            }
            else
            {
                out << "pwd = " << pwd << "\n";
            }
#ifndef _WIN32
            out << "uid/gid = " << uid << "/" << gid << "\n";
#endif
            if (!envs.empty())
            {
                out << "envs = " << toString(envs, ", ") << "\n";
            }
            if (!args.empty())
            {
                out << "args = " << toString(args);
            }
        }
    }

    //
    // Activate and create.
    //
#ifdef _WIN32

    //
    // Compose command line.
    //
    string cmd;
    for (StringSeq::const_iterator p = args.begin(); p != args.end(); ++p)
    {
        if (p != args.begin())
        {
            cmd.push_back(' ');
        }
        //
        // Enclose arguments containing spaces in double quotes.
        //
        if ((*p).find(' ') != string::npos)
        {
            cmd.push_back('"');
            cmd.append(*p);
            cmd.push_back('"');
        }
        else
        {
            cmd.append(*p);
        }
    }

    //
    // IceGrid doesn't support to use string converters, so don't need to use
    // any string converter in stringToWstring conversions.
    //
    wstring wpwd = stringToWstring(pwd);
    const wchar_t* dir = !wpwd.empty() ? wpwd.c_str() : nullptr;

    //
    // Make a copy of the command line.
    //
    wchar_t* cmdbuf = _wcsdup(stringToWstring(cmd).c_str());

    //
    // Create the environment block for the child process. We start with the environment
    // of this process, and then merge environment variables from the server description.
    // Since Windows is case insensitive wrt environment variables we convert the keys to
    // uppercase to ensure matches are found.
    //
    const wchar_t* env = nullptr;
    wstring envbuf;
    if (!envs.empty())
    {
        map<wstring, wstring, UnicodeStringLess> envMap;
        LPVOID parentEnv = GetEnvironmentStringsW();
        const wchar_t* var = reinterpret_cast<const wchar_t*>(parentEnv);
        if (*var == L'=')
        {
            //
            // The environment block may start with some information about the
            // current drive and working directory. This is indicated by a leading
            // '=' character, so we skip to the first '\0' byte.
            //
            while (*var != L'\0')
                var++;
            var++;
        }
        while (*var != L'\0')
        {
            wstring s(var);
            wstring::size_type pos = s.find(L'=');
            if (pos != wstring::npos)
            {
                envMap[s.substr(0, pos)] = s.substr(pos + 1);
            }
            var += s.size();
            var++; // Skip the '\0' byte
        }
        FreeEnvironmentStringsW(static_cast<wchar_t*>(parentEnv));
        for (StringSeq::const_iterator p = envs.begin(); p != envs.end(); ++p)
        {
            //
            // IceGrid doesn't support to use string converters, so don't need to use
            // any string converter in stringToWstring conversions.
            //
            wstring s = stringToWstring(*p);
            wstring::size_type pos = s.find(L'=');
            if (pos != wstring::npos)
            {
                envMap[s.substr(0, pos)] = s.substr(pos + 1);
            }
        }

        for (map<wstring, wstring, UnicodeStringLess>::const_iterator q = envMap.begin(); q != envMap.end(); ++q)
        {
            envbuf.append(q->first);
            envbuf.push_back(L'=');
            envbuf.append(q->second);
            envbuf.push_back(L'\0');
        }
        envbuf.push_back(L'\0');
        env = envbuf.c_str();
    }

    Process process;

    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    BOOL b = CreateProcessW(
        nullptr,                                               // Executable
        cmdbuf,                                                // Command line
        nullptr,                                               // Process attributes
        nullptr,                                               // Thread attributes
        FALSE,                                                 // Do NOT inherit handles
        CREATE_NEW_PROCESS_GROUP | CREATE_UNICODE_ENVIRONMENT, // Process creation flags
        (LPVOID)env,                                           // Process environment
        dir,                                                   // Current directory
        &si,                                                   // Startup info
        &pi                                                    // Process info
    );

    free(cmdbuf);

    if (!b)
    {
        string message = IceInternal::lastErrorToString();

        Ice::Warning out(_traceLevels->logger);
        out << "server activation failed for '" << name << "':\n" << message;

        throw runtime_error(message);
    }

    //
    // Caller is responsible for closing handles in PROCESS_INFORMATION. We don't need to
    // keep the thread handle, so we close it now. The process handle will be closed later.
    //
    CloseHandle(pi.hThread);
    process.activator = this;
    process.pid = pi.dwProcessId;
    process.hnd = pi.hProcess;
    process.server = server;
    map<string, Process>::iterator it = _processes.insert(make_pair(name, process)).first;

    Process* pp = &it->second;
    if (!RegisterWaitForSingleObject(
            &pp->waithnd,
            pp->hnd,
            activatorWaitCallback,
            pp,
            INFINITE,
            WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE))
    {
        TerminateProcess(pp->hnd, 0);

        string message = IceInternal::lastErrorToString();

        Ice::Warning out(_traceLevels->logger);
        out << "server activation failed for '" << name << "':\ncouldn't register wait callback\n" << message;

        throw runtime_error(message);
    }

    //
    // Don't print the following trace, this might interfer with the
    // output of the started process if it fails with an error message.
    //
    //     if(_traceLevels->activator > 0)
    //     {
    //         Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
    //         out << "activated server '" << name << "' (pid = " << pi.dwProcessId << ")";
    //     }

    return static_cast<int32_t>(process.pid);
#else
    struct passwd pwbuf;
    vector<char> buffer(4096); // 4KB initial buffer size
    struct passwd* pw;

    int err;
    while ((err = getpwuid_r(uid, &pwbuf, &buffer[0], buffer.size(), &pw)) == ERANGE &&
           buffer.size() < 1024 * 1024) // Limit buffer to 1M
    {
        buffer.resize(buffer.size() * 2);
    }

    if (err != 0)
    {
        throw SyscallException{__FILE__, __LINE__, "getpwuid_r failed", err};
    }
    else if (pw == nullptr)
    {
        ostringstream os;
        os << uid;
        throw runtime_error("unknown user id '" + os.str() + "'");
    }

    vector<gid_t> groups;
    groups.resize(20);
    int ngroups = static_cast<int>(groups.size());
#    if defined(__APPLE__)
    if (getgrouplist(pw->pw_name, static_cast<int>(gid), reinterpret_cast<int*>(&groups[0]), &ngroups) < 0)
#    else
    if (getgrouplist(pw->pw_name, gid, &groups[0], &ngroups) < 0)
#    endif
    {
        groups.resize(static_cast<size_t>(ngroups));
#    if defined(__APPLE__)
        getgrouplist(pw->pw_name, static_cast<int>(gid), reinterpret_cast<int*>(&groups[0]), &ngroups);
#    else
        getgrouplist(pw->pw_name, gid, &groups[0], &ngroups);
#    endif
    }
    else
    {
        groups.resize(static_cast<size_t>(ngroups));
    }

    if (groups.size() > NGROUPS_MAX)
    {
        groups.resize(NGROUPS_MAX);
    }

    int fds[2];
    if (pipe(fds) != 0)
    {
        throw SyscallException{__FILE__, __LINE__, "pipe failed", errno};
    }

    int errorFds[2];
    if (pipe(errorFds) != 0)
    {
        throw SyscallException{__FILE__, __LINE__, "pipe failed", errno};
    }

    //
    // Convert to standard argc/argv.
    //
    IceInternal::ArgVector av(args);
    IceInternal::ArgVector env(envs);

    //
    // Current directory
    //
    const char* pwdCStr = pwd.c_str();

    pid_t pid = fork();
    if (pid == -1)
    {
        throw SyscallException{__FILE__, __LINE__, "fork failed", errno};
    }

    if (pid == 0) // Child process.
    {
        //
        // Until exec, we can only use async-signal safe functions
        //

        //
        // Unblock signals blocked by Ice::CtrlCHandler.
        //
        sigset_t sigs;
        sigemptyset(&sigs);
        sigaddset(&sigs, SIGHUP);
        sigaddset(&sigs, SIGINT);
        sigaddset(&sigs, SIGTERM);
        sigprocmask(SIG_UNBLOCK, &sigs, nullptr);

        //
        // Change the uid/gid under which the process will run.
        //
        if (setgid(gid) == -1)
        {
            ostringstream os;
            os << gid;
            reportChildError(
                getSystemErrno(),
                errorFds[1],
                "cannot set process group id",
                os.str().c_str(),
                _traceLevels);
        }

        //
        // Don't initialize supplementary groups if we are not running as root.
        //
        if (getuid() == 0 && setgroups(static_cast<int>(groups.size()), &groups[0]) == -1)
        {
            ostringstream os;
            for (auto p = groups.begin(); p != groups.end(); ++p)
            {
                os << *p;
                if (p + 1 != groups.end())
                {
                    os << ", ";
                }
            }
            reportChildError(
                getSystemErrno(),
                errorFds[1],
                "cannot set process supplementary groups",
                os.str().c_str(),
                _traceLevels);
        }

        if (setuid(uid) == -1)
        {
            ostringstream os;
            os << uid;
            reportChildError(
                getSystemErrno(),
                errorFds[1],
                "cannot set process user id",
                os.str().c_str(),
                _traceLevels);
        }

        //
        // Assign a new process group for this process.
        //
        setpgid(0, 0);

        //
        // Close all file descriptors, except for standard input,
        // standard output, standard error, and the write side
        // of the newly created pipe.
        //
        int maxFd = static_cast<int>(sysconf(_SC_OPEN_MAX));
        if (maxFd <= 0)
        {
            maxFd = numeric_limits<int>::max();
        }

        for (int fd = 3; fd < maxFd; ++fd)
        {
            if (fd != fds[1] && fd != errorFds[1])
            {
                close(fd);
            }
        }

        for (int i = 0; i < env.argc; i++) // NOLINT(clang-analyzer-unix.Malloc)
        {
            //
            // Each env is leaked on purpose ... see man putenv().
            //
            if (putenv(strdup(env.argv[i])) != 0)
            {
                reportChildError(errno, errorFds[1], "cannot set environment variable", env.argv[i], _traceLevels);
            }
        }

        //
        // Change working directory.
        //
        if (strlen(pwdCStr) != 0)
        {
            if (chdir(pwdCStr) == -1)
            {
                reportChildError(errno, errorFds[1], "cannot change working directory to", pwdCStr, _traceLevels);
            }
        }

        //
        // Close on exec the error message file descriptor.
        //
        int flags = fcntl(errorFds[1], F_GETFD);
        flags |= 1; // FD_CLOEXEC
        if (fcntl(errorFds[1], F_SETFD, flags) == -1)
        {
            close(errorFds[1]);
            errorFds[1] = -1;
        }

        if (execvp(av.argv[0], av.argv) == -1)
        {
            if (errorFds[1] != -1)
            {
                reportChildError(errno, errorFds[1], "cannot execute", av.argv[0], _traceLevels);
            }
            else
            {
                reportChildError(errno, fds[1], "cannot execute", av.argv[0], _traceLevels);
            }
        }
    }
    else // Parent process.
    {
        close(fds[1]);
        close(errorFds[1]);

        //
        // Read a potential error message over the error message pipe.
        //
        char s[16];
        ssize_t rs;
        string message;

        // We keep the Activator mutex locked for the whole activate.
        while ((rs = read(errorFds[0], &s, 16)) > 0) // NOLINT(clang-analyzer-unix.BlockInCriticalSection)
        {
            message.append(s, static_cast<size_t>(rs));
        }

        //
        // If an error occurred before the exec() we do some cleanup and throw.
        //
        if (!message.empty())
        {
            Ice::Warning out(_traceLevels->logger);
            out << "server activation failed for '" << name << "':\n" << message;

            close(fds[0]);
            close(errorFds[0]);
            waitPid(pid);
            throw runtime_error(message);
        }

        //
        // Otherwise, the exec() was successful and we don't need the error message
        // pipe anymore.
        //
        close(errorFds[0]);

        Process process;
        process.pid = pid;
        process.pipeFd = fds[0];
        process.server = server;
        _processes.insert(make_pair(name, process));

        int flags = fcntl(process.pipeFd, F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(process.pipeFd, F_SETFL, flags);

        setInterrupt();

        //
        // Don't print the following trace, this might interfere with the
        // output of the started process if it fails with an error message.
        //
        //      if(_traceLevels->activator > 0)
        //      {
        //          Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
        //          out << "activated server '" << name << "' (pid = " << pid << ")";
        //      }
    }

    return pid;
#endif
}

void
Activator::deactivate(const string& name, const optional<Ice::ProcessPrx>& process)
{
#ifdef _WIN32
    int32_t pid = getServerPid(name);
    if (pid == 0)
    {
        //
        // Server is already deactivated.
        //
        return;
    }
#endif

    //
    // Try to shut down the server gracefully using the process proxy.
    //
    if (process)
    {
        if (_traceLevels->activator > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
            out << "deactivating '" << name << "' using process proxy '" << process << "'";
        }

        process->shutdownAsync(
            nullptr,
            [self = shared_from_this(), name](exception_ptr ex)
            {
                try
                {
                    rethrow_exception(ex);
                }
                catch (const std::exception& e)
                {
                    Ice::Warning out(self->_traceLevels->logger);
                    out << "exception occurred while deactivating '" << name << "' using process proxy:\n" << e;
                }

                //
                // Send a SIGTERM to the process.
                //
                self->sendSignal(name, SIGTERM);
            });
        return;
    }

    if (_traceLevels->activator > 1)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
        out << "no process proxy, deactivating '" << name << "' using signal";
    }

    //
    // Send a SIGTERM to the process.
    //
    sendSignal(name, SIGTERM);
}

void
Activator::kill(const string& name)
{
    sendSignal(name, SIGKILL);
}

void
Activator::sendSignal(const string& name, const string& signal)
{
    sendSignal(name, stringToSignal(signal));
}

void
Activator::sendSignal(const string& name, int signal)
{
    int32_t pid = getServerPid(name);
    if (pid == 0)
    {
        //
        // Server is already deactivated.
        //
        return;
    }

#ifdef _WIN32
    if (signal == SIGTERM)
    {
        //
        // Generate a Ctrl+Break event on the child.
        //
        if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pid))
        {
            if (_traceLevels->activator > 1)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
                out << "sent Ctrl+Break to server '" << name << "' (pid = " << pid << ")";
            }
        }
        else if (GetLastError() != ERROR_INVALID_PARAMETER) // Process with pid doesn't exist anymore.
        {
            throw SyscallException{
                __FILE__,
                __LINE__,
                "GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT) failed",
                GetLastError()};
        }
    }
    else if (signal == SIGKILL)
    {
        HANDLE hnd = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hnd == nullptr)
        {
            throw SyscallException{__FILE__, __LINE__, "OpenProcess(PROCESS_TERMINATE) failed", GetLastError()};
        }

        TerminateProcess(hnd, 0); // We use 0 for the exit code to make sure it's not considered as a crash.

        CloseHandle(hnd);

        if (_traceLevels->activator > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
            out << "terminated server '" << name << "' (pid = " << pid << ")";
        }
    }
    else
    {
        throw BadSignalException("signal not supported on Windows");
    }
#else
    int ret = ::kill(static_cast<pid_t>(pid), signal);
    if (ret != 0 && errno != ESRCH)
    {
        throw SyscallException{__FILE__, __LINE__, "kill failed", errno};
    }

    if (_traceLevels->activator > 1)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
        out << "sent " << signalToString(signal) << " to server '" << name << "' (pid = " << pid << ")";
    }
#endif
}

int
Activator::getServerPid(const string& name)
{
    lock_guard lock(_mutex);

    auto p = _processes.find(name);
    if (p == _processes.end())
    {
        return 0;
    }

    return static_cast<int>(p->second.pid);
}

void
Activator::start()
{
    //
    // Create and start the termination listener thread.
    //
    _thread = thread([this] { runTerminationListener(); });
}

void
Activator::waitForShutdown()
{
    unique_lock lock(_mutex);
    _condVar.wait(lock, [this] { return _deactivating; });
}

void
Activator::shutdown()
{
    lock_guard lock(_mutex);
    if (_deactivating)
    {
        return;
    }

    //
    // Deactivation has been initiated. Set _deactivating to true to
    // prevent activation of new processes. This will also cause the
    // termination listener thread to stop when there are no more
    // active processes.
    //
    _deactivating = true;
    setInterrupt();
    _condVar.notify_all();
}

void
Activator::destroy()
{
    map<string, Process> processes;
    {
        lock_guard lock(_mutex);
        assert(_deactivating);
        processes = _processes;
    }

    //
    // Stop all active processes.
    //
    for (const auto& [name, proc] : processes)
    {
        //
        // Stop the server. The listener thread should detect the
        // process deactivation and remove it from the activator's
        // list of active processes.
        //
        try
        {
            proc.server->stopAsync(nullptr, nullptr, Ice::emptyCurrent);
        }
        catch (const ServerStopException&)
        {
            // Server already stopped or destroyed.
        }
        catch (const ObjectNotExistException&)
        {
            //
            // Expected if the server was in the process of being destroyed.
            //
        }
        catch (const Ice::LocalException& ex)
        {
            Ice::Warning out(_traceLevels->logger);
            out << "unexpected exception raised by server '" << name << "' stop:\n" << ex;
        }
    }

    //
    // Join the termination listener thread. This thread terminates
    // when there's no more processes and when _deactivating is set to
    // true.
    //
    if (_thread.joinable())
    {
        _thread.join();
    }
    assert(_processes.empty());
}

bool
Activator::isActive()
{
    lock_guard lock(_mutex);
    return !_deactivating;
}

void
Activator::runTerminationListener()
{
    while (true)
    {
        try
        {
            terminationListener();
            break;
        }
        catch (const Exception& ex)
        {
            Error out(_traceLevels->logger);
            out << "exception in process termination listener:\n" << ex;
        }
        catch (...)
        {
            Error out(_traceLevels->logger);
            out << "unknown exception in process termination listener";
        }
    }
}

void
Activator::terminationListener()
{
#ifdef _WIN32
    while (true)
    {
        //
        // Wait for the interrupt event to be signaled.
        //
        DWORD ret = WaitForSingleObject(_hIntr, INFINITE);
        if (ret == WAIT_FAILED)
        {
            throw SyscallException{__FILE__, __LINE__, "WaitForSingleObject failed", GetLastError()};
        }
        clearInterrupt();

        //
        // Collect terminated processes
        //
        vector<Process> terminated;
        bool deactivated = false;
        {
            lock_guard lock(_mutex);
            for (vector<Process*>::const_iterator q = _terminated.begin(); q != _terminated.end(); ++q)
            {
                for (map<string, Process>::iterator p = _processes.begin(); p != _processes.end(); ++p)
                {
                    if (&p->second == *q)
                    {
                        terminated.push_back(p->second);
                        _processes.erase(p);
                        break;
                    }
                }
            }
            _terminated.clear();
            deactivated = _deactivating && _processes.empty();
        }

        for (vector<Process>::const_iterator p = terminated.begin(); p != terminated.end(); ++p)
        {
            UnregisterWait(p->waithnd);

            DWORD status;
            GetExitCodeProcess(p->hnd, &status);

            CloseHandle(p->hnd);
            assert(status != STILL_ACTIVE);

            if (_traceLevels->activator > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
                out << "detected termination of server '" << p->server->getId() << "'";
                if (status != 0)
                {
                    out << "\nexit code = " << status;
                }
            }

            try
            {
                p->server->terminated("", status);
            }
            catch (const Ice::LocalException& ex)
            {
                Ice::Warning out(_traceLevels->logger);
                out << "unexpected exception raised by server '" << p->server->getId() << "' termination:\n" << ex;
            }
        }

        if (deactivated)
        {
            return;
        }
    }
#else
    while (true)
    {
        fd_set fdSet;
        int maxFd = _fdIntrRead;
        FD_ZERO(&fdSet);
        FD_SET(_fdIntrRead, &fdSet);

        {
            lock_guard lock(_mutex);

            for (const auto& p : _processes)
            {
                int fd = p.second.pipeFd;
                FD_SET(fd, &fdSet);
                if (maxFd < fd)
                {
                    maxFd = fd;
                }
            }
        }

    repeatSelect:
        int ret = ::select(maxFd + 1, &fdSet, nullptr, nullptr, nullptr);
        assert(ret != 0);

        if (ret == -1)
        {
#    ifdef EPROTO
            if (errno == EINTR || errno == EPROTO)
            {
                goto repeatSelect;
            }
#    else
            if (errno == EINTR)
            {
                goto repeatSelect;
            }
#    endif

            throw SyscallException{__FILE__, __LINE__, "select failed", errno};
        }

        vector<Process> terminated;
        bool deactivated = false;
        {
            lock_guard lock(_mutex);

            if (FD_ISSET(_fdIntrRead, &fdSet))
            {
                clearInterrupt();

                if (_deactivating && _processes.empty())
                {
                    return;
                }
            }

            auto p = _processes.begin();
            while (p != _processes.end())
            {
                int fd = p->second.pipeFd;
                if (!FD_ISSET(fd, &fdSet))
                {
                    ++p;
                    continue;
                }

                char s[16];
                ssize_t rs;
                string message;

                //
                // Read the message over the pipe.
                //
                while ((rs = read(fd, &s, 16)) > 0) // NOLINT(clang-analyzer-unix.BlockInCriticalSection)
                {
                    message.append(s, static_cast<size_t>(rs));
                }

                //
                // Keep the received message.
                //
                if (!message.empty())
                {
                    p->second.msg += message;
                }

                if (rs == -1)
                {
                    if (errno != EAGAIN || message.empty())
                    {
                        throw SyscallException{__FILE__, __LINE__, "read failed", errno};
                    }

                    ++p;
                }
                else if (rs == 0)
                {
                    //
                    // If the pipe was closed, the process has terminated.
                    //

                    terminated.push_back(p->second);

                    close(p->second.pipeFd);
                    _processes.erase(p++);
                }
            }

            //
            // We are deactivating and there's no more active processes.
            //
            deactivated = _deactivating && _processes.empty();
        }

        for (const auto& p : terminated)
        {
            int status = waitPid(p.pid);
            if (_traceLevels->activator > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
                out << "detected termination of server '" << p.server->getId() << "'";
                if (!p.msg.empty())
                {
                    out << "\nreason = " << p.msg;
                }
                if (WIFEXITED(status) && status != 0)
                {
                    out << "\nexit code = " << WEXITSTATUS(status);
                }
                else if (WIFSIGNALED(status))
                {
                    out << "\nsignal = " << signalToString(WTERMSIG(status));
                }
            }

            try
            {
                p.server->terminated(p.msg, status);
            }
            catch (const Ice::LocalException& ex)
            {
                Ice::Warning out(_traceLevels->logger);
                out << "unexpected exception raised by server '" << p.server->getId() << "' termination:\n" << ex;
            }
        }

        if (deactivated)
        {
            return;
        }
    }
#endif
}

void
Activator::clearInterrupt()
{
#ifdef _WIN32
    ResetEvent(_hIntr);
#else
    char c;
    while (read(_fdIntrRead, &c, 1) == 1) // NOLINT(clang-analyzer-unix.BlockInCriticalSection)
        ;
#endif
}

void
Activator::setInterrupt()
{
#ifdef _WIN32
    SetEvent(_hIntr);
#else
    char c = 0;
    ssize_t sz = write(_fdIntrWrite, &c, 1);
    if (sz == -1)
    {
        throw SyscallException{__FILE__, __LINE__, "write failed", errno};
    }
#endif
}

#ifndef _WIN32
int
Activator::waitPid(pid_t processPid)
{
    try
    {
        int status;
#    if defined(__linux__)
        int nRetry = 0;
        while (true) // The while loop is necessary for the linux workaround.
        {
            pid_t pid = waitpid(processPid, &status, 0);
            if (pid < 0)
            {
                //
                // Some Linux distribution have a bogus waitpid() (e.g.: CentOS 4.x). It doesn't
                // block and reports an incorrect ECHILD error on the first call. We sleep a
                // little and retry to work around this issue (it appears from testing that a
                // single retry is enough but to make sure we retry up to 10 times before to throw.)
                //
                if (errno == ECHILD && nRetry < 10)
                {
                    // Wait 1ms, 11ms, 21ms, etc.
                    this_thread::sleep_for(chrono::milliseconds(nRetry * 10 + 1));
                    ++nRetry;
                    continue;
                }
                throw SyscallException{__FILE__, __LINE__, "waitpid failed", errno};
            }
            assert(pid == processPid);
            break;
        }
#    else
        pid_t pid = waitpid(processPid, &status, 0);
        if (pid < 0)
        {
            throw SyscallException{__FILE__, __LINE__, "waitpid failed", errno};
        }
        assert(pid == processPid);
#    endif
        return status;
    }
    catch (const Ice::LocalException& ex)
    {
        Error out(_traceLevels->logger);
        out << "unable to get process status:\n" << ex;
        return -1;
    }
}
#else
void
Activator::processTerminated(Activator::Process* process)
{
    lock_guard lock(_mutex);
    setInterrupt();
    _terminated.push_back(process);
}
#endif
