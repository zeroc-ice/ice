// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <stdexcept>

using namespace std;

class Client : public Test::TestHelper
{
public:
    Client() : Test::TestHelper(false) {}

    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::registerIceWS(); // for static builds

    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    string testdir;
#if TARGET_OS_IPHONE == 0
    if (argc < 2)
    {
        ostringstream os;
        os << "Usage: " << argv[0] << " testdir";
        throw std::invalid_argument(os.str());
    }
    testdir = argv[1];
#endif

    void allAuthenticationOptionsTests(Test::TestHelper*, const string&);
    allAuthenticationOptionsTests(this, testdir);

    Test::ServerFactoryPrx allTests(Test::TestHelper*, const string&, bool);

    cerr << "testing with PKCS12 certificates..." << endl;
    Test::ServerFactoryPrx factory = allTests(this, testdir, true);
#if TARGET_OS_IPHONE == 0
    cerr << "testing with PEM certificates..." << endl;
    factory = allTests(this, testdir, false);
#endif
    factory->shutdown();
}

DEFINE_TEST(Client)
