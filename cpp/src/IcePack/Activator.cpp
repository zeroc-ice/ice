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
#include <sstream>
#include <fcntl.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::Activator::Activator(const CommunicatorPtr& communicator) :
    _communicator(communicator),
    _destroy(false)
{
    int fds[2];
    if (::pipe(fds) != 0)
    {
	throw SystemException(__FILE__, __LINE__);
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
    JTCSyncT<JTCMonitor> sync(*this);

    if (!_destroy) // Don't destroy twice
    {
	_destroy = true;
	setInterrupt();
    }
}

void
IcePack::Activator::activate(const ServerDescriptionPtr& desc)
{
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
	    JTCSyncT<JTCMonitor> sync(*this);

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
	    
	    throw SystemException(__FILE__, __LINE__);
	}
	
	{
	    JTCSyncT<JTCMonitor> sync(*this);
	    
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
		    char c;
		    int ret = ::read(fd, &c, 1);
		    if (ret == -1)
		    {
			throw SystemException(__FILE__, __LINE__);
		    }
		    
		    //
		    // Either the pipe was closed, which means that
		    // the process has terminated. Or the process is
		    // misbehaving and sending some garbage data. In
		    // both cases we remove the process.
		    //
		    map<string, Process>::iterator q = p;
		    ++p;
		    _processes.erase(q);
		    close(fd);
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
IcePack::Activator::clearInterrupt()
{
    char s[32]; // Clear up to 32 interrupts at once
    while (::read(_fdIntrRead, s, 32) == 32)
	;
}

void
IcePack::Activator::setInterrupt()
{
    char c = 0;
    ::write(_fdIntrWrite, &c, 1);
}
