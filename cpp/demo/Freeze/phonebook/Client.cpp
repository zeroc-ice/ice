// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
#include <Parser.h>

using namespace std;
using namespace Ice;

class PhoneBookClient : public Application
{
    int run(int argc, char* argv[]);
};

int
PhoneBookClient::run(int argc, char* argv[])
{
    int runParser(int, char*[], const CommunicatorPtr&);
    return runParser(argc, argv, communicator());
}

int
main(int argc, char* argv[])
{
    PhoneBookClient app;
    return app.main(argc, argv, "config");
}
