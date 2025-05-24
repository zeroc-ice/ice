// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <stdexcept>

using namespace std;

class Client : public Test::TestHelper
{
public:
    Client() = default;

    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
#if TARGET_OS_IPHONE == 0
    if (argc < 2)
    {
        ostringstream os;
        os << "Usage: " << argv[0] << " certsDir";
        throw std::invalid_argument(os.str());
    }
    string certsDir = argv[1];
#else
    string certsDir = "certs/configuration";
#endif

    void allAuthenticationOptionsTests(Test::TestHelper*, const string&);
    allAuthenticationOptionsTests(this, certsDir);

    Test::ServerFactoryPrx allTests(Test::TestHelper*, const string&, bool);

    cerr << "testing with PKCS12 certificates..." << endl;
    Test::ServerFactoryPrx factory = allTests(this, certsDir, true);
#if TARGET_OS_IPHONE == 0
    cerr << "testing with PEM certificates..." << endl;
    factory = allTests(this, certsDir, false);
#endif
    factory->shutdown();
}

DEFINE_TEST(Client)
