// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Queue.h>

using namespace std;
using namespace Demo;

class AMI_Queue_getI : public AMI_Queue_get
{
public:

    virtual void ice_response(const string& message)
    {
        cout << message << endl;
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
        cout << ex << endl;
    }
};

class QueueConsumer : public Ice::Application
{
public:

    virtual int run(int, char*[]);

private:

    void menu();
};

int
main(int argc, char* argv[])
{
    QueueConsumer app;
    return app.main(argc, argv, "config.client");
}

int
QueueConsumer::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();
    const char* proxyProperty = "Queue.Proxy";
    string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << argv[0] << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    QueuePrx queue = QueuePrx::checkedCast(communicator()->stringToProxy(proxy));
    if(!queue)
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
	    if(c == 'g')
	    {
		queue->get_async(new AMI_Queue_getI);
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

void
QueueConsumer::menu()
{
    cout <<
	"usage:\n"
	"g: get a message\n"
	"x: exit\n"
	"?: help\n";
}
