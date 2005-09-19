// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

class HelloClient : public Ice::Application
{
public:

    virtual int run(int, char*[]);

private:

    void menu();
};

int
main(int argc, char* argv[])
{
    HelloClient app;
    return app.main(argc, argv, "config");
}

void
HelloClient::menu()
{
    cout <<
	"usage:\n"
	"t: send greeting\n"
	"s: shutdown server\n"
	"x: exit\n"
	"?: help\n";
}

int
HelloClient::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();

    Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
    args = properties->parseCommandLineOptions("", args);
    Ice::stringSeqToArgs(args, argc, argv);

    Ice::ObjectPrx base = communicator()->stringToProxy(properties->getPropertyWithDefault("Identity", "hello"));
    HelloPrx twoway = HelloPrx::checkedCast(base);
    if(!twoway)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }
    
    menu();

    char c;
    do
    {
	try
	{
	    cout << "==> ";
	    cin >> c;
	    if(c == 't')
	    {
		twoway->sayHello();
	    }
	    else if(c == 's')
	    {
		twoway->shutdown();
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
		cout << "unknown command `" << c << "'" << endl;
		menu();
	    }
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	}
    }
    while(cin.good() && c != 'x');

    return EXIT_SUCCESS;
}

