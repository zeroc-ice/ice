// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Queue.h>

using namespace std;
using namespace Demo;

class QueuePublisher : public Ice::Application
{
public:

    virtual int run(int, char*[]);

private:

    void menu();
    string trim(const string& s);
};

int
main(int argc, char* argv[])
{
    QueuePublisher app;
    return app.main(argc, argv, "config.client");
}

int
QueuePublisher::run(int argc, char* argv[])
{
    QueuePrx queue = QueuePrx::checkedCast(communicator()->propertyToProxy("Queue.Proxy"));
    if(!queue)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    cout << "Type a message and hit return to queue a message." << endl;
    menu();

    try
    {
        do
        {
            string s;
            cout << "==> ";
            getline(cin, s);
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
                    queue->add(s);
                }
            }
        }
        while(cin.good());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void
QueuePublisher::menu()
{
    cout << "Enter /quit to exit." << endl;
}

string
QueuePublisher::trim(const string& s)
{
    static const string delims = "\t\r\n ";
    string::size_type last = s.find_last_not_of(delims);
    if(last != string::npos)
    {
        return s.substr(s.find_first_not_of(delims), last+1);
    }
    return s;
}

