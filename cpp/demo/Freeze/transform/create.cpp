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
#include <Contacts.h>

using namespace std;
using namespace Demo;
using namespace Freeze;

class Create : public Ice::Application
{
public:
    
    virtual int run(int, char*[]);
};

int main(int argc, char* argv[])
{
    Create app;
    return app.main(argc, argv);
}

int
Create::run(int, char*[])
{
    const string names[] = { "don", "ed", "frank", "gary", "arnold", "bob", "carlos" };
    const string phoneNumbers[] = { "(777)777-7777", "(666)666-6666", "(555)555-5555 x123", 
                                    "(444)444-4444", "(333)333-3333 x1234", "(222)222-2222", "(111)111-1111" };
    const size_t size = 7;


    ConnectionPtr connection = createConnection(communicator(), "db");
    Contacts contacts(connection, "contacts");
    
  
    //
    // Create a bunch of contacts within one transaction, and commit it
    //
  
    TransactionHolder txh(connection);
    for(size_t i = 0; i < size; ++i)
    {
        ContactData data;
        data.phoneNumber = phoneNumbers[i];

        contacts.put(Contacts::value_type(names[i], data));
    }
    
    txh.commit();
            
    cout << size << " contacts were successfully created or updated" << endl;
    
    return 0;
}
