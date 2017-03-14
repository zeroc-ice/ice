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
#include <fstream>

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
#endif

    cout << "testing logger encoding with Ice.LogFile... " << flush;
    Ice::InitializationData id;
    id.properties = Ice::createProperties();
    id.properties->load("config.client");
    id.properties->setProperty("Ice.LogFile", "log.txt");
    const string programName = id.properties->getProperty("Ice.ProgramName");

    Client c;
    c.main(argc, argv, id);

    ifstream in("log.txt");
    if(!in)
    {
        test(false);
    }
    string s;
    if(!getline(in, s))
    {
        test(false);
    }
    test(s.find(programName) != string::npos);
    in.close();
    remove("log.txt");
    cout << "ok" << endl;
    return EXIT_SUCCESS;
}
