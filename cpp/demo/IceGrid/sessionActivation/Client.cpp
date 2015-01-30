// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

class HelloClient : public Ice::Application
{
public:

    HelloClient();
    virtual int run(int, char*[]);

private:

    void cleanup();
    void menu();
    string trim(const string&);
};

int
main(int argc, char* argv[])
{
    HelloClient app;
    return app.main(argc, argv, "config.client");
}

HelloClient::HelloClient() :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling)
{
}

int
HelloClient::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;

    IceGrid::RegistryPrx registry =
        IceGrid::RegistryPrx::checkedCast(communicator()->stringToProxy("DemoIceGrid/Registry"));
    if(!registry)
    {
        cerr << argv[0] << ": could not contact registry" << endl;
        return EXIT_FAILURE;
    }

    IceGrid::SessionPrx session;
    while(!session)
    {
        cout << "This demo accepts any user-id / password combination.\n";

        string id;
        cout << "user id: " << flush;
        getline(cin, id);
        id = trim(id);

        string password;
        cout << "password: " << flush;
        getline(cin, password);
        password = trim(password);

        try
        {
            session = registry->createSession(id, password);
            break;
        }
        catch(const IceGrid::PermissionDeniedException& ex)
        {
            cout << "permission denied:\n" << ex.reason << endl;
        }
    }

    //
    // Enable heartbeats on the session connection to maintain the
    // connection alive even if idle.
    //
    session->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::HeartbeatOnIdle);

    try
    {
        HelloPrx hello = HelloPrx::checkedCast(session->allocateObjectById(communicator()->stringToIdentity("hello")));

        menu();

        char c = 'x';
        do
        {
            try
            {
                cout << "==> ";
                cin >> c;
                if(c == 't')
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
    }
    catch(const IceGrid::AllocationException& ex)
    {
        cerr << argv[0] << ": could not allocate object: " << ex.reason << endl;
        status = EXIT_FAILURE;
    }
    catch(const IceGrid::ObjectNotRegisteredException&)
    {
        cerr << argv[0] << ": object not registered with registry" << endl;
        status = EXIT_FAILURE;
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }
    catch(...)
    {
        cerr << "unexpected exception" << endl;
        status = EXIT_FAILURE;
    }

    session->destroy();

    return status;
}

void
HelloClient::menu()
{
    cout <<
        "usage:\n"
        "t: send greeting\n"
        "x: exit\n"
        "?: help\n";
}

string
HelloClient::trim(const string& s)
{
    static const string delims = "\t\r\n ";
    string::size_type last = s.find_last_not_of(delims);
    if(last != string::npos)
    {
        return s.substr(s.find_first_not_of(delims), last+1);
    }
    return s;
}

