// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/Activator.h>
#include <IcePack/Admin.h>
#include <fcntl.h>
#include <sys/wait.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::Activator::Activator(const CommunicatorPtr& communicator) :
    _communicator(communicator),
    _destroy(false)
{
    int fds[2];
    if (pipe(fds) != 0)
    {
	SystemException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
    _fdIntrRead = fds[0];
    _fdIntrWrite = fds[1];
    int flags = fcntl(_fdIntrRead, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(_fdIntrRead, F_SETFL, flags);
}

IcePack::Activator::~Activator()
{
    assert(_destroy);
    close(_fdIntrRead);
    close(_fdIntrWrite);
    for (map<string, Process>::iterator p = _processes.begin(); p != _processes.end(); ++p)
    {
	close(p->second.fd);
    }
}

void
IcePack::Activator::run()
{
    try
    {
	terminationListener();
    }
    catch (const LocalException& ex)
    {
	ostringstream s;
	s << "exception in process termination listener:\n" << ex;
	_communicator->getLogger()->error(s.str());
    }
    catch (const JTCException& ex)
    {
	ostringstream s;
	s << "exception in process termination listener:\n" << ex;
	_communicator->getLogger()->error(s.str());
    }
    catch (...)
    {
	_communicator->getLogger()->error("unknown exception in process termination listener");
    }
}

void
IcePack::Activator::destroy()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_destroy) // Don't destroy twice
    {
	return;
    }

    _destroy = true;
    setInterrupt();
}

bool
IcePack::Activator::activate(const ServerDescription& desc)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_destroy)
    {
	return false;
    }

    string path = desc.path;
    if (path.empty())
    {
	return false;
    }

    //
    // Normalize the pathname a bit
    //
    string::size_type pos;
    while ((pos = path.find("//")) != string::npos)
    {
	path.erase(pos, 1);
    }
    while ((pos = path.find("/./")) != string::npos)
    {
	path.erase(pos, 2);
    }

    //
    // Do nothing if the process exists
    //
    if (_processes.count(path))
    {
	return false;
    }

    //
    // Process does not exist, activate and create
    //
    int fds[2];
    if (pipe(fds) != 0)
    {
	SystemException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
    pid_t pid = fork();
    if (pid == -1)
    {
	SystemException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
    if (pid == 0) // Child process
    {
	close(fds[0]);

	//
	// Close all filedescriptors, except for standard input,
	// standard output, standard error output, and the write side
	// of the newly created pipe.
	//
	int maxFd = static_cast<int>(sysconf(_SC_OPEN_MAX));
	for (int fd = 3; fd < maxFd; ++fd)
	{
	    if (fd != fds[1])
	    {
		close(fd);
	    }
	}

	int argc = desc.args.size() + 2;
	char** argv = static_cast<char**>(malloc(argc * sizeof(char*)));
	argv[0] = strdup(path.c_str());
	for (unsigned int i = 0; i < desc.args.size(); ++i)
	{
	    argv[i + 1] = strdup(desc.args[i].c_str());
	}
	argv[argc - 1] = 0;

	if (execvp(argv[0], argv) == -1)
	{
	    //
	    // Send any errors to the parent process, using the write
	    // end of the pipe.
	    //
	    SystemException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
	    throw ex;
	    ostringstream s;
	    s << "can't execute `" << path << "':\n" << ex;
	    write(fds[1], s.str().c_str(), s.str().length());
	    close(fds[1]);
	    exit(EXIT_FAILURE);
	}
    }
    else // Parent process
    {
	close(fds[1]);

	Process process;
	process.pid = pid;
	process.fd = fds[0];
	_processes[path] = process;

	int flags = fcntl(process.fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(process.fd, F_SETFL, flags);
	
	setInterrupt();
    }

    return true;
}

void
IcePack::Activator::terminationListener()
{
    while (true)
    {
	fd_set fdSet;
	int maxFd = _fdIntrRead;
	FD_ZERO(&fdSet);
	FD_SET(_fdIntrRead, &fdSet);
	
	{
	    JTCSyncT<JTCMutex> sync(*this);

	    if (_destroy)
	    {
		return;
	    }
	    
	    for (map<string, Process>::iterator p = _processes.begin(); p != _processes.end(); ++p)
	    {
		int fd = p->second.fd;
		FD_SET(fd, &fdSet);
		if (maxFd < fd)
		{
		    maxFd = fd;
		}
	    }
	}
	
    repeatSelect:
	int ret = ::select(maxFd + 1, &fdSet, 0, 0, 0);
	assert(ret != 0);
	
	if (ret == -1)
	{
	    if (errno == EINTR || errno == EPROTO)
	    {
		goto repeatSelect;
	    }
	    
	    SystemException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
	    throw ex;
	}
	
	{
	    JTCSyncT<JTCMutex> sync(*this);
	    
	    if (FD_ISSET(_fdIntrRead, &fdSet))
	    {
		clearInterrupt();
	    }
	    
	    if (_destroy)
	    {
		return;
	    }

	    map<string, Process>::iterator p = _processes.begin();
	    while (p != _processes.end())
	    {
		int fd = p->second.fd;
		if (FD_ISSET(fd, &fdSet))
		{
		    char s[16];
		    int ret = read(fd, &s, 16);
		    if (ret == -1)
		    {
			SystemException ex(__FILE__, __LINE__);
			ex.error = getSystemErrno();
			throw ex;
		    }
		    else if(ret == 0)
		    {
			//
			// If the pipe was closed, the process has
			// terminated.
			//
			map<string, Process>::iterator q = p;
			++p;
			_processes.erase(q);
			close(fd);
		    }
		    else
		    {
			//
			// Other messages that are sent down the pipe
			// are interpreted as error messages and
			// logged as error.
			//
			string err;
			do
			{
			    err.append(s, ret);
			    ret = read(fd, &s, 16);
			}
			while (ret != 0);
			_communicator->getLogger()->error(err);
		    }
		}
		else
		{
		    ++p;
		}
	    }
	}

	//
	// Remove zombie processes, if any
	//
	if (waitpid(-1, 0, WNOHANG | WUNTRACED) == -1)
	{
	    if (errno != ECHILD) // Ignore ECHILD
	    {
		SystemException ex(__FILE__, __LINE__);
		ex.error = getSystemErrno();
		throw ex;
	    }
	}
    }
}

void
IcePack::Activator::clearInterrupt()
{
    char s[32]; // Clear up to 32 interrupts at once
    while (read(_fdIntrRead, s, 32) == 32)
	;
}

void
IcePack::Activator::setInterrupt()
{
    char c = 0;
    write(_fdIntrWrite, &c, 1);
}
