// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <FilesystemI.h>

using namespace std;
using namespace Filesystem;

class FilesystemApp : virtual public Ice::Application {
public:
    virtual int run(int, char * []) {
        // Terminate cleanly on receipt of a signal
        //
        shutdownOnInterrupt();

        // Create an object adapter (stored in the NodeI::_adapter
        // static member)
        //
        NodeI::_adapter =
            communicator()->createObjectAdapterWithEndpoints(
                                "SimpleFilesystem", "default -p 10000");

        // Create the root directory (with name "/" and no parent)
        //
        DirectoryIPtr root = new DirectoryI(communicator(), "/", 0);

        // Create a file called "README" in the root directory
        //
        FilePtr file = new FileI(communicator(), "README", root);
        Lines text;
        text.push_back("This file system contains a collection of poetry.");
        file->write(text);

        // Create a directory called "Coleridge" in the root directory
        //
        DirectoryIPtr coleridge = new DirectoryI(communicator(), "Coleridge", root);

        // Create a file called "Kubla_Khan" in the Coleridge directory
        //
        file = new FileI(communicator(), "Kubla_Khan", coleridge);
        text.erase(text.begin(), text.end());
        text.push_back("In Xanadu did Kubla Khan");
        text.push_back("A stately pleasure-dome decree:");
        text.push_back("Where Alph, the sacred river, ran");
        text.push_back("Through caverns measureless to man");
        text.push_back("Down to a sunless sea.");
        file->write(text);

        // All objects are created, allow client requests now
        //
        NodeI::_adapter->activate();

        // Wait until we are done
        //
        communicator()->waitForShutdown();
        if (interrupted()) {
            cerr << appName()
                 << ": received signal, shutting down" << endl;
        }

        NodeI::_adapter = 0;

        return 0;
    };
};

int
main(int argc, char* argv[])
{
    FilesystemApp app;
    return app.main(argc, argv);
}
