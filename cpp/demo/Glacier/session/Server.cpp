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
#include <Glacier/Glacier.h>
#include <HelloSessionI.h>

using namespace std;

class PasswordVerifierI : public Glacier::PasswordVerifier
{
public:

    PasswordVerifierI(const Ice::ObjectAdapterPtr& adapter) :
	_adapter(adapter)
    {
    }

    virtual bool
    checkPassword(const string& userId, const string& password, const Ice::Current&)
    {
	return userId == password;
    }
    
private:

    Ice::ObjectAdapterPtr _adapter;
};

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("HelloSessionManagerAdapter");

    //
    // Create the SessionManager.
    //
    Ice::ObjectPtr object = new HelloSessionManagerI(adapter);
    adapter->add(object, Ice::stringToIdentity("HelloSessionManager"));

    //
    // Create the PasswordVerifier.
    //
    Ice::ObjectPtr passwdVerifier = new PasswordVerifierI(adapter);
    adapter->add(passwdVerifier, Ice::stringToIdentity("PasswordVerifier"));

    adapter->activate();
    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	Ice::PropertiesPtr properties = Ice::createPropertiesFromFile(argc, argv, "config");
	communicator = Ice::initializeWithProperties(properties);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if (communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
