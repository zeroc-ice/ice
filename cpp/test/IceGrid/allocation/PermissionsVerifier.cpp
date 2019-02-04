//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/PermissionsVerifier.h>

using namespace std;

class PermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    virtual bool
    checkPermissions(const string&, const string&, string&, const Ice::Current&) const
    {
        return true;
    }
};

class PermissionsVerifierServer : public Ice::Application
{
public:

    virtual int run(int, char*[])
    {
        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("PermissionsVerifier");
        adapter->add(new PermissionsVerifierI, Ice::stringToIdentity("PermissionsVerifier"));
        adapter->activate();
        communicator()->waitForShutdown();
        return EXIT_SUCCESS;
    }
};

int
main(int argc, char* argv[])
{
    PermissionsVerifierServer app;
    return app.main(argc, argv);
}
