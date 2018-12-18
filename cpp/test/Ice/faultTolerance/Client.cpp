// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0"); // test aborts
    Ice::CommunicatorHolder ich = initialize(argc, argv, properties);

    vector<int> ports;
    for(int i = 1; i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            ostringstream os;
            os << "unknown option `" << argv[i] << "'";
            throw invalid_argument(os.str());
        }
        ports.push_back(atoi(argv[i]));
    }

    if(ports.empty())
    {
        throw runtime_error("no ports specified");
    }

    void allTests(Test::TestHelper*, const vector<int>&);
    allTests(this, ports);
}

DEFINE_TEST(Client)
