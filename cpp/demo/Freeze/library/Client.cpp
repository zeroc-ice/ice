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

class LibraryClient : public Ice::Application
{
    virtual int run(int argc, char* argv[]);
};

int
main(int argc, char* argv[])
{
    LibraryClient app;
    return app.main(argc, argv, "config");
}

int
LibraryClient::run(int argc, char* argv[])
{
    int runParser(int, char*[], const Ice::CommunicatorPtr&);
    return runParser(argc, argv, communicator());
}
