// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Filesystem.h>

using namespace std;
using namespace Filesystem;

class FilesystemClient : public Ice::Application
{
    virtual int run(int argc, char* argv[]);
    virtual void interruptCallback(int);
};

int
main(int argc, char* argv[])
{
    FilesystemClient app;
    return app.main(argc, argv, "config.client");
}

int
FilesystemClient::run(int argc, char* argv[])
{
    //
    // Since this is an interactive demo we want the custom interrupt
    // callback to be called when the process is interrupted.
    //
    callbackOnInterrupt();

    //
    // Create a proxy for the root directory.
    //
    DirectoryPrx rootDir = DirectoryPrx::checkedCast(communicator()->propertyToProxy("RootDir.Proxy"));
    if(!rootDir)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    //
    // Create a file called "README" in the root directory.
    //
    try
    {
        FilePrx readme = rootDir->createFile("README");
        Lines text;
        text.push_back("This file system contains a collection of poetry.");
        readme->write(text);
        cout << "Created README." << endl;
    }
    catch(const NameInUse&)
    {
        //
        // Ignore - file already exists.
        //
    }

    //
    // Create a directory called "Coleridge" in the root directory.
    //
    DirectoryPrx coleridge;
    try
    {
        coleridge = rootDir->createDirectory("Coleridge");
        cout << "Created Coleridge." << endl;
    }
    catch(const NameInUse&)
    {
        NodeDesc desc = rootDir->resolve("Coleridge");
        coleridge = DirectoryPrx::checkedCast(desc.proxy);
        assert(coleridge);
    }

    //
    // Create a file called "Kubla_Khan" in the Coleridge directory.
    //
    try
    {
        FilePrx file = coleridge->createFile("Kubla_Khan");
        Lines text;
        text.push_back("In Xanadu did Kubla Khan");
        text.push_back("A stately pleasure-dome decree:");
        text.push_back("Where Alph, the sacred river, ran");
        text.push_back("Through caverns measureless to man");
        text.push_back("Down to a sunless sea.");
        file->write(text);
        cout << "Created Coleridge/Kubla_Khan." << endl;
    }
    catch(const NameInUse&)
    {
        //
        // Ignore - file already exists.
        //
    }

    cout << "Contents of filesystem:" << endl;
    NodeDict contents = rootDir->list(RecursiveList);
    NodeDict::iterator p;
    for(p = contents.begin(); p != contents.end(); ++p)
    {
        cout << "  " << p->first << endl;
    }

    NodeDesc desc = rootDir->resolve("Coleridge/Kubla_Khan");
    FilePrx file = FilePrx::checkedCast(desc.proxy);
    assert(file);
    Lines text = file->read();
    cout << "Contents of file Coleridge/Kubla_Khan:" << endl;
    for(Lines::iterator i = text.begin(); i != text.end(); ++i)
    {
        cout << "  " << *i << endl;
    }

    //
    // Destroy the filesystem.
    //
    contents = rootDir->list(NormalList);
    for(p = contents.begin(); p != contents.end(); ++p)
    {
        cout << "Destroying " << p->first << "..." << endl;
        p->second.proxy->destroy();
    }

    return EXIT_SUCCESS;
}

void
FilesystemClient::interruptCallback(int)
{
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
