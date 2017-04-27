// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>

using namespace std;

namespace
{

class Client : public Ice::Application
{
public:
    virtual int
    run(int, char*[])
    {
        communicator()->getLogger()->trace("info", "XXX");
        return EXIT_SUCCESS;
    };
};

}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL(false);
    Ice::registerIceWS(true);
#endif

#ifdef _WIN32
    //
    // 28605 == ISO 8859-15 codepage
    //
    int cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
    setProcessStringConverter(Ice::createWindowsStringConverter(28605));
#else
    setProcessStringConverter(Ice::createIconvStringConverter<char>("ISO8859-15"));
#endif
    Ice::InitializationData id;
    id.properties = Ice::createProperties();
    id.properties->load("config.client");
    Client c;
    int status = c.main(argc, argv, id);
#ifdef _WIN32
    SetConsoleOutputCP(cp);
#endif
    return status;
}
