// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <PersistentFilesystemI.h>

using namespace std;
using namespace Filesystem;

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
        // Install object factories.
        //
        Ice::ObjectFactoryPtr factory = new NodeFactory;
        communicator()->addObjectFactory(factory, PersistentFile::ice_staticId());
        communicator()->addObjectFactory(factory, PersistentDirectory::ice_staticId());

        //
        // Create an object adapter (stored in the NodeI::_adapter static member).
        //
        NodeI::_adapter = communicator()->createObjectAdapterWithEndpoints("FreezeFilesystem", "default -p 10000");

        //
        // Create the Freeze evictor (stored in the NodeI::_evictor static member).
        //
        Freeze::ServantInitializerPtr init = new NodeInitializer;
        NodeI::_evictor = Freeze::createBackgroundSaveEvictor(NodeI::_adapter, _envName, "evictorfs", init);

        NodeI::_adapter->addServantLocator(NodeI::_evictor, "");

        //
        // Create the root node if it doesn't exist.
        //
        Ice::Identity rootId = communicator()->stringToIdentity("RootDir");
        if(!NodeI::_evictor->hasObject(rootId))
        {
            PersistentDirectoryPtr root = new DirectoryI(rootId);
            root->nodeName = "/";
            NodeI::_evictor->add(root, rootId);
        }

        //
        // Ready to accept requests now.
        //
        NodeI::_adapter->activate();

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
