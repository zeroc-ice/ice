// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <IceSSL/RSAKeyPair.h>
#include <Glacier/StarterI.h>
#include <Ice/SliceChecksums.h>
#ifndef _WIN32
#   include <fcntl.h>
#endif

using namespace std;
using namespace Ice;
using namespace Glacier;

using IceSSL::RSAKeyPairPtr;

Glacier::StarterI::StarterI(const CommunicatorPtr& communicator, const PermissionsVerifierPrx& verifier) :
    _communicator(communicator),
    _logger(_communicator->getLogger()),
    _properties(_communicator->getProperties()),
    _verifier(verifier)
{
    assert(_verifier);

    _traceLevel = _properties->getPropertyAsInt("Glacier.Starter.Trace");

    // Set up the Certificate Generation context
    string country = _properties->getPropertyWithDefault("Glacier.Starter.Certificate.Country", "US");
    string stateProv = _properties->getPropertyWithDefault("Glacier.Starter.Certificate.StateProvince", "DC");
    string locality = _properties->getPropertyWithDefault("Glacier.Starter.Certificate.Locality", "Washington");
    string org = _properties->getPropertyWithDefault("Glacier.Starter.Certificate.Organization", "Some Company Inc.");
    string orgUnit = _properties->getPropertyWithDefault("Glacier.Starter.Certificate.OranizationalUnit", "Sales");
    string commonName = _properties->getPropertyWithDefault("Glacier.Starter.Certificate.CommonName", "John Doe");

    Int bitStrength = _properties->getPropertyAsIntWithDefault(
	"Glacier.Starter.Certificate.BitStrength", 1024);
    Int secondsValid = _properties->getPropertyAsIntWithDefault(
	"Glacier.Starter.Certificate.SecondsValid",
	static_cast<Int>(IceSSL::RSACertificateGenContext::daysToSeconds(1)));
    Int issuedAdjust = _properties->getPropertyAsIntWithDefault("Glacier.Starter.Certificate.IssuedAdjust", 0);
    
    _certContext.setCountry(country);
    _certContext.setStateProvince(stateProv);
    _certContext.setLocality(locality);
    _certContext.setOrganization(org);
    _certContext.setOrgainizationalUnit(orgUnit);
    _certContext.setCommonName(commonName);
    _certContext.setBitStrength(bitStrength);
    _certContext.setSecondsValid(secondsValid);
    _certContext.setSecondsValid(issuedAdjust);
}

static bool
prefixOK(const string& property)
{
    if(property.find("--Ice.") == 0)
    {
	return false;
    }
    if(property.find("--IceSSL.") == 0)
    {
	return false;
    }
    if(property.find("--Glacier.Router") == 0)
    {
	return false;
    }

    return true;
}

void
Glacier::StarterI::destroy()
{
    //
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    //
    _communicator = 0;
    _logger = 0;
    _properties = 0;
}

Glacier::RouterPrx
Glacier::StarterI::startRouter(const string& userId, const string& password, ByteSeq& privateKey, ByteSeq& publicKey,
			       ByteSeq& routerCert, const Current&)
{
    assert(_communicator); // Destroyed?

    string reason;
    if(!_verifier->checkPermissions(userId, password, reason))
    {
	PermissionDeniedException ex;
	ex.reason = reason;
	throw ex;
    }

    bool sslConfigured = !_properties->getProperty("IceSSL.Server.Config").empty();

    //
    // routerPrivateKeyBase64 and routerCertificateBase64 are passed to the
    // router as the values for the properties
    //  * IceSSL.Server.Overrides.Server.RSA.PrivateKey
    //  * IceSSL.Server.Overrides.Server.RSA.Certificate
    // respectively.
    //
    // If the router is to act as a client to the Client as well, then
    // these values should also be passed into the router as the properties
    //  * IceSSL.Client.Overrides.RSA.PrivateKey
    //  * IceSSL.Client.Overrides.RSA.Certificate
    // respectively.
    //
    // The value of clientCertificateBase64 should be passed in to the router
    // in the property
    //  * Glacier.Router.AcceptCert
    //
    string routerPrivateKeyBase64;
    string routerCertificateBase64;
    string clientCertificateBase64;
    
    if(sslConfigured)
    {
        //
        // Create a certificate for the client and the router.
        //
        RSAKeyPairPtr clientKeyPair = _certificateGenerator.generate(_certContext);
        RSAKeyPairPtr routerKeyPair = _certificateGenerator.generate(_certContext);

        clientKeyPair->keyToByteSeq(privateKey);
        clientKeyPair->certToByteSeq(publicKey);
        routerKeyPair->certToByteSeq(routerCert);

        routerKeyPair->keyToBase64(routerPrivateKeyBase64);
        routerKeyPair->certToBase64(routerCertificateBase64);
        clientKeyPair->certToBase64(clientCertificateBase64);
    }

    string path = _properties->getPropertyWithDefault("Glacier.Starter.RouterPath", "glacierrouter");
    string uuid = IceUtil::generateUUID();

#ifdef _WIN32
    //
    // Get the absolute pathname of the executable.
    //
    char absbuf[_MAX_PATH];
    char* filePart;
    if(SearchPath(NULL, path.c_str(), ".exe", _MAX_PATH, absbuf, &filePart) == 0)
    {
        CannotStartRouterException ex;
        ex.reason = "cannot convert `" + path + "' into an absolute path";
        throw ex;
    }
    path = absbuf;

    //
    // Create a named pipe that allows the router to send its proxy to the starter.
    // We use the UUID as the pipe name.
    //
    HANDLE pipe = NULL;
    HANDLE event = NULL;
    try
    {
        //
        // An event object is necessary for using overlapped I/O, which we need
        // in order to have a timeout for router startup.
        //
        event = CreateEvent(NULL, TRUE, FALSE, NULL);
        if(event == NULL)
        {
            SyscallException ex(__FILE__, __LINE__);
            ex.error = getSystemErrno();
            throw ex;
        }

        //
        // Windows 9x/ME does not allow colons in a pipe name, so we ensure
        // our UUID does not have any.
        //
        string pipeName = "\\\\.\\pipe\\" + uuid;
        string::size_type pos;
        while((pos = pipeName.find(':')) != string::npos)
        {
            pipeName[pos] = '-';
        }

        pipe = CreateNamedPipe(
            pipeName.c_str(),                                      // Name
            PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,            // Read-only, overlapped
            PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, // Pipe mode
            1,                                                     // Instances allowed
            1024,                                                  // Output buffer size
            1024,                                                  // Input buffer size
            NMPWAIT_USE_DEFAULT_WAIT,                              // Client time out
            NULL);                                                 // No security attributes

        if(pipe == INVALID_HANDLE_VALUE)
        {
            SyscallException ex(__FILE__, __LINE__);
            ex.error = getSystemErrno();
            throw ex;
        }
    }
    catch(const LocalException& ex)
    {
        if(event != NULL)
        {
            CloseHandle(event);
        }

        Error out(_logger);
        out << ex;
        ex.ice_throw();
    }
#else
    //
    // Setup the pipe between the router and starter.
    //
    int fds[2];
    try
    {
	if(pipe(fds) != 0)
	{
	    SyscallException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
	    throw ex;
	}
    }
    catch(const LocalException& ex)
    {
	Error out(_logger);
	out << ex;
	ex.ice_throw();
    }
#endif

    //
    // Setup arguments to start the router with.
    //
    StringSeq args = _properties->getCommandLineOptions();

    //
    // Filter all arguments that don't start with "--Ice.",
    // "--IceSSL.", or "--Glacier.Router.".
    //
    args.erase(remove_if(args.begin(), args.end(), prefixOK), args.end());

    args.push_back("--Glacier.Router.Identity=" + uuid);

    //
    // TODO: Potential security risk, command line parameters can
    // be seen with `ps'. Keys and certificate should rather be
    // passed through a pipe? (ML will take care of this...)
    //
    if(sslConfigured)
    {
	args.push_back("--IceSSL.Server.Overrides.RSA.PrivateKey=" + routerPrivateKeyBase64);
	args.push_back("--IceSSL.Server.Overrides.RSA.Certificate=" + routerCertificateBase64);
	args.push_back("--IceSSL.Client.Overrides.RSA.PrivateKey=" + routerPrivateKeyBase64);
	args.push_back("--IceSSL.Client.Overrides.RSA.Certificate=" + routerCertificateBase64);
	args.push_back("--Glacier.Router.AcceptCert=" + clientCertificateBase64);
    }

    args.push_back("--Glacier.Router.UserId=" + userId);
	
    int addUserMode = _properties->getPropertyAsIntWithDefault("Glacier.Starter.AddUserToAllowCategories", 0);
    if(addUserMode == 1)
    {
	// Add user id to allowed categories.
	args.push_back("--Glacier.Router.AllowCategories=" +
		       _properties->getProperty("Glacier.Router.AllowCategories") + " " + userId);
    }
    else if(addUserMode == 2)
    {
	// Add user id with prepended underscore to allowed categories.
	args.push_back("--Glacier.Router.AllowCategories=" +
		       _properties->getProperty("Glacier.Router.AllowCategories") + " _" + userId);
    }

#ifdef _WIN32
    //
    // On Windows, the PrintProxyOnFd property is just a flag to signal that
    // the router should write its proxy to the named pipe.
    //
    args.push_back("--Glacier.Router.PrintProxyOnFd=1");
#else
    ostringstream s;
    s << "--Glacier.Router.PrintProxyOnFd=" << fds[1];
    args.push_back(s.str());
#endif
    string override = _properties->getProperty("Glacier.Starter.PropertiesOverride");
    if(!override.empty())
    {
	string::size_type end = 0;
	while(end != string::npos)
	{
	    const string delim = " \t\r\n";
		
	    string::size_type beg = override.find_first_not_of(delim, end);
	    if(beg == string::npos)
	    {
		break;
	    }
		
	    end = override.find_first_of(delim, beg);
	    string arg;
	    if(end == string::npos)
	    {
		arg = override.substr(beg);
	    }
	    else
	    {
		arg = override.substr(beg, end - beg);
	    }
	    if(arg.find("--") != 0)
	    {
		arg = "--" + arg;
	    }
	    args.push_back(arg);
	}
    }

#ifdef _WIN32
    //
    // Compose command line.
    //
    string cmd = path;
    StringSeq::const_iterator p;
    for(p = args.begin(); p != args.end(); ++p)
    {
        cmd.push_back(' ');

        //
        // Enclose arguments containing spaces in double quotes.
        //
        if((*p).find_first_of(" \t\n\r") != string::npos)
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
    // Make a copy of the command line.
    //
    char* cmdbuf = strdup(cmd.c_str());

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    HANDLE router = NULL;

    try
    {
        BOOL b;

        OVERLAPPED ol;
        ZeroMemory(&ol, sizeof(ol));
        ol.hEvent = event;

        //
        // "Connect" to named pipe. This operation won't complete until the
        // client connects to the pipe with CreateFile(), so we expect
        // ConnectNamedPipe() to return false.
        //
        b = ConnectNamedPipe(pipe, &ol);
        if(b || GetLastError() != ERROR_IO_PENDING)
        {
            CannotStartRouterException ex;
            ex.reason = "unexpected result connecting to named pipe";
            throw ex;
        }

        if(_traceLevel >= 2)
        {
            Trace out(_logger, "Glacier");
            out << "creating new router:\n" << cmdbuf;
        }

        //
        // Start the router.
        //
        b = CreateProcess(
            NULL,                     // Executable
            cmdbuf,                   // Command line
            NULL,                     // Process attributes
            NULL,                     // Thread attributes
            FALSE,                    // Inherit handles
            CREATE_NEW_PROCESS_GROUP, // Process creation flags
            NULL,                     // Process environment
            NULL,                     // Current directory
            &si,                      // Startup info
            &pi                       // Process info
        );

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
        router = pi.hProcess;

        //
        // Get the startup timeout.
        //
        DWORD timeout = _properties->getPropertyAsIntWithDefault("Glacier.Starter.StartupTimeout", 10);
        if(timeout < 1)
        {
            timeout = 1;
        }
        timeout *= 1000; // milliseconds

        //
        // Wait for the router to connect to the pipe.
        //
        while(true)
        {
            DWORD result = WaitForSingleObject(event, timeout);
            if(result == WAIT_OBJECT_0)
            {
                break;
            }
            else if(result == WAIT_TIMEOUT)
            {
                CannotStartRouterException ex;
                ex.reason = "timeout while starting `" + path + "'";
                throw ex;
            }
            else
            {
                SyscallException ex(__FILE__, __LINE__);
                ex.error = getSystemErrno();
                throw ex;
            }
        }

        //
        // Read the output from the child.
        //
        string output;
        while(true)
        {
            char buff[1024];
            DWORD count;
            b = ReadFile(pipe, buff, 1024, &count, NULL);
            if(count == 0 || (!b && GetLastError() == ERROR_BROKEN_PIPE))
            {
                break;
            }
            if(!b)
            {
                SyscallException ex(__FILE__, __LINE__);
                ex.error = getSystemErrno();
                throw ex;
            }
            output.append(buff, 0, count);
        }

        if(output.find(uuid) == 0)
        {
            //
            // We got the stringified router proxy.
            //
            RouterPrx prx = RouterPrx::uncheckedCast(_communicator->stringToProxy(output));

            if(_traceLevel >= 2)
            {
                Trace out(_logger, "Glacier");
                out << "started new router:\n" << _communicator->proxyToString(prx);
            }

            CloseHandle(router);
            CloseHandle(pipe);
            CloseHandle(event);

            return prx;
        }
        else
        {
            //
            // We got something else.
            //
            CannotStartRouterException ex;
            ex.reason = output;
            throw ex;
        }
    }
    catch(const Exception& ex)
    {
        if(router != NULL)
        {
            CloseHandle(router);
        }
        CloseHandle(pipe);
        CloseHandle(event);

        Error out(_logger);
        out << ex;
        ex.ice_throw();
    }
#else
    //
    // Convert to standard argc/argv.
    //
    int argc = static_cast<int>(args.size()) + 1;
    char** argv = static_cast<char**>(malloc((argc + 1) * sizeof(char*)));
    StringSeq::iterator p;
    int i;
    for(p = args.begin(), i = 1; p != args.end(); ++p, ++i)
    {
	assert(i < argc);
	argv[i] = strdup(p->c_str());
    }
    assert(i == argc);
    argv[0] = strdup(path.c_str());
    argv[argc] = 0;
    
    //
    // Start a router.
    //
    pid_t pid = -1; // Initialize to keep the compiler happy.
    try
    {
	pid = fork();

	if(pid == -1)
	{
	    close(fds[0]);
	    close(fds[1]);
	    SyscallException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
	    throw ex;
	}
    }
    catch(const LocalException& ex)
    {
	Error out(_logger);
	out << ex;
	ex.ice_throw();
    }

    if(pid == 0) // Child process.
    {
	//
	// Until exec, we can only use async-signal safe functions
	//

#ifdef __linux
	//
	// Create a process group for this child, to be able to send 
	// a signal to all the thread-processes with killpg
	//
	setpgrp();
#endif

	//
	// Close all filedescriptors, except for standard input,
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
	// Try to start the router.
	//
	if(execvp(argv[0], argv) == -1)
	{
	    //
	    // Send any errors to the parent process, using the write
	    // end of the pipe.
	    //
	    int err = errno;
	    char msg[500];
	    strcpy(msg,  "can't execute `");
	    strcat(msg, argv[0]);
	    strcat(msg, "':  ");
	    strcat(msg, strerror(err));

	    write(fds[1], msg, strlen(msg));
	    close(fds[1]);

	    //
	    // _exit instead of exit to avoid interferences with
	    // the parent process.
	    //
	    _exit(EXIT_FAILURE);
	}
    }
    else // Parent process.
    {
	//
	// Close the write side of the newly created pipe.
	//
	close(fds[1]);

	try
	{
	    //
	    // Wait until data can be read from the newly started router,
	    // with timeout.
	    //
	    int flags = fcntl(fds[0], F_GETFL);
	    flags |= O_NONBLOCK;
	    if(fcntl(fds[0], F_SETFL, flags) == -1)
	    {
		SyscallException ex(__FILE__, __LINE__);
		ex.error = getSystemErrno();
		throw ex;
	    }
	    
	repeatSelect:
	    fd_set fdSet;
	    FD_ZERO(&fdSet);
	    FD_SET(fds[0], &fdSet);
	    struct timeval tv;
	    tv.tv_sec = _properties->getPropertyAsIntWithDefault("Glacier.Starter.StartupTimeout", 10);
	    if(tv.tv_sec < 1)
	    {
		tv.tv_sec = 1; // One second is minimum.
	    }
	    tv.tv_usec = 0;
	    int ret = ::select(fds[0] + 1, &fdSet, 0, 0, &tv);
	    
	    if(ret == -1)
	    {
		if(errno == EINTR)
		{
		    goto repeatSelect;
		}
		
		SyscallException ex(__FILE__, __LINE__);
		ex.error = getSystemErrno();
		throw ex;
	    }
	    
	    if(ret == 0) // Timeout.
	    {
		CannotStartRouterException ex;
		ex.reason = "timeout while starting `" + path + "'";
		throw ex;
	    }

	    assert(FD_ISSET(fds[0], &fdSet));
	
	    //
	    // Read the response.
	    //
	    char buf[4*1024];
	    ssize_t sz = read(fds[0], buf, sizeof(buf)/sizeof(char) - 1);
	    if(sz == -1)
	    {
		SyscallException ex(__FILE__, __LINE__);
		ex.error = getSystemErrno();
		throw ex;
	    }

	    if(sz == 0) // EOF?
	    {
		CannotStartRouterException ex;
		ex.reason = "got EOF from `" + path + "'";
		throw ex;
	    }

	    buf[sz] = '\0'; // Terminate the string we got back.

	    if(strncmp(buf, uuid.c_str(), uuid.length()) == 0)
	    {
		close(fds[0]);

		//
		// We got the stringified router proxy.
		//
		RouterPrx router = RouterPrx::uncheckedCast(_communicator->stringToProxy(buf));

		if(_traceLevel >= 2)
		{
		    Trace out(_logger, "Glacier");
		    out << "started new router:\n" << _communicator->proxyToString(router);
		}

		return router;
	    }
	    else
	    {
		//
		// We got something else.
		//
		CannotStartRouterException ex;
		ex.reason = buf;
		throw ex;
	    }
	}
	catch(const CannotStartRouterException& ex)
	{
	    close(fds[0]);

	    if(_traceLevel >= 1)
	    {
		Trace out(_logger, "Glacier");
		out << "router starter exception:\n" << ex << ":\n" << ex.reason;
	    }
	    
	    ex.ice_throw();
	}
	catch(const Exception& ex)
	{
	    close(fds[0]);

	    Error out(_logger);
	    out << ex;
	    ex.ice_throw();
	}
    }
#endif

    assert(false); // Should never be reached.
    return 0; // To keep the compiler from complaining.
}

SliceChecksumDict
Glacier::StarterI::getSliceChecksums(const Current&) const
{
    return sliceChecksums();
}

Glacier::CryptPasswordVerifierI::CryptPasswordVerifierI(const map<string, string>& passwords) :
    _passwords(passwords)
{
}

bool
Glacier::CryptPasswordVerifierI::checkPermissions(
    const string& userId, const string& password, string&, const Current&) const
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, string>::const_iterator p = _passwords.find(userId);

    if(p == _passwords.end())
    {
	return false;
    }

    if(p->second.size() != 13) // Crypt passwords are 13 characters long.
    {
	return false;
    }

    char buff[14];
    string salt = p->second.substr(0, 2);
#if OPENSSL_VERSION_NUMBER >= 0x0090700fL
    DES_fcrypt(password.c_str(), salt.c_str(), buff);
#else
    des_fcrypt(password.c_str(), salt.c_str(), buff);
#endif
    return p->second == buff;
}
