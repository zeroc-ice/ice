// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifdef _WIN32
#   error Sorry, the IcePack Activator is not yet supported on WIN32.
#endif

#include <Ice/Ice.h>
#include <IcePack/ActivatorI.h>
#include <IcePack/Admin.h>
#include <IcePack/ServerManager.h>
#include <fcntl.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::ActivatorI::ActivatorI(const CommunicatorPtr& communicator) :
    _communicator(communicator),
    _destroy(false)
{
    int fds[2];
    if(pipe(fds) != 0)
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

IcePack::ActivatorI::~ActivatorI()
{
    assert(_destroy);
    close(_fdIntrRead);
    close(_fdIntrWrite);
    for(vector<Process>::iterator p = _processes.begin(); p != _processes.end(); ++p)
    {
	close(p->fd);
    }
}

void
IcePack::ActivatorI::run()
{
    try
    {
	terminationListener();
    }
    catch(const Exception& ex)
    {
	Error out(_communicator->getLogger());
	out << "exception in process termination listener:\n" << ex;
    }
    catch(...)
    {
	Error out(_communicator->getLogger());
	out << "unknown exception in process termination listener";
    }
}

void
IcePack::ActivatorI::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    if(_destroy) // Don't destroy twice.
    {
	return;
    }

    _destroy = true;
    setInterrupt();
}

bool
IcePack::ActivatorI::activate(const ServerPrx& server, const ::Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    if(_destroy)
    {
	return false;
    }

    ServerDescription desc = server->getServerDescription();

    string path = desc.path;
    if(path.empty())
    {
	return false;
    }

    //
    // Normalize the pathname a bit.
    //
    string::size_type pos;
    while((pos = path.find("//")) != string::npos)
    {
	path.erase(pos, 1);
    }
    while((pos = path.find("/./")) != string::npos)
    {
	path.erase(pos, 2);
    }

    //
    // Process does not exist, activate and create.
    //
    int fds[2];
    if(pipe(fds) != 0)
    {
	SystemException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
    pid_t pid = fork();
    if(pid == -1)
    {
	SystemException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
    if(pid == 0) // Child process.
    {
	//
	// Close all file descriptors, except for standard input,
	// standard output, standard error output, and the write side
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
	// Redirect the standard error output to the write side of the
	// pipe.
	//
	// TODO: This doesn't work well if the server doesn't control
	// when and how the output is flushed. This can result for
	// example in printing on character by line... The standard
	// Ice logger could be changed to flush the stream and not use
	// automatic flushing.
	//

// TODO: ML: Can you be more specific? Sending messages through a pipe
// is one of the most basic unix mechanisms. I don't see how this
// could not work.

// 	if(fds[1] != STDERR_FILENO)
// 	{
// 	    if(dup2(fds[1], STDERR_FILENO) != STDERR_FILENO)
// 	    {
// 		//
// 		// Send any errors to the parent process, using the write
// 		// end of the pipe.
// 		//
// 		SystemException ex(__FILE__, __LINE__);
// 		ex.error = getSystemErrno();
// 		ostringstream s;
// 		s << "can't redirect stderr to the pipe output";
// 		write(fds[1], s.str().c_str(), s.str().length());
// 		close(fds[1]);
// 		exit(EXIT_FAILURE);
// 	    }
// 	}

	//
	// Change working directory.
	//
	string pwd = desc.pwd;
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

	    if(chdir(pwd.c_str()) == -1)
	    {
		//
		// Send any errors to the parent process, using the write
		// end of the pipe.
		//
		SystemException ex(__FILE__, __LINE__);
		ex.error = getSystemErrno();
		ostringstream s;
		s << "can't change working directory to `" << pwd << "':\n" << ex;
		write(fds[1], s.str().c_str(), s.str().length());
		close(fds[1]);
		exit(EXIT_FAILURE);
	    }
	}
	
	//
	// Compute arguments.
	//
	int argc = desc.args.size() + 2;
	char** argv = static_cast<char**>(malloc(argc * sizeof(char*)));
	argv[0] = strdup(path.c_str());
	unsigned int i = 0;
	vector<string>::const_iterator q;
	for(q = desc.args.begin(); q != desc.args.end(); ++q, ++i)
	{
	    argv[i + 1] = strdup(q->c_str());
	}
	argv[argc - 1] = 0;

	if(execvp(argv[0], argv) == -1)
	{
	    //
	    // Send any errors to the parent process, using the write
	    // end of the pipe.
	    //
	    SystemException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
	    ostringstream s;
	    s << "can't execute `" << path << "':\n" << ex;
	    write(fds[1], s.str().c_str(), s.str().length());
	    close(fds[1]);
	    exit(EXIT_FAILURE);
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
    }

    return true;
}

void
IcePack::ActivatorI::terminationListener()
{
    while(true)
    {
	fd_set fdSet;
	int maxFd = _fdIntrRead;
	FD_ZERO(&fdSet);
	FD_SET(_fdIntrRead, &fdSet);
	
	{
	    IceUtil::Mutex::Lock sync(*this);

	    if(_destroy)
	    {
		return;
	    }
	    
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
	    if(errno == EINTR || errno == EPROTO)
	    {
		goto repeatSelect;
	    }
	    
	    SystemException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
	    throw ex;
	}
	
	{
	    IceUtil::Mutex::Lock sync(*this);
	    
	    if(FD_ISSET(_fdIntrRead, &fdSet))
	    {
		clearInterrupt();
	    }
	    
	    if(_destroy)
	    {
		return;
	    }

	    vector<Process>::iterator p = _processes.begin();
	    while(p != _processes.end())
	    {
		int fd = p->fd;
		if(FD_ISSET(fd, &fdSet))
		{
		    char s[16];
		    int ret;
		    string message;

		    //
		    // Read the message over the pipe.
		    //
		    while((ret = read(fd, &s, 16)) > 0)
		    {
			message.append(s, ret);
		    }

		    if(ret == -1)
		    {
			if(errno != EAGAIN || message.empty())
			{
			    SystemException ex(__FILE__, __LINE__);
			    ex.error = getSystemErrno();
			    throw ex;
			}

			++p;
		    }
		    else if(ret == 0)
		    {
			ServerPrx server = p->server;

			//
			// If the pipe was closed, the process has
			// terminated.
			//
			p = _processes.erase(p);
			close(fd);
			
			//
			// Notify the server it has terminated.
			//
			try
			{
			    server->terminationCallback();
			}
			catch(const Ice::ObjectAdapterDeactivatedException&)
			{
			    //
			    // Expected when IcePack is shutdown.
			    //
			}
		    }

		    //
		    // Log the received message.
		    //
		    if(!message.empty())
		    {
			Error out(_communicator->getLogger());
			out << message;
		    }
		}
		else
		{
		    ++p;
		}
	    }
	}
    }
}

void
IcePack::ActivatorI::clearInterrupt()
{
    char s[32]; // Clear up to 32 interrupts at once.
    while(read(_fdIntrRead, s, 32) == 32)
    {
    }
}

void
IcePack::ActivatorI::setInterrupt()
{
    char c = 0;
    write(_fdIntrWrite, &c, 1);
}
