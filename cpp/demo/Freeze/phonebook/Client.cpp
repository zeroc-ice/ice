// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
