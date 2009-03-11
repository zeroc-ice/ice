// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Nrvo.h>
#include <MyStringSeq.h>
#include <iomanip>

using namespace std;
using namespace Demo;

class NrvoClient : public Ice::Application
{
public:

    NrvoClient();
    virtual int run(int, char*[]);

private:

    void menu();
};

int
main(int argc, char* argv[])
{
    NrvoClient app;
    return app.main(argc, argv, "config.client");
}

NrvoClient::NrvoClient()
{
}

int
NrvoClient::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    NrvoPrx nrvo = NrvoPrx::checkedCast(communicator()->propertyToProxy("Nrvo.Proxy"));
    if(!nrvo)
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

            if(c == '1' || c == '2' || c == '3' || c == 's' || c == '?' || c == 'x')
            {
                switch(c)
                {
                    case '1':
                    {
                        cout << "calling op1" << endl;
                        MyStringSeq seq = nrvo->op1();
                        break;
                    }

                    case '2':
                    {
                        cout << "calling op2" << endl;
                        MyStringSeq seq = nrvo->op2();
                        break;
                    }

                    case '3':
                    {
                        cout << "calling op3" << endl;
                        MyStringSeq seq = nrvo->op3(10);
                        break;
                    }

                    case 's':
                    {
                        nrvo->shutdown();
                        break;
                    }

                    case '?':
                    {
                        menu();
                        break;
                    }

                    case 'x':
                    {
                        break;
                    }
                }
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
NrvoClient::menu()
{
    cout <<
        "usage:\n"
        "\n"
        "Operation to call:\n"
        "1: return a string sequence\n"
        "2: return a string sequence that is a data member of the servant\n"
        "3: return a string sequence from an operation with multiple return paths\n"
        "s: shutdown server\n"
        "x: exit\n"
        "?: show this menu\n";
}
