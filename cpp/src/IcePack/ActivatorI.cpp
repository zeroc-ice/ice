// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/ActivatorI.h>
#include <IcePack/Admin.h>
#include <IcePack/Internal.h>
#include <IcePack/TraceLevels.h>

#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

namespace IcePack
{

class TerminationListenerThread : public IceUtil::Thread
{
public:

    TerminationListenerThread(ActivatorI& activator) :
	_activator(activator)
    {
    }

    virtual 
    void run()
    {
	_activator.runTerminationListener();
    }

private:
    
    ActivatorI& _activator;
};

}

IcePack::ActivatorI::ActivatorI(const TraceLevelsPtr& traceLevels, const PropertiesPtr& properties) :
    _traceLevels(traceLevels),
    _properties(properties),
    _deactivating(false)
{
#ifdef _WIN32
    _hIntr = CreateEvent(
        NULL,  // Security attributes
        TRUE,  // Manual reset
        FALSE, // Initial state is nonsignaled
        NULL   // Unnamed
    );

    if(_hIntr == NULL)
    {
	SyscallException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
#else
    int fds[2];
    if(pipe(fds) != 0)
    {
	SyscallException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
    _fdIntrRead = fds[0];
    _fdIntrWrite = fds[1];
    int flags = fcntl(_fdIntrRead, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(_fdIntrRead, F_SETFL, flags);
#endif

    //
    // Parse the properties override property.
    //
    string props = _properties->getProperty("IcePack.Node.PropertiesOverride");
    if(!props.empty())
    {
	string::size_type end = 0;
	while(end != string::npos)
	{
	    const string delim = " \t\r\n";
		
	    string::size_type beg = props.find_first_not_of(delim, end);
	    if(beg == string::npos)
	    {
		break;
	    }
		
	    end = props.find_first_of(delim, beg);
	    string arg;
	    if(end == string::npos)
	    {
		arg = props.substr(beg);
	    }
	    else
	    {
		arg = props.substr(beg, end - beg);
	    }
	    if(arg.find("--") != 0)
	    {
		arg = "--" + arg;
	    }
	    _propertiesOverride.push_back(arg);
	}
    }    
}

IcePack::ActivatorI::~ActivatorI()
{
    assert(!_thread);
    
#ifdef _WIN32
    if(_hIntr != NULL)
    {
        CloseHandle(_hIntr);
    }
#else
    close(_fdIntrRead);
    close(_fdIntrWrite);
#endif
}

bool
IcePack::ActivatorI::activate(const ServerPtr& server)
{
    IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);

    if(_deactivating)
    {
	return false;
    }

    string path = server->description.path;
    if(path.empty())
    {
	return false;
    }

    string pwd = server->description.pwd;

#ifdef _WIN32
    //
    // Get the absolute pathname of the executable.
    //
    char absbuf[_MAX_PATH];
    if(_fullpath(absbuf, path.c_str(), _MAX_PATH) == NULL)
    {
        Error out(_traceLevels->logger);
        out << "cannot convert `" << path << "' into an absolute path";
        return false;
    }
    path = absbuf;

    //
    // Get the absolute pathname of the working directory.
    //
    if(!pwd.empty())
    {
        if(_fullpath(absbuf, pwd.c_str(), _MAX_PATH) == NULL)
        {
            Error out(_traceLevels->logger);
            out << "cannot convert `" << pwd << "' into an absolute path";
            return false;
        }
        pwd = absbuf;
    }
#else
    //
    // Normalize the pathname a bit.
    //
    {
	string::size_type pos;
	while((pos = path.find("//")) != string::npos)
	{
	    path.erase(pos, 1);
	}
	while((pos = path.find("/./")) != string::npos)
	{
	    path.erase(pos, 2);
	}
    }

    //
    // Normalize the path to the working directory.
    //
    if(!pwd.empty())
    {
	string::size_type pos;
	while((pos = pwd.find("//")) != string::npos)
	{
	    pwd.erase(pos, 1);
	}
	while((pos = pwd.find("/./")) != string::npos)
	{
	    pwd.erase(pos, 2);
	}
    }
#endif

    //
    // Setup arguments.
    //
    StringSeq args;
    args.push_back(path);
    args.insert(args.end(), server->description.args.begin(), server->description.args.end());
    args.insert(args.end(), _propertiesOverride.begin(), _propertiesOverride.end());
    args.push_back("--Ice.Default.Locator=" + _properties->getProperty("Ice.Default.Locator"));

    if(_traceLevels->activator > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
	out << "activating server `" << server->description.name << "'";
	if(_traceLevels->activator > 2)
	{
	    out << "\n";
	    out << "path = " << path << "\n";
	    out << "pwd = " << pwd << "\n";
	    out << "args = ";

	    StringSeq::const_iterator p = args.begin();
            ++p;
	    for(StringSeq::const_iterator q = p; q != args.end(); ++q)
	    {
		out << " " << *q;
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
    StringSeq::const_iterator p;
    for(p = args.begin(); p != args.end(); ++p)
    {
        if(p != args.begin())
        {
            cmd.push_back(' ');
        }
        //
        // Enclose arguments containing spaces in double quotes.
        //
        if((*p).find(' ') != string::npos)
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

    const char* dir;
    if(!pwd.empty())
    {
        dir = pwd.c_str();
    }
    else
    {
        dir = NULL;
    }

    //
    // Make a copy of the command line.
    //
    char* cmdbuf = strdup(cmd.c_str());

    //
    // Create the environment block for the child process. We start with the environment
    // of this process, and then merge environment variables from the server description.
    //
    const char* env = NULL;
    string envbuf;
//cout << "Activator: description.envs.size() = " << server->description.envs.size() << endl;
    if(!server->description.envs.empty())
    {
//cout << "Activator: creating environment block" << endl;
        map<string, string> envMap;
        LPVOID parentEnv = GetEnvironmentStrings();
        const char* var = reinterpret_cast<const char*>(parentEnv);
        while(*var)
        {
            string s(var);
//cout << "  - adding parent variable `" << s << "'" << endl;
            string::size_type pos = s.find('=');
            if(pos != string::npos)
            {
                envMap.insert(map<string, string>::value_type(s.substr(0, pos), s.substr(pos + 1)));
            }
        }
        FreeEnvironmentStrings(static_cast<char*>(parentEnv));
        for(p = server->description.envs.begin(); p != server->description.envs.end(); ++p)
        {
            string s = *p;
//cout << "  - adding descriptor variable `" << s << "'" << endl;
            string::size_type pos = s.find('=');
            if(pos != string::npos)
            {
                envMap.insert(map<string, string>::value_type(s.substr(0, pos), s.substr(pos + 1)));
            }
        }
        for(map<string, string>::const_iterator q = envMap.begin(); q != envMap.end(); ++q)
        {
            envbuf.append(q->first);
            envbuf.push_back('=');
            envbuf.append(q->second);
            envbuf.push_back('\0');
        }
        envbuf.push_back('\0');
        env = envbuf.c_str();
//cout << "  - envbuf.size() = " << envbuf.size() << endl;
    }

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    BOOL b = CreateProcess(
        NULL,                     // Executable
        cmdbuf,                   // Command line
        NULL,                     // Process attributes
        NULL,                     // Thread attributes
        FALSE,                    // Inherit handles
        CREATE_NEW_PROCESS_GROUP, // Process creation flags
        (LPVOID)env,              // Process environment
        dir,                      // Current directory
        &si,                      // Startup info
        &pi                       // Process info
    );

//    delete[] cmdbuf;
    free(cmdbuf);

    if(!b)
    {
        SyscallException ex(__FILE__, __LINE__);
        ex.error = getSystemErrno();
        throw ex;
    }

    //
    // Caller is responsible for closing handles in PROCESS_INFORMATION. We don't need to
    // keep the thread handle, so we close it now. The process handle will be closed later.
    //
    CloseHandle(pi.hThread);

    Process process;
    process.pid = pi.dwProcessId;
    process.hnd = pi.hProcess;
    process.server = server;
    _processes.push_back(process);
    
    setInterrupt();

    if(_traceLevels->activator > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
        out << "activated server `" << server->description.name << "' (pid = " << pi.dwProcessId << ")";
    }
#else
    int fds[2];
    if(pipe(fds) != 0)
    {
	SyscallException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
    pid_t pid = fork();
    if(pid == -1)
    {
	SyscallException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }

    if(pid == 0) // Child process.
    {
	//
	// TODO: eliminate all non-async-signal-safe calls, in particular anything
	// that may allocate dynamic memory.
	//


#ifdef __linux
	//
	// Create a process group for this child, to be able to send 
	// a signal to all the thread-processes with killpg
	//
	setpgrp();
#endif

	//
	// Close all file descriptors, except for standard input,
	// standard output, standard error, and the write side
	// of the newly created pipe.
	//
	int maxFd = static_cast<int>(sysconf(_SC_OPEN_MAX));
	for(int fd = 3; fd < maxFd; ++fd)
	{
	    if(fd != fds[1])
	    {
		close(fd);
	    }
	}

	//
	// Convert to standard argc/argv.
	//
	int argc = static_cast<int>(args.size());
	char** argv = static_cast<char**>(malloc((argc + 1) * sizeof(char*)));
	int i = 0;
	for(StringSeq::const_iterator p = args.begin(); p != args.end(); ++p, ++i)
	{
	    assert(i < argc);
	    argv[i] = strdup(p->c_str());
	}
	assert(i == argc);
	argv[argc] = 0;

	for(StringSeq::const_iterator q = server->description.envs.begin(); q != server->description.envs.end(); ++q)
	{
	    if(putenv(strdup(q->c_str())) != 0)
	    {
		SyscallException ex(__FILE__, __LINE__);
		ex.error = getSystemErrno();
		ostringstream s;
		s << "can't set environment variable: " << *q << "':\n" << ex;
		write(fds[1], s.str().c_str(), s.str().length());
		close(fds[1]);
		_exit(EXIT_FAILURE);
	    }
	}

	//
	// Redirect the standard error output to the write side of the
	// pipe.
	//
	// TODO: This doesn't work well if the server doesn't control
	// when and how the output is flushed. For example, we could
	// receive one character after the other over the pipe which
	// is problematic since we don't know when to log message. For
	// sure we don't want to log each character one after the
	// other. We want to log the message. One solution would be to
	// put the received information in a buffer and log it only
	// when we receive a '\n'.
	//

// 	if(fds[1] != STDERR_FILENO)
// 	{
// 	    if(dup2(fds[1], STDERR_FILENO) != STDERR_FILENO)
// 	    {
// 		//
// 		// Send any errors to the parent process, using the write
// 		// end of the pipe.
// 		//
// 		SyscallException ex(__FILE__, __LINE__);
// 		ex.error = getSystemErrno();
// 		ostringstream s;
// 		s << "can't redirect stderr to the pipe output";
// 		write(fds[1], s.str().c_str(), s.str().length());
// 		close(fds[1]);
// 		_exit(EXIT_FAILURE);
// 	    }
// 	}

	//
	// Change working directory.
	//
	if(!pwd.empty())
	{
	    if(chdir(pwd.c_str()) == -1)
	    {
		//
		// Send any errors to the parent process, using the write
		// end of the pipe.
		//
		SyscallException ex(__FILE__, __LINE__);
		ex.error = getSystemErrno();
		ostringstream s;
		s << "can't change working directory to `" << pwd << "':\n" << ex;
		write(fds[1], s.str().c_str(), s.str().length());
		close(fds[1]);
		_exit(EXIT_FAILURE);
	    }
	}	

	if(execvp(argv[0], argv) == -1)
	{
	    //
	    // Send any errors to the parent process, using the write
	    // end of the pipe.
	    //
	    SyscallException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
	    ostringstream s;
	    s << "can't execute `" << path << "':\n" << ex;
	    write(fds[1], s.str().c_str(), s.str().length());
	    close(fds[1]);
	    _exit(EXIT_FAILURE);
	}
    }
    else // Parent process.
    {
	close(fds[1]);

	Process process;
	process.pid = pid;
	process.fd = fds[0];
	process.server = server;
	_processes.push_back(process);
	
	int flags = fcntl(process.fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(process.fd, F_SETFL, flags);

	setInterrupt();

	if(_traceLevels->activator > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
	    out << "activated server `" << server->description.name << "' (pid = " << pid << ")";
	}
    }
#endif

    return true;
}

void
IcePack::ActivatorI::deactivate(const ServerPtr& server)
{
    Ice::Int pid = server->getPid();

    if(pid == 0)
    {
	//
	// Server is already deactivated.
	//
	return;
    }

#ifdef _WIN32
    //
    // Generate a Ctrl+Break event on the child.
    //
    if(GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pid))
    {
        if(_traceLevels->activator > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
            out << "sent Ctrl+Break to server `" << server->description.name << "' (pid = " << pid << ")";
        }
    }
    else
    {
        SyscallException ex(__FILE__, __LINE__);
        ex.error = getSystemErrno();
        throw ex;
    }
#else
    //
    // Send a SIGTERM to the process.
    //

#ifdef __linux
    //
    // Use process groups on Linux instead of processes
    //
    int ret = ::killpg(static_cast<pid_t>(pid), SIGTERM);
#else
    int ret = ::kill(static_cast<pid_t>(pid), SIGTERM);
#endif
    if(ret != 0 && getSystemErrno() != ESRCH)
    {
	SyscallException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }

    if(_traceLevels->activator > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
	out << "sent SIGTERM to server `" << server->description.name << "' (pid = " << pid << ")";
    }
#endif
}

void
IcePack::ActivatorI::kill(const ServerPtr& server)
{
    Ice::Int pid = server->getPid();
    
    if(pid == 0)
    {
	//
	// Server is already deactivated.
	//
	return;
    }

#ifdef _WIN32
    HANDLE hnd = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if(hnd == NULL)
    {
        SyscallException ex(__FILE__, __LINE__);
        ex.error = getSystemErrno();
        throw ex;
    }

    BOOL b = TerminateProcess(hnd, 1);

    CloseHandle(hnd);

    if(!b)
    {
        SyscallException ex(__FILE__, __LINE__);
        ex.error = getSystemErrno();
        throw ex;
    }

    if(_traceLevels->activator > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
	out << "terminating server `" << server->description.name << "' (pid = " << pid << ")";
    }
#else
    //
    // Send a SIGKILL to the process.
    //
#ifdef __linux
    // Use process groups on Linux instead of processes
    int ret = ::killpg(static_cast<pid_t>(pid), SIGKILL);
#else
    int ret = ::kill(static_cast<pid_t>(pid), SIGKILL);
#endif
    if(ret != 0 && getSystemErrno() != ESRCH)
    {
	SyscallException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }

    if(_traceLevels->activator > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
	out << "sent SIGKILL to server `" << server->description.name << "' (pid = " << pid << ")";
    }
#endif
}

Ice::Int
IcePack::ActivatorI::getServerPid(const ServerPtr& server)
{
    IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);

    for(vector<Process>::iterator p = _processes.begin(); p != _processes.end(); ++p)
    {
	if(p->server == server)
	{
	    return static_cast<Ice::Int>(p->pid);
	}
    }

    return 0;
}

void
IcePack::ActivatorI::start()
{
    //
    // Create and start the termination listener thread.
    //
    _thread = new TerminationListenerThread(*this);
    _thread->start();
}

void
IcePack::ActivatorI::waitForShutdown()
{
    IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);
    while(!_deactivating)
    {
	wait();
    }
}

void
IcePack::ActivatorI::shutdown()
{
    IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);
    //
    // Deactivation has been initiated. Set _deactivating to true to 
    // prevent activation of new processes. This will also cause the
    // termination listener thread to stop when there are no more
    // active processes.
    //
    _deactivating = true;
    setInterrupt();
    notifyAll();
}

void
IcePack::ActivatorI::destroy()
{
    {
	IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);
	assert(_deactivating);
    }

    //
    // Deactivate all the processes.
    //
    deactivateAll();

    //
    // Join the termination listener thread. This thread terminates
    // when there's no more processes and when _deactivating is set to
    // true.
    //
    _thread->getThreadControl().join();
    _thread = 0;
}

void
IcePack::ActivatorI::runTerminationListener()
{
    try
    {
	terminationListener();
    }
    catch(const Exception& ex)
    {
	Error out(_traceLevels->logger);
	out << "exception in process termination listener:\n" << ex;
    }
    catch(...)
    {
	Error out(_traceLevels->logger);
	out << "unknown exception in process termination listener";
    }
}

void
IcePack::ActivatorI::deactivateAll()
{
    //
    // Stop all active processes.
    //
    vector<Process> processes;
    {
	IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);
	processes = _processes;
    }

    for(vector<Process>::iterator p = processes.begin(); p != processes.end(); ++p)
    {
	//
	// Stop the server. The listener thread should detect the
	// process deactivation and remove it from the activator's
	// list of active processes.
	//
	try
	{
	    p->server->stop();
	}
	catch(const ObjectNotExistException&)
	{
	    //
	    // Expected if the server was in the process of being destroyed.
	    //
	}
    }
}

void
IcePack::ActivatorI::terminationListener()
{
#ifdef _WIN32
    while(true)
    {
        vector<HANDLE> handles;

        //
        // Lock while we collect the process handles.
        //
        {
            IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);

            for(vector<Process>::iterator p = _processes.begin(); p != _processes.end(); ++p)
            {
                handles.push_back(p->hnd);
            }
        }

        handles.push_back(_hIntr);

        //
        // Wait for a child to terminate, or the interrupt event to be signaled.
        //
        DWORD ret = WaitForMultipleObjects(handles.size(), &handles[0], FALSE, INFINITE);
        if(ret == WAIT_FAILED)
        {
            SyscallException ex(__FILE__, __LINE__);
            ex.error = getSystemErrno();
            throw ex;
        }

        vector<HANDLE>::size_type pos = ret - WAIT_OBJECT_0;
        assert(pos < handles.size());
        HANDLE hnd = handles[pos];

        IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);

        if(hnd == _hIntr)
        {
            clearInterrupt();
        }
        else
        {
            for(vector<Process>::iterator p = _processes.begin(); p != _processes.end(); ++p)
            {
                if(p->hnd == hnd)
                {
                    if(_traceLevels->activator > 0)
                    {
                        Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
                        out << "detected termination of server `" << p->server->description.name << "'";
                    }

                    try
                    {
                        p->server->terminated();
                    }
                    catch(const Ice::LocalException& ex)
                    {
                        Ice::Warning out(_traceLevels->logger);
                        out << "unexpected exception raised by server `" << p->server->description.name 
                            << "' termination:\n" << ex;
                    }
                            
                    _processes.erase(p);
                    break;
                }
            }
            CloseHandle(hnd);
        }

        if(_deactivating && _processes.empty())
        {
            return;
        }
    }
#else
    while(true)
    {
	fd_set fdSet;
	int maxFd = _fdIntrRead;
	FD_ZERO(&fdSet);
	FD_SET(_fdIntrRead, &fdSet);
	
	{
	    IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);

	    for(vector<Process>::iterator p = _processes.begin(); p != _processes.end(); ++p)
	    {
		int fd = p->fd;
		FD_SET(fd, &fdSet);
		if(maxFd < fd)
		{
		    maxFd = fd;
		}
	    }
	}
	
    repeatSelect:
	int ret = ::select(maxFd + 1, &fdSet, 0, 0, 0);
	assert(ret != 0);
	
	if(ret == -1)
	{
#ifdef EPROTO
	    if(errno == EINTR || errno == EPROTO)
	    {
		goto repeatSelect;
	    }
#else
	    if(errno == EINTR)
	    {
		goto repeatSelect;
	    }
#endif
	    
	    SyscallException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
	    throw ex;
	}
	
	{
	    IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);
	    
	    if(FD_ISSET(_fdIntrRead, &fdSet))
	    {
		clearInterrupt();

		if(_deactivating && _processes.empty())
                {
                    return;
                }
	    }
	    
	    vector<Process>::iterator p = _processes.begin();
	    while(p != _processes.end())
	    {
		int fd = p->fd;
		if(!FD_ISSET(fd, &fdSet))   
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
		while((rs = read(fd, &s, 16)) > 0)
		{
		    message.append(s, rs);
		}

		if(rs == -1)
		{
		    if(errno != EAGAIN || message.empty())
		    {
			SyscallException ex(__FILE__, __LINE__);
			ex.error = getSystemErrno();
			throw ex;
		    }

		    ++p;
		}
		else if(rs == 0)
		{    
		    //
		    // If the pipe was closed, the process has terminated.
		    //

		    if(_traceLevels->activator > 0)
		    {
			Ice::Trace out(_traceLevels->logger, _traceLevels->activatorCat);
                        out << "detected termination of server `" << p->server->description.name << "'";
		    }

		    try
		    {
			p->server->terminated();
		    }
		    catch(const Ice::LocalException& ex)
		    {
			Ice::Warning out(_traceLevels->logger);
			out << "unexpected exception raised by server `" << p->server->description.name 
			    << "' termination:\n" << ex;
		    }
			    
		    p = _processes.erase(p);
		    close(fd);

		    //
		    // We are deactivating and there's no more active processes. We can now 
		    // end this loop
		    //
		    if(_deactivating && _processes.empty())
		    {
			return;
		    }
		}

		//
		// Log the received message.
		//
		if(!message.empty())
		{
		    Error out(_traceLevels->logger);
		    out << message;
		}
	    }
	}
    }
#endif
}

void
IcePack::ActivatorI::clearInterrupt()
{
#ifdef _WIN32
    ResetEvent(_hIntr);
#else
    char c;
    while(read(_fdIntrRead, &c, 1) == 1)
        ;
#endif
}

void
IcePack::ActivatorI::setInterrupt()
{
#ifdef _WIN32
    SetEvent(_hIntr);
#else
    char c = 0;
    write(_fdIntrWrite, &c, 1);
#endif
}
