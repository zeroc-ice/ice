// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <IntSMap.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

int
run(const CommunicatorPtr& communicator, const string& envName, const string& dbName)
{
    ConnectionPtr connection = createConnection(communicator, envName);
    IntSMap m(connection, dbName);

    m.put(IntSMap::value_type(0, S()));

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;
 
    string envName = "db";

    try
    {
        communicator = Ice::initialize(argc, argv);
        if(argc != 1)
        {
            envName = argv[1];
            envName += "/";
            envName += "db";
        }
       
        status = run(communicator, envName, "default.db");
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

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
