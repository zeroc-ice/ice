// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <fstream>

using namespace std;

class Client1 : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client1::run(int argc, char** argv)
{
    cout << "testing logger encoding with Ice.LogFile... " << flush;
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->load("config.client");
    properties->setProperty("Ice.LogFile", "log.txt");
    const string programName = properties->getProperty("Ice.ProgramName");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getLogger()->trace("info", "XXX");

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
}

DEFINE_TEST(Client1)
