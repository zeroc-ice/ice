//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <stdexcept>

using namespace std;

class Client : public Test::TestHelper
{
public:
    Client() : Test::TestHelper(false) {}

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    //
    // Explicitly register the IceSSL plugin to test registerIceSSL. The tests
    // don't set Ice.Plugin.IceSSL to ensure the plugin is registered without
    // the property setting.
    //
#if !defined(ICE_USE_OPENSSL)
    Ice::registerIceSSL();
#endif

#ifdef ICE_STATIC_LIBS
    Ice::registerIceWS(true);
#endif

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

#if defined(ICE_USE_SECURE_TRANSPORT_MACOS) // TODO add iOS support
    void allSecureTransportTests(Test::TestHelper*, const string&);

    cerr << "testing with SecureTransport native APIs..." << endl;
    allSecureTransportTests(this, testdir);
#endif

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
