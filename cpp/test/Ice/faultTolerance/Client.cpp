// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"

#include <stdexcept>

using namespace std;

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0"); // test aborts
    Ice::CommunicatorHolder ich = initialize(argc, argv, properties);

    vector<int> ports;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            ostringstream os;
            os << "unknown option `" << argv[i] << "'";
            throw invalid_argument(os.str());
        }
        ports.push_back(stoi(argv[i]));
    }

    if (ports.empty())
    {
        throw runtime_error("no ports specified");
    }

    void allTests(Test::TestHelper*, const vector<int>&);
    allTests(this, ports);
}

DEFINE_TEST(Client)
