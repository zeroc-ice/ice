// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <Glacier/GlacierI.h>
#include <fcntl.h>
//#include <sys/wait.h>

#ifdef WIN32
#   error Sorry, the glacier starter is not yet supported on WIN32.
#endif

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::StarterI::StarterI(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

RouterPrx
Glacier::StarterI::startRouter(const string& userId, const string& password, const Current&)
{
    //
    // TODO: userId/password check
    //

    //
    // Start a router
    //
    string uuid = IceUtil::generateUUID();
    char buf[4*1024];
    try
    {
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
	    
	    //
	    // Setup arguments to start the router with.
	    //
	    StringSeq args = _communicator->getProperties()->getCommandLineOptions();
	    args.push_back("--Glacier.Router.Identity=" + uuid);
	    ostringstream s;
	    s << "--Glacier.Router.PrintProxyOnFd=" << fds[1];
	    args.push_back(s.str());
	    
	    //
	    // Convert to standard argc/argv.
	    //
	    int argc = args.size() + 1;
	    char** argv = static_cast<char**>(malloc((argc + 1) * sizeof(char*)));
	    StringSeq::iterator p;
	    int i;
	    for (p = args.begin(), i = 1; p != args.end(); ++p, ++i)
	    {
		assert(i < argc);
		argv[i] = strdup(p->c_str());
	    }
	    assert(i == argc);
	    argv[0] = strdup("glacier"); // TODO: Property
	    argv[argc] = 0;
	    
	    //
	    // Try to start the router.
	    //
	    if (execvp(argv[0], argv) == -1)
	    {
		//
		// Send any errors to the parent process, using the write
		// end of the pipe.
		//
		ostringstream s;
		s << "can't execute `" << argv[0] << "': " << strerror(errno);
		write(fds[1], s.str().c_str(), s.str().length());
		close(fds[1]);
		exit(EXIT_FAILURE);
	    }
	}
	else // Parent process
	{
	    //
	    // Close the write side of the newly created pipe
	    //
	    close(fds[1]);
	    
	    //
	    // Wait until data can be read from the newly started router,
	    // with timeout.
	    //
	    int flags = fcntl(fds[0], F_GETFL);
	    flags |= O_NONBLOCK;
	    if (fcntl(fds[0], F_SETFL, flags) == -1)
	    {
		SystemException ex(__FILE__, __LINE__);
		ex.error = getSystemErrno();
		throw ex;
	    }
	    
	repeatSelect:
	    fd_set fdSet;
	    FD_ZERO(&fdSet);
	    FD_SET(fds[0], &fdSet);
	    struct timeval tv;
	    tv.tv_sec = 5; // TODO: Property
	    tv.tv_usec = 0;
	    int ret = ::select(fds[0] + 1, &fdSet, 0, 0, &tv);
	    
	    if (ret == -1)
	    {
		if (errno == EINTR)
		{
		    goto repeatSelect;
		}
		
		SystemException ex(__FILE__, __LINE__);
		ex.error = getSystemErrno();
		throw ex;
	    }
	    
	    if (ret == 0) // Timeout
	    {
		assert(false); // TODO: Handle this situation.
	    }

	    assert(FD_ISSET(fds[0], &fdSet));
	
	    //
	    // Read the response
	    //
	    ssize_t sz = read(fds[0], buf, sizeof(buf)/sizeof(char) - 1);
	    if(sz == -1)
	    {
		SystemException ex(__FILE__, __LINE__);
		ex.error = getSystemErrno();
		throw ex;
	    }
	    assert(sz != 0); // TODO: Handle EOF
	    buf[sz] = '\0'; // Terminate the string we got back.
	}
    }
    catch(const LocalException& ex)
    {
	// TODO: Log exception or print warning
	cerr << ex << endl;
	ex.ice_throw();
    }

    if (strncmp(buf, uuid.c_str(), uuid.length()) == 0)
    {
	//
	// We got the stringified router proxy.
	//
	return RouterPrx::uncheckedCast(_communicator->stringToProxy(buf));
    }
    else
    {
	//
	// We got something else.
	//
	CannotStartRouterException ex;
	ex.reason = buf;
	// TODO: Log exception.
	cerr << ex << endl;
	throw ex;
    }
}
