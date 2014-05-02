// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Client.h>
#include <Greet.h>

using namespace std;
using namespace Demo;

void
menu()
{
    cout <<
        "usage:\n"
        "t: send greeting\n"
        "s: shutdown server\n"
        "x: exit\n"
        "?: help\n";
}

string
decodeString(const string& str)
{
    ostringstream result;
    for(string::const_iterator p = str.begin(); p != str.end(); ++p)
    {
        if(*p >= 32 && *p <= 126)
        {
            result << *p;
        }
        else
        {
            result << "\\"
                   << oct << static_cast<unsigned int>(static_cast<unsigned char>(*p));
        }
    }
    return result.str();
}

int
Demo::Client::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << argv[0] << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    const string proxyProperty = "Greet.Proxy";
    GreetPrx greet = GreetPrx::checkedCast(communicator()->propertyToProxy(proxyProperty));
    if(!greet)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    menu();

    string greeting = "Bonne journ\351e";

    char c;
    do
    {
        try
        {
            cout << "==> ";
            cin >> c;
            if(c == 't')
            {
                string ret = greet->exchangeGreeting(greeting);
                cout << "Received: \"" << decodeString(ret) << '\"' << endl;
            }
            else if(c == 's')
            {
                greet->shutdown();
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
