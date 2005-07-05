// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <HelloI.h>

using namespace std;

int
run(int argc, char* argv[], const IceE::CommunicatorPtr& communicator)
{
    IceE::ObjectAdapterPtr adapter = communicator->createObjectAdapter("Hello");
    IceE::ObjectPtr object = new HelloI;
    adapter->add(object, IceE::stringToIdentity("hello"));
    adapter->activate();
    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    int argc = 0;
    char** argv = 0;

#else

int
main(int argc, char* argv[])
{

#endif

    int status;
    IceE::CommunicatorPtr communicator;

    try
    {
	IceE::PropertiesPtr properties = IceE::createProperties();
        properties->load("config");
	communicator = IceE::initializeWithProperties(argc, argv, properties);
	status = run(argc, argv, communicator);
    }
    catch(const IceE::Exception& ex)
    {
	fprintf(stderr, "%s\n", ex.toString().c_str());
	status = EXIT_FAILURE;
    }

    if(communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const IceE::Exception& ex)
	{
	    fprintf(stderr, "%s\n", ex.toString().c_str());
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
