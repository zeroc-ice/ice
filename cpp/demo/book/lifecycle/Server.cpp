// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <FilesystemI.h>

using namespace std;
using namespace Filesystem;
using namespace FilesystemI;

class FilesystemApp : virtual public Ice::Application
{
public:

    virtual int run(int, char*[])
    {
        //
        // Terminate cleanly on receipt of a signal
        //
        shutdownOnInterrupt();

        //
        // Create an object adapter.
        //
        Ice::ObjectAdapterPtr adapter =
            communicator()->createObjectAdapterWithEndpoints("SimpleFilesystem", "default -h 127.0.0.1 -p 10000");

        //
        // Create the root directory.
        //
        DirectoryIPtr root = new DirectoryI;
        Ice::Identity id;
        id.name = "RootDir";
        adapter->add(root, id);

        //
        // All objects are created, allow client requests now.
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
    };
};

int
main(int argc, char* argv[])
{
    FilesystemApp app;
    return app.main(argc, argv);
}
