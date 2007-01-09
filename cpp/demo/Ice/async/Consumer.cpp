// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <IceUtil/UUID.h>
#include <Queue.h>

using namespace std;
using namespace Demo;

static IceUtil::StaticMutex requestsMutex = ICE_STATIC_MUTEX_INITIALIZER;
static set<string> requests;

class AMI_Queue_getI : public AMI_Queue_get
{
public:
    AMI_Queue_getI(const string& id)
        : _id(id)
    {
        IceUtil::StaticMutex::Lock lock(requestsMutex);
	requests.insert(_id);
    }

    virtual void ice_response(const string& message)
    {
        IceUtil::StaticMutex::Lock lock(requestsMutex);
	requests.erase(_id);

        cout << message << endl;
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
        IceUtil::StaticMutex::Lock lock(requestsMutex);
	requests.erase(_id);

        cout << ex << endl;
    }

private:

    string _id;
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
    QueuePrx queue = QueuePrx::checkedCast(communicator()->propertyToProxy("Queue.Proxy"));
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
	        string id = IceUtil::generateUUID();
		queue->get_async(new AMI_Queue_getI(id), id);
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


    //
    // Cancel any outstanding requests.
    //
    /*
    IceUtil::StaticMutex::Lock lock(requestsMutex);
    if(requests.size() != 0)
    {
       try
       {
           vector<string> ids(requests.size());
	   for(set<string>::iterator p = requests.begin(); p != requests.end(); ++p)
	   {
	       ids.push_back(*p);
	   }
           queue->cancel(ids);
       }
       catch(const Ice::Exception&)
       {
           // Ignore
       }
    }
    */

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
