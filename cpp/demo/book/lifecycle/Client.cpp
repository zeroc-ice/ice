// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Filesystem.h>
#include <Parser.h>

using namespace std;
using namespace Filesystem;

class FilesystemClient : virtual public Ice::Application
{
public:
    virtual int run(int, char * []) {
        // Since this is an interactive demo we want the custom
        // interrupt callback to be called when the process is
        // interrupted.
        //
        callbackOnInterrupt();

        // Create a proxy for the root directory
        //
        Ice::ObjectPrx base = communicator()->stringToProxy("RootDir:default -p 10000");
        if(!base)
        {
            throw "Could not create proxy";
        }

        // Down-cast the proxy to a Directory proxy.
        //
        DirectoryPrx rootDir = DirectoryPrx::checkedCast(base);
        if(!rootDir)
        {
            throw "Invalid proxy";
        }

        ParserPtr p = new Parser(rootDir);
        return p->parse();
    }

    virtual void interruptCallback(int) {
        try
        {
            communicator()->destroy();
        }
        catch(const IceUtil::Exception& ex)
        {
            cerr << appName() << ": " << ex << endl;
        }
        catch(...)
        {
            cerr << appName() << ": unknown exception" << endl;
        }
        exit(EXIT_SUCCESS);
    }
};

int
main(int argc, char* argv[])
{
    FilesystemClient client;
    return client.main(argc, argv);
}
