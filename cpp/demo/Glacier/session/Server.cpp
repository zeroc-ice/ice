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
    checkPassword(const string& userId, const string& password, const Ice::Current&) const
    {
	return userId == password;
    }
    
private:

    Ice::ObjectAdapterPtr _adapter;
};

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("HelloSessionManager");

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
	Ice::PropertiesPtr properties = Ice::createProperties(argc, argv);
        properties->load("config");
	communicator = Ice::initializeWithProperties(argc, argv, properties);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if(communicator)
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
