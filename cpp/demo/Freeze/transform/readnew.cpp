// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include <NewContacts.h>

using namespace std;
using namespace Demo;
using namespace Freeze;

class ReadNew : public Ice::Application
{
public:
    
    virtual int run(int, char*[]);
};

int main(int argc, char* argv[])
{
    ReadNew app;
    return app.main(argc, argv);
}

int
ReadNew::run(int, char*[])
{
    ConnectionPtr connection = createConnection(communicator(), "dbnew");
    bool createDb = true;
    const NewContacts contacts(connection, "contacts", createDb);
    
    NewContacts::const_iterator p;

    cout << "All contacts (default order)" << endl;
    for(p = contacts.begin(); p != contacts.end(); ++p)
    {
        cout << p->first << ":\t\t" << p->second.phoneNumber
             << " " << p->second.emailAddress << endl;
    }
    
    cout << endl << "All contacts (ordered by phone number)" << endl;
    for(p = contacts.beginForPhoneNumber(); p != contacts.endForPhoneNumber(); ++p)
    {
        cout << p->first << ":\t\t" << p->second.phoneNumber
             << " " << p->second.emailAddress << endl;
    }
    
    return 0;
}
