// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Ice/RoutingTable.h>
#include <Glacier/RouterI.h>
#include <Glacier/ClientBlobject.h>
#include <Glacier/ServerBlobject.h>
#include <Glacier/SessionManager.h>
#include <IceUtil/Base64.h>
#include <IceUtil/Options.h>
#include <IceSSL/CertificateVerifierF.h>
#include <IceSSL/Plugin.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

namespace Glacier
{

class ServantLocator : public Ice::ServantLocator
{
public:

    ServantLocator(const ObjectPtr&);
    
    virtual ObjectPtr locate(const Current&, LocalObjectPtr&);
    virtual void finished(const Current&, const ObjectPtr&, const LocalObjectPtr&);
    virtual void deactivate(const string&);

private:

    ObjectPtr _blobject;
};

class RouterApp : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

};

Glacier::ServantLocator::ServantLocator(const ObjectPtr& blobject) :
    _blobject(blobject)
{
}

ObjectPtr
Glacier::ServantLocator::locate(const Current&, LocalObjectPtr&)
{
    return _blobject;
}

void
Glacier::ServantLocator::finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
{
    // Nothing to do
}

void
Glacier::ServantLocator::deactivate(const string&)
{
    ClientBlobject* clientBlobject = dynamic_cast<ClientBlobject*>(_blobject.get());
    if(clientBlobject)
    {
	clientBlobject->destroy();
    }
    
    ServerBlobject* serverBlobject = dynamic_cast<ServerBlobject*>(_blobject.get());
    if(serverBlobject)
    {
	serverBlobject->destroy();
    }
    
    _blobject = 0;
}

void
Glacier::RouterApp::usage()
{
    cerr << "Usage: " << appName() << " [options]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
Glacier::RouterApp::run(int argc, char* argv[])
{
    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    
    vector<string> args;
    try
    {
    	args = opts.parse(argc, argv);
    }
    catch(const IceUtil::Options::BadOpt& e)
    {
        cerr << e.reason << endl;
	usage();
	return EXIT_FAILURE;
    }

    if(opts.isSet("h") || opts.isSet("help"))
    {
	usage();
	return EXIT_SUCCESS;
    }
    if(opts.isSet("v") || opts.isSet("version"))
    {
	cout << ICE_STRING_VERSION << endl;
	return EXIT_SUCCESS;
    }

    if(!args.empty())
    {
	usage();
	return EXIT_FAILURE;
    }

    PropertiesPtr properties = communicator()->getProperties();

    string clientConfig = properties->getProperty("IceSSL.Client.Config");
    string serverConfig = properties->getProperty("IceSSL.Server.Config");

    //
    // Only do this if we've been configured for SSL.
    //
    if(!clientConfig.empty() && !serverConfig.empty())
    {
        IceSSL::ContextType contextType = IceSSL::ClientServer;

        //
        // Get the SSL plugin.
        //
        PluginManagerPtr pluginManager = communicator()->getPluginManager();
        PluginPtr plugin = pluginManager->getPlugin("IceSSL");
        IceSSL::PluginPtr sslPlugin = IceSSL::PluginPtr::dynamicCast(plugin);
        assert(sslPlugin);

        //
        // The plug-in must configure itself (using config files as specified).
        //
        sslPlugin->configure(contextType);

        // If we have been told only to only accept a single certificate.
        string clientCertBase64 = properties->getProperty("Glacier.Router.AcceptCert");
        if(!clientCertBase64.empty())
        {
            // Install a Certificate Verifier that only accepts indicated certificate.
            ByteSeq clientCert = IceUtil::Base64::decode(clientCertBase64);
            sslPlugin->setCertificateVerifier(contextType, sslPlugin->getSingleCertVerifier(clientCert));
        
            // Add the Client's certificate as a trusted certificate.
            sslPlugin->addTrustedCertificateBase64(contextType, clientCertBase64);
        }
    }

    //
    // Create the routing table.
    //
    IceInternal::RoutingTablePtr routingTable = new IceInternal::RoutingTable;

    //
    // Initialize the client object adapter.
    //
    const char* clientEndpointsProperty = "Glacier.Router.Client.Endpoints";
    if(properties->getProperty(clientEndpointsProperty).empty())
    {
	cerr << appName() << ": property `" << clientEndpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }
    ObjectAdapterPtr clientAdapter = communicator()->createObjectAdapter("Glacier.Router.Client");

    //
    // Initialize the server object adapter.
    //
    ObjectAdapterPtr serverAdapter;
    if(!properties->getProperty("Glacier.Router.Server.Endpoints").empty())
    {
	serverAdapter = communicator()->createObjectAdapter("Glacier.Router.Server");
    }

    //
    // Create the client and server blobjects and the associated
    // servant locators.
    //
    const char* allowCategoriesProperty = "Glacier.Router.AllowCategories";
    string allowCategories = properties->getProperty(allowCategoriesProperty);
    ObjectPtr clientBlobject = new ClientBlobject(communicator(), routingTable, allowCategories);
    Ice::ServantLocatorPtr clientServantLocator = new Glacier::ServantLocator(clientBlobject);
    clientAdapter->addServantLocator(clientServantLocator, "");
    if(serverAdapter)
    {
	ObjectPtr serverBlobject = new ServerBlobject(clientAdapter);
	Ice::ServantLocatorPtr serverServantLocator = new Glacier::ServantLocator(serverBlobject);
	serverAdapter->addServantLocator(serverServantLocator, "");
    }

    //
    // Initialize the router object adapter and the router object.
    //
    const char* routerEndpointsProperty = "Glacier.Router.Endpoints";
    if(properties->getProperty(routerEndpointsProperty).empty())
    {
	cerr << appName() << ": property `" << routerEndpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }

    const char* routerIdentityProperty = "Glacier.Router.Identity";
    string routerIdentity = properties->getPropertyWithDefault(routerIdentityProperty, "Glacier/router");

    const char* sessionManagerProperty = "Glacier.Router.SessionManager";
    string sessionManager = properties->getProperty(sessionManagerProperty);

    SessionManagerPrx sessionManagerPrx;
    if(!sessionManager.empty())
    {
	sessionManagerPrx = SessionManagerPrx::checkedCast(communicator()->stringToProxy(sessionManager));
    }

    const char* userIdProperty = "Glacier.Router.UserId";
    string userId = properties->getProperty(userIdProperty);

    ObjectAdapterPtr routerAdapter = communicator()->createObjectAdapter("Glacier.Router");
    RouterPtr router = new RouterI(clientAdapter, serverAdapter, routingTable, sessionManagerPrx, userId);
    routerAdapter->add(router, stringToIdentity(routerIdentity));

#ifdef _WIN32
    //
    // Send the stringified router proxy to a named pipe, if so requested.
    //
    string outputFd = properties->getProperty("Glacier.Router.PrintProxyOnFd");
    if(!outputFd.empty())
    {
        //
        // Windows 9x/ME does not allow colons in a pipe name, so we ensure
        // our UUID does not have any.
        //
        string pipeName = "\\\\.\\pipe\\" + routerIdentity;
        string::size_type pos;
        while((pos = pipeName.find(':')) != string::npos)
        {
            pipeName[pos] = '-';
        }

        HANDLE pipe = CreateFile(
            pipeName.c_str(), // Pipe name
            GENERIC_WRITE,    // Write access
            0,                // No sharing
            NULL,             // No security attributes
            OPEN_EXISTING,    // Opens existing pipe
            0,                // Default attributes
            NULL);            // No template file

        if(pipe == INVALID_HANDLE_VALUE)
        {
            cerr << appName() << ": cannot open pipe `" << pipeName << "' to starter" << endl;

            //
            // Destroy the router. The client and server blobjects get destroyed by ServantLocator::deactivate.
            //
            RouterI* rtr = dynamic_cast<RouterI*>(router.get());
            assert(rtr);
            rtr->destroy();

            return EXIT_FAILURE;
        }

	string ref = communicator()->proxyToString(routerAdapter->createProxy(stringToIdentity(routerIdentity)));
        string::size_type count = 0;
        while(count < ref.size())
        {
            DWORD n;
            if(!WriteFile(pipe, ref.c_str(), ref.length(), &n, NULL))
            {
                cerr << appName() << ": unable to write proxy to pipe" << endl;

                //
                // Destroy the router. The client and server blobjects get destroyed by ServantLocator::deactivate.
                //
                RouterI* rtr = dynamic_cast<RouterI*>(router.get());
                assert(rtr);
                rtr->destroy();

                return EXIT_FAILURE;
            }
            count += n;
        }

        FlushFileBuffers(pipe);
        CloseHandle(pipe);
    }
#else
    //
    // Print the stringified router proxy on a filedescriptor
    // specified in the properties, if so requested.
    //
    string outputFd = properties->getProperty("Glacier.Router.PrintProxyOnFd");
    if(!outputFd.empty())
    {
	int fd = atoi(outputFd.c_str());
	string ref = communicator()->proxyToString(routerAdapter->createProxy(stringToIdentity(routerIdentity)));
	ref += "\n";
	string::size_type sz = static_cast<string::size_type>(write(fd, ref.c_str(), ref.length()));
	if(sz != ref.length())
	{
	    cerr << appName() << ": cannot write stringified router proxy to filedescriptor " << fd << ": "
		 << strerror(errno) << endl;

	    //
	    // Destroy the router. The client and server blobjects get
	    // destroyed by ServantLocator::deactivate.
	    //
	    RouterI* rtr = dynamic_cast<RouterI*>(router.get());
	    assert(rtr);
	    rtr->destroy();

	    return EXIT_FAILURE;
	}
	close(fd);
    }
#endif

    //
    // Everything ok, let's go.
    //
    shutdownOnInterrupt();
    clientAdapter->activate();
    if(serverAdapter)
    {
	serverAdapter->activate();
    }
    routerAdapter->activate();
    communicator()->waitForShutdown();
    ignoreInterrupt();

    //
    // Destroy the router. The client and server blobjects get
    // destroyed by ServantLocator::deactivate.
    //
    // Destroying the router will also destroy all sessions associated
    // with the router.
    //
    RouterI* rtr = dynamic_cast<RouterI*>(router.get());
    assert(rtr);
    rtr->destroy();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    //
    // Make sure that this process doesn't use a router.
    //
    try
    {
	PropertiesPtr defaultProperties = getDefaultProperties(argc, argv);
	defaultProperties->setProperty("Ice.Default.Router", "");
    }
    catch(const Exception& e)
    {
	cerr << e << endl;
	exit(EXIT_FAILURE);
    }

    Glacier::RouterApp app;
    return app.main(argc, argv);
}
