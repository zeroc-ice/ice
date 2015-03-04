// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Parser.h>

using namespace std;


class PhoneBookClient : public Ice::Application
{
public:

    PhoneBookClient();

    virtual int run(int argc, char* argv[]);
};

int
main(int argc, char* argv[])
{
    PhoneBookClient app;
    return app.main(argc, argv, "config.client");
}

PhoneBookClient::PhoneBookClient() :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling)
{
}

int
PhoneBookClient::run(int argc, char* argv[])
{
    int runParser(int, char*[], const Ice::CommunicatorPtr&);
    return runParser(argc, argv, communicator());
}
