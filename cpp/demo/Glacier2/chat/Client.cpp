// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Glacier2/Router.h>
#include <Chat.h>

using namespace std;
using namespace Ice;
using namespace Demo;

class ChatCallbackI : public ChatCallback
{
public:

    virtual void
    message(const string& data, const Ice::Current&)
    {
	cout << data << endl;
    }
};

class CallbackClient : public Application
{
public:

    virtual int
    run(int argc, char* argv[])
    {
	RouterPrx defaultRouter = communicator()->getDefaultRouter();
	if(!defaultRouter)
	{
	    cerr << argv[0] << ": no default router set" << endl;
	    return EXIT_FAILURE;
	}

	Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(defaultRouter);
	{
	    if(!router)
	    {
		cerr << argv[0] << ": configured router is not a Glacier2 router" << endl;
		return EXIT_FAILURE;
	    }
	}

	ChatSessionPrx session;
	while(true)
	{
	    cout << "This demo accepts any user-id / password combination.\n";

	    string id;
	    cout << "user id: " << flush;
	    std::getline(cin, id);
	    id = trim(id);

	    string pw;
	    cout << "password: " << flush;
	    std::getline(cin, pw);
	    pw = trim(pw);

	    try
	    {
		session = ChatSessionPrx::uncheckedCast(router->createSession(id, pw));
		break;
	    }
	    catch(const Glacier2::PermissionDeniedException& ex)
	    {
		cout << "permission denied:\n" << ex.reason << endl;
	    }
	}

	string category = router->getServerProxy()->ice_getIdentity().category;
	Identity callbackReceiverIdent;
	callbackReceiverIdent.name = "callbackReceiver";
	callbackReceiverIdent.category = category;

	ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Chat.Client");
	ChatCallbackPrx callback = ChatCallbackPrx::uncheckedCast(
	    adapter->add(new ChatCallbackI, callbackReceiverIdent));
	adapter->activate();

	session->setCallback(callback);

	menu();
	do
	{
	    try
	    {
		string s;
		cout << "==> ";
		std::getline(cin, s);
		s = trim(s);
		if(s.empty())
		{
		    continue;
		}
		if(s[0] == '/')
		{
		    if(s == "/quit")
		    {
			session->destroy();
			break;
		    }
		    else
		    {
			menu();
		    }
		}
		else
		{
		    session->say(s);
		}
	    }
	    catch(const Exception& ex)
	    {
		cerr << ex << endl;
		return EXIT_FAILURE;
	    }
	}
	while(cin.good());

	return EXIT_SUCCESS;
    }

private:

    void
    menu()
    {
	cout << "enter /quit to exit." << endl;
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
};

int
main(int argc, char* argv[])
{
    CallbackClient app;
    return app.main(argc, argv, "config");
}


