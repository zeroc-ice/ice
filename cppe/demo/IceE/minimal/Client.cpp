// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

void
menu()
{
    printf("usage:\nh: say hello\nx: exit\n?: help\n");
}

int
run(int argc, char* argv[], const IceE::CommunicatorPtr& communicator)
{
    IceE::PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "Hello.Proxy";
    string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
        fprintf(stderr, "%s: property `%s' not set\n", argv[0], proxyProperty);
	return EXIT_FAILURE;
    }

    HelloPrx hello = HelloPrx::checkedCast(communicator->stringToProxy(proxy));
    if(!hello)
    {
        
        fprintf(stderr, "%s: invalid proxy\n", argv[0]);
	return EXIT_FAILURE;
    }

    menu();

    char c;
    do
    {
	try
	{
	    printf("==> ");
	    do
	    {
	    c = getchar();
	    }
	    while(c != EOF && c == '\n');
	    if(c == 'h')
	    {
		hello->sayHello();
	    }
	    else if(c == 'x')
	    {
		// Nothing to do
	    }
	    else if(c == '?')
	    {
		menu();
	    }
	    else
	    {
	        printf("unknown command `%c'\n", c);
		menu();
	    }
	}
	catch(const IceE::Exception& ex)
	{
	    fprintf(stderr, "%s\n", ex.toString().c_str());
	}
    }
    while(c != EOF && c != 'x');

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
