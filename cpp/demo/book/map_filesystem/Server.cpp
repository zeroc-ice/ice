// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FilesystemI.h>
#include <Ice/Application.h>
#include <Freeze/Freeze.h>

using namespace std;
using namespace Filesystem;
using namespace FilesystemI;

class FilesystemApp : virtual public Ice::Application
{
public:

    FilesystemApp(const string& envName) :
        _envName(envName)
    {
    }

    virtual int run(int, char*[])
    {
        //
        // Terminate cleanly on receipt of a signal.
        //
        shutdownOnInterrupt();

        //
        // Install object factories.
        //
        communicator()->addObjectFactory(PersistentFile::ice_factory(), PersistentFile::ice_staticId());
        communicator()->addObjectFactory(PersistentDirectory::ice_factory(), PersistentDirectory::ice_staticId());

        //
        // Create an object adapter.
        //
        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapterWithEndpoints(
                                                                "MapFilesystem", "default -p 10000");

        NodeI::_envName = _envName;
        NodeI::_dbName = "mapfs";

        //
        // Find the persistent node for the root directory,
        // or create it if not found.
        //
        Freeze::ConnectionPtr connection =
            Freeze::createConnection(communicator(), _envName);
        IdentityNodeMap persistentMap(connection, NodeI::_dbName);

        Ice::Identity rootId = communicator()->stringToIdentity("RootDir");
        PersistentDirectoryPtr pRoot;
        {
            IdentityNodeMap::iterator p = persistentMap.find(rootId);

            if(p != persistentMap.end())
            {
                pRoot = PersistentDirectoryPtr::dynamicCast(p->second);
                assert(pRoot);
            }
            else
            {
                pRoot = new PersistentDirectory;
                pRoot->name = "/";
                persistentMap.insert(IdentityNodeMap::value_type(rootId, pRoot));
            }
            DirectoryIPtr dir = new DirectoryI(adapter, rootId, pRoot, 0);
            dir->activate(adapter);
        }

        //
        // Ready to accept requests now.
        //
        adapter->activate();

        //
        // Wait until we are done.
        //
        communicator()->waitForShutdown();
        if(interrupted())
        {
            cerr << appName() << ": received signal, shutting down" << endl;
        }

        return 0;
    }

private:

    string _envName;
};

int
main(int argc, char* argv[])
{
    FilesystemApp app("db");
    return app.main(argc, argv, "config.server");
}
