// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

int
main(int argc, char* argv[])
{
    int status = EXIT_SUCCESS;
    Ice::CommunicatorPtr communicator;
 
    string envName = "db";

    communicator = Ice::initialize(argc, argv);
    if(argc != 1)
    {
        envName = argv[1];
        envName += "/db";
    }
    
    {
        Freeze::ConnectionPtr connection;
        try
        {
            connection = Freeze::createConnection(communicator, envName);
            test(true);
        }
        catch(const exception& ex)
        {
            cerr << "excetpion:\n" << ex.what() << endl;
            test(false);
        }
        catch(...)
        {
            test(false);
        }
        
        cout << "File lock acquired.\n" 
            << "Enter some input and press enter, to release the lock and terminate the program." << endl;
        //
        // Block the test waiting for IO, so the file lock is preserved.
        //
        string foo;
        cin >> foo;

        //
        // Clean up.
        //
        if(connection)
        {
            connection->close();
        }
    }
    cout << "File lock released." << endl;
    
    try
    {
        communicator->destroy();
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }
    return status;
}
