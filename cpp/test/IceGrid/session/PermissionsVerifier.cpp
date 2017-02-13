// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Glacier2/PermissionsVerifier.h>
#include <IceSSL/Plugin.h>
#include <Test.h>
#include <TestCommon.h>

using namespace std;

class AdminPermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    virtual bool
    checkPermissions(const string& userId, const string& passwd, string&, const Ice::Current& c) const
    {
        if(c.ctx.find("throw") != c.ctx.end())
        {
            throw Test::ExtendedPermissionDeniedException("reason");
        }
        return (userId == "admin1" && passwd == "test1") || (userId == "admin2" && passwd == "test2") ||
                (userId == "admin3" && passwd == "test3");
    }
};

class PermissionsVerifierServer : public Ice::Application
{
public:

    virtual int run(int, char*[])
    {
        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("PermissionsVerifier");
        adapter->add(new AdminPermissionsVerifierI, Ice::stringToIdentity("AdminPermissionsVerifier"));
        adapter->activate();
        communicator()->waitForShutdown();
        return EXIT_SUCCESS;
    }
};

int
main(int argc, char* argv[])
{
    PermissionsVerifierServer app;
    Ice::InitializationData initData = getTestInitData(argc, argv);
    initData.properties->parseCommandLineOptions("", Ice::argsToStringSeq(argc, argv));
    return app.main(argc, argv, initData);
}
