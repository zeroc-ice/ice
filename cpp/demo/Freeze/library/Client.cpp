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
