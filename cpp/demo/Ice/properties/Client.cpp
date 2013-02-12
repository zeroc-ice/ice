// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Props.h>

using namespace std;
using namespace Demo;

class PropsClient : public Ice::Application
{
public:

    PropsClient();

    virtual int run(int, char*[]);

private:

    void menu();
    void show(const Ice::PropertiesAdminPrx&);
};

int
main(int argc, char* argv[])
{
    PropsClient app;
    return app.main(argc, argv, "config.client");
}

PropsClient::PropsClient() :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling)
{
}

int
PropsClient::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    PropsPrx props = PropsPrx::checkedCast(communicator()->propertyToProxy("Props.Proxy"));
    if(!props)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    Ice::PropertiesAdminPrx admin =
        Ice::PropertiesAdminPrx::checkedCast(communicator()->propertyToProxy("Admin.Proxy"));

    Ice::PropertyDict batch1;
    batch1["Demo.Prop1"] = "1";
    batch1["Demo.Prop2"] = "2";
    batch1["Demo.Prop3"] = "3";

    Ice::PropertyDict batch2;
    batch2["Demo.Prop1"] = "10";
    batch2["Demo.Prop2"] = ""; // An empty value removes this property
    batch2["Demo.Prop3"] = "30";

    show(admin);
    menu();

    char c;
    do
    {
        try
        {
            cout << "==> ";
            cin >> c;
            if(c == '1' || c == '2')
            {
                Ice::PropertyDict dict = c == '1' ? batch1 : batch2;
                cout << "Sending:" << endl;
                for(Ice::PropertyDict::iterator p = dict.begin(); p != dict.end(); ++p)
                {
                    if(p->first.find("Demo") == 0)
                    {
                        cout << "  " << p->first << "=" << p->second << endl;
                    }
                }
                cout << endl;

                admin->setProperties(dict);

                cout << "Changes:" << endl;
                Ice::PropertyDict changes = props->getChanges();
                if(changes.empty())
                {
                    cout << "  None." << endl;
                }
                else
                {
                    for(Ice::PropertyDict::iterator p = changes.begin(); p != changes.end(); ++p)
                    {
                        cout << "  " << p->first;
                        if(p->second.empty())
                        {
                            cout << " was removed" << endl;
                        }
                        else
                        {
                            cout << " is now " << p->second << endl;
                        }
                    }
                }
            }
            else if(c == 'c')
            {
                show(admin);
            }
            else if(c == 's')
            {
                props->shutdown();
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
PropsClient::menu()
{
    cout << endl <<
        "usage:\n"
        "1: set properties (batch 1)\n"
        "2: set properties (batch 2)\n"
        "c: show current properties\n"
        "s: shutdown server\n"
        "x: exit\n"
        "?: help\n";
}

void
PropsClient::show(const Ice::PropertiesAdminPrx& admin)
{
    Ice::PropertyDict props = admin->getPropertiesForPrefix("Demo");
    cout << "Server's current settings:" << endl;
    for(Ice::PropertyDict::iterator p = props.begin(); p != props.end(); ++p)
    {
        cout << "  " << p->first << "=" << p->second << endl;
    }
}
