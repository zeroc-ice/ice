// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FilesystemI.h>
#include <IdentityFileEntryMap.h>
#include <IdentityDirectoryEntryMap.h>
#include <Ice/Application.h>
#include <Freeze/Freeze.h>

using namespace std;
using namespace Filesystem;
using namespace FilesystemDB;

class FilesystemApp : public virtual Ice::Application
{
public:

    FilesystemApp(const string& envName)
        : _envName(envName)
    {
    }

    virtual int run(int, char*[])
    {
        //
        // Terminate cleanly on receipt of a signal
        //
        shutdownOnInterrupt();

        //
        // Create an object adapter
        //
        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("MapFilesystem");

        //
        // Open a connection to the files and directories database. This should remain open
        // for the duration of the application for performance reasons.
        //
        const Freeze::ConnectionPtr connection(Freeze::createConnection(communicator(), _envName));
        const IdentityFileEntryMap fileDB(connection, FileI::filesDB());
        const IdentityDirectoryEntryMap dirDB(connection, DirectoryI::directoriesDB());

        //
        // Add default servants for the file and directory.
        //
        adapter->addDefaultServant(new FileI(communicator(), _envName), "file");
        adapter->addDefaultServant(new DirectoryI(communicator(), _envName), "");

        //
        // Ready to accept requests now
        //
        adapter->activate();

        //
        // Wait until we are done
        //
        communicator()->waitForShutdown();

        //
        // Clean up
        //
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
