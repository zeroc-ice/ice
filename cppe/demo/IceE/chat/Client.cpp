// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <PingThread.h>
#include <Router.h>
#include <Chat.h>
#include <IceE/IceE.h>

#ifdef ICEE_HAS_ROUTER

using namespace std;
using namespace Demo;

class ChatCallbackI : public ChatCallback
{
public:

    virtual void
    message(const string& data, const Ice::Current&)
    {
	printf("%s\n", data.c_str());
    }
};

void
menu()
{
    printf("enter /quit to exit.\n");
}

string
trim(const string& s)
{
    static const string delims = "\t\r\n ";
    string::size_type last = s.find_last_not_of(delims);
    if(last != string::npos)
    {
        return s.substr(s.find_first_not_of(delims), last+1);
    }
    return s;
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::RouterPrx defaultRouter = communicator->getDefaultRouter();
    if(!defaultRouter)
    {
        fprintf(stderr, "%s: no default router set\n", argv[0]);
        return EXIT_FAILURE;
    }

    Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(defaultRouter);
    {
        if(!router)
        {
	    fprintf(stderr, "%s: configured router is not a Glacier2 router\n", argv[0]);
    	    return EXIT_FAILURE;
        }
    }

    char buffer[1024];

    ChatSessionPrx session;
    while(true)
    {
	printf("This demo accepts any user-id / password combination.\n");

        printf("user id: ");
	fgets(buffer, 1024, stdin);
	string id(buffer);
        id = trim(id);

        printf("password: ");
	fgets(buffer, 1024, stdin);
	string pw(buffer);
        pw = trim(pw);

        try
        {
	    session = ChatSessionPrx::uncheckedCast(router->createSession(id, pw));
	    break;
        }
        catch(const Glacier2::PermissionDeniedException& ex)
        {
	    fprintf(stderr, "permission denied:\n%s", ex.toString().c_str()); 
        }
    }

    SessionPingThreadPtr ping = new SessionPingThread(session, (long)router->getSessionTimeout() / 2);
    ping->start();

    string category = router->getServerProxy()->ice_getIdentity().category;
    Ice::Identity callbackReceiverIdent;
    callbackReceiverIdent.name = "callbackReceiver";
    callbackReceiverIdent.category = category;

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("Chat.Client");
    ChatCallbackPrx callback = ChatCallbackPrx::uncheckedCast(
        adapter->add(new ChatCallbackI, callbackReceiverIdent));
    adapter->activate();

    session->setCallback(callback);

    menu();

    try
    {
        do
        {
	    printf("==> ");
	    char* ret = fgets(buffer, 1024, stdin);
	    if(ret == NULL)
	    {
	        break;
	    }

	    string s(buffer);
	    s = trim(s);
	    if(!s.empty())
	    {
	        if(s[0] == '/')
	        {
	    	    if(s == "/quit")
	    	    {
	    	        break;
	    	    }
	    	    menu();
	        }
	        else
	        {
	    	    session->say(s);
	        }
	    }
	}
	while(true);
	
	try
	{
	    router->destroySession();
	}
	catch(const Ice::ConnectionLostException&)
	{
	    //
	    // Expected: the router closed the connection.
	    //
	}
    }
    catch(const Ice::Exception& ex)
    {
        fprintf(stderr, "%s\n", ex.toString().c_str());
	ping->destroy();
	ping->getThreadControl().join();
        return EXIT_FAILURE;
    }

    ping->destroy();
    ping->getThreadControl().join();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();
        initData.properties->load("config");
        communicator = Ice::initialize(argc, argv, initData);
        status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
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
        catch(const Ice::Exception& ex)
        {
            fprintf(stderr, "%s\n", ex.toString().c_str());
            status = EXIT_FAILURE;
        }
    }

    return status;
}

#else

int
main(int argc, char* argv[])
{
    fprintf(stderr, "This demo requires Ice-E be built with router support enabled.\n");
    return -1;
}

#endif
