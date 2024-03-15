//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <TestHelper.h>
#include <Test.h>

#include <iterator>

using namespace std;
using namespace Test;
using namespace IceGrid;

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    IceGrid::RegistryPrx registry(
        communicator,
        communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry");

    optional<AdminSessionPrx> session = registry->createAdminSession("foo", "bar");

    session->ice_getConnection()->setACM(registry->getACMTimeout(), nullopt, Ice::ACMHeartbeat::HeartbeatAlways);

    optional<AdminPrx> admin = session->getAdmin();
    test(admin);

    cout << "testing distributions... " << flush;
    {
        admin->startServer("Test.IcePatch2");
        admin->startServer("IcePatch2-Direct");

        TestIntfPrx testPrx(communicator, "server-all");
        test(testPrx->getServerFile("rootfile") == "");

        try
        {
            admin->patchServer("server-all", true);
        }
        catch (const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        test(testPrx->getServerFile("rootfile") == "rootfile");
        test(testPrx->getServerFile("dir1/file1") == "dummy-file1");
        test(testPrx->getServerFile("dir1/file2") == "dummy-file2");
        test(testPrx->getServerFile("dir2/file3") == "dummy-file3");

        test(testPrx->getApplicationFile("rootfile") == "");
        test(testPrx->getApplicationFile("dir1/file1") == "");
        test(testPrx->getApplicationFile("dir1/file2") == "");
        test(testPrx->getApplicationFile("dir2/file3") == "dummy-file3");

        testPrx = TestIntfPrx(communicator, "server-all-direct");

        test(testPrx->getServerFile("rootfile") == "");
        test(testPrx->getServerFile("dir1/file1") == "");
        test(testPrx->getServerFile("dir1/file2") == "");
        test(testPrx->getServerFile("dir2/file3") == "");

        test(testPrx->getApplicationFile("rootfile") == "");
        test(testPrx->getApplicationFile("dir1/file1") == "");
        test(testPrx->getApplicationFile("dir1/file2") == "");
        test(testPrx->getApplicationFile("dir2/file3") == "dummy-file3");

        try
        {
            admin->patchServer("server-all-direct", true);
        }
        catch (const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        test(testPrx->getServerFile("rootfile") == "rootfile");
        test(testPrx->getServerFile("dir1/file1") == "dummy-file1");
        test(testPrx->getServerFile("dir1/file2") == "dummy-file2");
        test(testPrx->getServerFile("dir2/file3") == "dummy-file3");

        test(testPrx->getApplicationFile("rootfile") == "");
        test(testPrx->getApplicationFile("dir1/file1") == "");
        test(testPrx->getApplicationFile("dir1/file2") == "");
        test(testPrx->getApplicationFile("dir2/file3") == "dummy-file3");

        try
        {
            admin->patchApplication("Test", true);
        }
        catch (const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }
        testPrx = TestIntfPrx(communicator, "server-dir1");

        test(testPrx->getServerFile("rootfile") == "");
        test(testPrx->getServerFile("dir1/file1") == "dummy-file1");
        test(testPrx->getServerFile("dir1/file2") == "dummy-file2");
        test(testPrx->getServerFile("dir2/file3") == "");

        test(testPrx->getApplicationFile("rootfile") == "");
        test(testPrx->getApplicationFile("dir1/file1") == "");
        test(testPrx->getApplicationFile("dir1/file2") == "");
        test(testPrx->getApplicationFile("dir2/file3") == "dummy-file3");

        admin->stopServer("Test.IcePatch2");
        admin->stopServer("IcePatch2-Direct");
    }
    cout << "ok" << endl;

    cout << "testing distributions after update... " << flush;
    {
        ApplicationUpdateDescriptor update;
        update.name = "Test";
        update.variables["icepatch.directory"] = "${test.dir}/data/updated";
        admin->updateApplication(update);

        admin->startServer("Test.IcePatch2");
        admin->startServer("IcePatch2-Direct");

        TestIntfPrx testPrx(communicator, "server-all");
        test(testPrx->getServerFile("rootfile") == "rootfile");

        try
        {
            admin->patchServer("server-all", true);
        }
        catch (const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        test(testPrx->getServerFile("rootfile") == "rootfile-updated!");
        test(testPrx->getServerFile("dir1/file1") == "");
        test(testPrx->getServerFile("dir1/file2") == "dummy-file2-updated!");
        test(testPrx->getServerFile("dir2/file3") == "dummy-file3");
        test(testPrx->getServerFile("dir2/file4") == "dummy-file4");

        test(testPrx->getApplicationFile("rootfile") == "");
        test(testPrx->getApplicationFile("dir1/file1") == "");
        test(testPrx->getApplicationFile("dir1/file2") == "");
        test(testPrx->getApplicationFile("dir2/file3") == "dummy-file3");
        test(testPrx->getApplicationFile("dir2/file4") == "dummy-file4");

        try
        {
            admin->patchServer("server-all-direct", true);
        }
        catch (const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }
        testPrx = TestIntfPrx(communicator, "server-all-direct");

        test(testPrx->getServerFile("rootfile") == "rootfile-updated!");
        test(testPrx->getServerFile("dir1/file1") == "");
        test(testPrx->getServerFile("dir1/file2") == "dummy-file2-updated!");
        test(testPrx->getServerFile("dir2/file3") == "dummy-file3");
        test(testPrx->getServerFile("dir2/file4") == "dummy-file4");

        test(testPrx->getApplicationFile("rootfile") == "");
        test(testPrx->getApplicationFile("dir1/file1") == "");
        test(testPrx->getApplicationFile("dir1/file2") == "");
        test(testPrx->getApplicationFile("dir2/file3") == "dummy-file3");
        test(testPrx->getApplicationFile("dir2/file4") == "dummy-file4");

        try
        {
            admin->patchApplication("Test", true);
        }
        catch (const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }
        testPrx = TestIntfPrx(communicator, "server-dir1");

        test(testPrx->getServerFile("rootfile") == "");
        test(testPrx->getServerFile("dir1/file1") == "");
        test(testPrx->getServerFile("dir1/file2") == "dummy-file2-updated!");
        test(testPrx->getServerFile("dir2/file3") == "");
        test(testPrx->getServerFile("dir2/file4") == "");

        test(testPrx->getApplicationFile("rootfile") == "");
        test(testPrx->getApplicationFile("dir1/file1") == "");
        test(testPrx->getApplicationFile("dir1/file2") == "");
        test(testPrx->getApplicationFile("dir2/file3") == "dummy-file3");
        test(testPrx->getApplicationFile("dir2/file4") == "dummy-file4");

        admin->stopServer("Test.IcePatch2");
        admin->stopServer("IcePatch2-Direct");
    }
    cout << "ok" << endl;

    cout << "testing application distrib configuration... " << flush;
    try
    {
        ApplicationDescriptor app = admin->getApplicationInfo("Test").descriptor;
        admin->removeApplication("Test");

        app.variables["icepatch.directory"] = "${test.dir}/data/original";
        test(app.nodes["localnode"].servers[2]->id == "server-dir1");
        app.nodes["localnode"].servers[2]->applicationDistrib = false;

        admin->addApplication(app);
        admin->startServer("Test.IcePatch2");

        try
        {
            admin->patchServer("server-dir1", true);
        }
        catch (const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        TestIntfPrx testPrx(communicator, "server-dir1");

        test(testPrx->getServerFile("rootfile") == "");
        test(testPrx->getServerFile("dir1/file1") == "dummy-file1");
        test(testPrx->getServerFile("dir1/file2") == "dummy-file2");
        test(testPrx->getServerFile("dir2/file3") == "");

        test(testPrx->getApplicationFile("rootfile") == "");
        test(testPrx->getApplicationFile("dir1/file1") == "");
        test(testPrx->getApplicationFile("dir1/file2") == "");
        test(testPrx->getApplicationFile("dir2/file3") == "");

        admin->removeApplication("Test");

        admin->addApplication(app);
        admin->startServer("Test.IcePatch2");
        admin->startServer("IcePatch2-Direct");

        try
        {
            admin->patchApplication("Test", true);
        }
        catch (const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        testPrx = TestIntfPrx(communicator, "server-dir1");

        test(testPrx->getServerFile("rootfile") == "");
        test(testPrx->getServerFile("dir1/file1") == "dummy-file1");
        test(testPrx->getServerFile("dir1/file2") == "dummy-file2");
        test(testPrx->getServerFile("dir2/file3") == "");

        test(testPrx->getApplicationFile("rootfile") == "");
        test(testPrx->getApplicationFile("dir1/file1") == "");
        test(testPrx->getApplicationFile("dir1/file2") == "");
        test(testPrx->getApplicationFile("dir2/file3") == "dummy-file3");

        admin->removeApplication("Test");

        app.distrib.icepatch = "";

        admin->addApplication(app);
        admin->startServer("Test.IcePatch2");
        admin->startServer("IcePatch2-Direct");

        try
        {
            admin->patchServer("server-dir1", true);
        }
        catch (const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        testPrx = TestIntfPrx(communicator, "server-dir1");

        test(testPrx->getServerFile("rootfile") == "");
        test(testPrx->getServerFile("dir1/file1") == "dummy-file1");
        test(testPrx->getServerFile("dir1/file2") == "dummy-file2");
        test(testPrx->getServerFile("dir2/file3") == "");

        test(testPrx->getApplicationFile("rootfile") == "");
        test(testPrx->getApplicationFile("dir1/file1") == "");
        test(testPrx->getApplicationFile("dir1/file2") == "");
        test(testPrx->getApplicationFile("dir2/file3") == "");

        testPrx = TestIntfPrx(communicator, "server-all");

        test(testPrx->getServerFile("rootfile") == "");
        test(testPrx->getServerFile("dir1/file1") == "");
        test(testPrx->getServerFile("dir1/file2") == "");
        test(testPrx->getServerFile("dir2/file3") == "");

        try
        {
            admin->patchApplication("Test", true);
        }
        catch (const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        test(testPrx->getServerFile("rootfile") == "rootfile");
        test(testPrx->getServerFile("dir1/file1") == "dummy-file1");
        test(testPrx->getServerFile("dir1/file2") == "dummy-file2");
        test(testPrx->getServerFile("dir2/file3") == "dummy-file3");
    }
    catch (const DeploymentException& ex)
    {
        cerr << ex << ":\n" << ex.reason << endl;
        test(false);
    }
    cout << "ok" << endl;

    session->destroy();
}
