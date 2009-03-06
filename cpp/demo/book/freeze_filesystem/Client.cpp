// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Filesystem.h>

using namespace std;
using namespace Filesystem;

// Recursively print the contents of directory "dir" in tree fashion.
// For files, show the contents of each file. The "depth"
// parameter is the current nesting level (for indentation).

static void
listRecursive(const DirectoryPrx & dir, int depth = 0)
{
    string indent(++depth, '\t');

    NodeDescSeq contents = dir->list();

    for (NodeDescSeq::const_iterator i = contents.begin(); i != contents.end(); ++i) {
        DirectoryPrx dir = DirectoryPrx::checkedCast(i->proxy);
        FilePrx file = FilePrx::uncheckedCast(i->proxy);
        cout << indent << i->name << (dir ? " (directory):" : " (file):") << endl;
        if (dir) {
            listRecursive(dir, depth);
        } else {
            Lines text = file->read();
            for (Lines::const_iterator j = text.begin(); j != text.end(); ++j)
                cout << indent << "\t" << *j << endl;
        }
    }
}

class FilesystemClient : public Ice::Application
{
public:

    FilesystemClient();
    virtual int run(int argc, char* argv[]);
};

int
main(int argc, char* argv[])
{
    FilesystemClient app;
    return app.main(argc, argv, "config.client");
}

FilesystemClient::FilesystemClient() :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling)
{
}

int
FilesystemClient::run(int argc, char* argv[])
{
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
    FilePrx readme;
    try
    {
        readme = rootDir->createFile("README");
        Lines text;
        text.push_back("This file system contains a collection of poetry.");
        readme->write(text);
        cout << "Created README." << endl;
    }
    catch(const NameInUse&)
    {
        NodeDesc desc = rootDir->find("README");
        readme = FilePrx::checkedCast(desc.proxy);
        assert(readme);
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
        NodeDesc desc = rootDir->find("Coleridge");
        coleridge = DirectoryPrx::checkedCast(desc.proxy);
        assert(coleridge);
    }

    //
    // Create a file called "Kubla_Khan" in the Coleridge directory.
    //
    FilePrx file;
    try
    {
        file = coleridge->createFile("Kubla_Khan");
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
        NodeDesc desc = coleridge->find("Kubla_Khan");
        file = FilePrx::checkedCast(desc.proxy);
        assert(file);
    }

    cout << "Contents of filesystem:" << endl;
    listRecursive(rootDir);

    //
    // Destroy the filesystem.
    //
    cout << "Destroying " << file->name() << endl;
    file->destroy();
    cout << "Destroying " << readme->name() << endl;
    readme->destroy();
    cout << "Destroying " << coleridge->name() << endl;
    coleridge->destroy();

    return EXIT_SUCCESS;
}
