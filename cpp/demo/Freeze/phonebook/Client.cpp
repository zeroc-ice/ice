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

#include <Ice/Application.h>
#include <Parser.h>

using namespace std;
using namespace Ice;

class PhoneBookClient : public Application
{
    virtual int run(int argc, char* argv[]);
};

int
main(int argc, char* argv[])
{
    PhoneBookClient app;
    return app.main(argc, argv, "config");
}

int
PhoneBookClient::run(int argc, char* argv[])
{
    int runParser(int, char*[], const CommunicatorPtr&);
    return runParser(argc, argv, communicator());
}
