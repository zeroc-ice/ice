// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/Freeze.h>
#include <ComplexDict.h>
#include <TestCommon.h>
#include <NodeI.h>
#include <Parser.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

//
// Don't use namespace Complex to ensure that the Complex namespace
// exists correctly.
//
// using namespace Complex;
//

static int
validate(const Complex::ComplexDict& m)
{
    cout << "testing database expressions... " << flush;
    Complex::ComplexDict::const_iterator p;
    Parser myParser;
    for(p = m.begin(); p != m.end(); ++p)
    {
        //
        // Verify the stored record is correct.
        //
        // COMPILERFIX: VC.NET reports an unhandled
        // exception if the test is written this way:
        //
        //test(p->first.result == p->second->calc());
        //
        Complex::NodePtr n = p->second;
        test(p->first.result == n->calc());

        //
        // Verify that the expression & result again.
        //
        Complex::NodePtr root = myParser.parse(p->first.expression);
        test(root->calc() == p->first.result);
    }
    cout << "ok" << endl;

    return EXIT_SUCCESS;
}

static const char* expressions[] = 
{
    "2",
    "10",
    "2+(5*3)",
    "5*(2+3)",
    "10+(10+(20+(8*(2*(3*2+4+5+6)))))"
};
static const size_t nexpressions = sizeof(expressions)/sizeof(expressions[0]);

static int
populate(Complex::ComplexDict& m)
{
    cout << "populating the database... " << flush;
    Parser myParser;
    for(size_t i = 0 ; i < nexpressions; ++i)
    {
        Complex::NodePtr root = myParser.parse(expressions[i]);
        assert(root);
        Complex::Key k;
        k.expression = expressions[i];
        k.result = root->calc();
        m.put(pair<const Complex::Key, const Complex::NodePtr>(k, root));
    }
    cout << "ok" << endl;

    return EXIT_SUCCESS;
}

static void
usage(const char* name)
{
    cerr << "Usage: " << name << " [options] validate|populate\n";
    cerr <<     
        "Options:\n"
        "--dbdir           Location of the database directory.\n";
}

static int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator, Complex::ComplexDict& m)
{
    //
    // Register a factory for the node types.
    //
    Ice::ObjectFactoryPtr factory = new Complex::ObjectFactoryI;
    communicator->addObjectFactory(factory, "::Complex::NumberNode");
    communicator->addObjectFactory(factory, "::Complex::AddNode");
    communicator->addObjectFactory(factory, "::Complex::MultiplyNode");

    if(argc > 1 && strcmp(argv[1], "populate") == 0)
    {
        return populate(m);
    }
    if(argc > 1 && strcmp(argv[1], "validate") == 0)
    {
        return validate(m);
    }
    usage(argv[0]);

    return EXIT_FAILURE;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;
   
    string envName = "db";

    try
    {
        //
        // Scan for --dbdir command line argument.
        //
        int i = 1;
        while(i < argc)
        {
            if(strcmp(argv[i], "--dbdir") == 0)
            {
                if(i +1 >= argc)
                {
                    usage(argv[0]);
                    return EXIT_FAILURE;
                }

                envName = argv[i+1];
                envName += "/";
                envName += "db";

                //
                // Consume arguments
                //
                while(i < argc - 2)
                {
                    argv[i] = argv[i+2];
                    ++i;
                }
                argc -= 2;
            }
            else
            {
                ++i;
            }
        }

        communicator = Ice::initialize(argc, argv);
        Freeze::ConnectionPtr connection = createConnection(communicator, envName);
        Complex::ComplexDict m(connection, "test");
        status = run(argc, argv, communicator, m);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    try
    {
        communicator->destroy();
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    return status;
}
