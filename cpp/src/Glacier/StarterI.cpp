// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifdef _WIN32
#   error Sorry, the Glacier Starter is not yet supported on WIN32.
#endif

#include <IceUtil/UUID.h>
#include <IceSSL/RSAKeyPair.h>
#include <Glacier/StarterI.h>
#include <fcntl.h>
#include <shadow.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

using IceSSL::OpenSSL::RSAKeyPairPtr;

Glacier::StarterI::StarterI(const CommunicatorPtr& communicator, const PasswordVerifierPrx& verifier) :
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
	"Glacier.Starter.Certificate.SecondsValid", IceSSL::OpenSSL::RSACertificateGenContext::daysToSeconds(1));
    
    _certContext.setCountry(country);
    _certContext.setStateProvince(stateProv);
    _certContext.setLocality(locality);
    _certContext.setOrganization(org);
    _certContext.setOrgainizationalUnit(orgUnit);
    _certContext.setCommonName(commonName);
    _certContext.setBitStrength(bitStrength);
    _certContext.setSecondsValid(secondsValid);
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

    if(!_verifier->checkPassword(userId, password))
    {
	throw InvalidPasswordException();
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

    //
    // Start a router.
    //
    string path = _properties->getPropertyWithDefault("Glacier.Starter.RouterPath", "glacierrouter");
    string uuid = IceUtil::generateUUID();
    pid_t pid;
    int fds[2];

    try
    {
	if(pipe(fds) != 0)
	{
	    SystemException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
	    throw ex;
	}
	pid = fork();
	if(pid == -1)
	{
	    SystemException ex(__FILE__, __LINE__);
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
	// Setup arguments to start the router with.
	//
	StringSeq args = _properties->getCommandLineOptions();
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
	
	if(!_properties->getProperty("Glacier.Starter.AddUserToAllowCategories").empty())
	{
	    args.push_back("--Glacier.Router.AllowCategories=" +
			   _properties->getProperty("Glacier.Router.AllowCategories") + " " + userId);
	}

	ostringstream s;
	s << "--Glacier.Router.PrintProxyOnFd=" << fds[1];
	args.push_back(s.str());
	string overwrite = _properties->getProperty("Glacier.Starter.PropertiesOverwrite");
	if(!overwrite.empty())
	{
	    string::size_type end = 0;
	    while(end != string::npos)
	    {
		static const string delim = " \t\r\n";
		
		string::size_type beg = overwrite.find_first_not_of(delim, end);
		if(beg == string::npos)
		{
		    break;
		}
		
		end = overwrite.find_first_of(delim, beg);
		string arg;
		if(end == string::npos)
		{
		    arg = overwrite.substr(beg);
		}
		else
		{
		    arg = overwrite.substr(beg, end - beg);
		}
		if(arg.find("--") != 0)
		{
		    arg = "--" + arg;
		}
		args.push_back(arg);
	    }
	}

/*
        StringSeq::iterator seqElem = args.begin();
        while(seqElem != args.end())
        {
            cerr << *seqElem << endl;
            seqElem++;
        }
*/
	
	//
	// Convert to standard argc/argv.
	//
	int argc = args.size() + 1;
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
	// Try to start the router.
	//
	if(execvp(argv[0], argv) == -1)
	{
	    //
	    // Send any errors to the parent process, using the write
	    // end of the pipe.
	    //
	    string msg = "can't execute `" + path + "': " + strerror(errno);
	    write(fds[1], msg.c_str(), msg.length());
	    close(fds[1]);
	    exit(EXIT_FAILURE);
	}
    }
    else // Parent process.
    {
	try
	{
	    //
	    // Close the write side of the newly created pipe.
	    //
	    close(fds[1]);
	    
	    //
	    // Wait until data can be read from the newly started router,
	    // with timeout.
	    //
	    int flags = fcntl(fds[0], F_GETFL);
	    flags |= O_NONBLOCK;
	    if(fcntl(fds[0], F_SETFL, flags) == -1)
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
		
		SystemException ex(__FILE__, __LINE__);
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
		SystemException ex(__FILE__, __LINE__);
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
	    if(_traceLevel >= 1)
	    {
		Trace out(_logger, "Glacier");
		out << "router starter exception:\n" << ex << ":\n" << ex.reason;
	    }
	    
	    ex.ice_throw();
	}
	catch(const Exception& ex)
	{
	    Error out(_logger);
	    out << ex;
	    ex.ice_throw();
	}
    }

    assert(false); // Should never be reached.
    return 0; // To keep the compiler from complaining.
}

Glacier::CryptPasswordVerifierI::CryptPasswordVerifierI(const map<string, string>& passwords) :
    _passwords(passwords)
{
}

bool
Glacier::CryptPasswordVerifierI::checkPassword(const string& userId, const string& password, const Current&)
{
    map<string, string>::const_iterator p = _passwords.find(userId);

    if(p == _passwords.end())
    {
	return false;
    }

    if(p->second.size() != 13) // Crypt passwords are 13 characters long.
    {
	return false;
    }

    {
	IceUtil::Lock<IceUtil::Mutex> sync(*this); // Need a lock as crypt() is not reentrant.
	
	return p->second == crypt(password.c_str(), p->second.substr(0, 2).c_str());
    }
}

void
Glacier::CryptPasswordVerifierI::destroy(const Current&)
{
    // Nothing to do.
}
