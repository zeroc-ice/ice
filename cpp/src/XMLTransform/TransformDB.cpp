// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <XMLTransform/XMLTransform.h>
#include <Freeze/Freeze.h>

using namespace std;

static void
usage(const string& appName)
{
    cerr << "Usage: " << appName << " [options] db-env db old-schema new-schema\n";
    cerr <<        
        "Options:\n"
        "-IDIR                Add directory DIR to the schema include path.\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
main(int argc, char* argv[])
{
    Ice::CommunicatorPtr communicator;
    Freeze::DBEnvironmentPtr dbEnv;
    Freeze::DBPtr db;
    bool failure = false;

    try
    {
        communicator = Ice::initialize(argc, argv);

        Ice::StringSeq paths;

        int idx = 1;
        while(idx < argc)
        {
            if(strncmp(argv[idx], "-I", 2) == 0)
            {
                if(strlen(argv[idx]) == 2)
                {
                    usage(argv[0]);
                    return EXIT_FAILURE;
                }
                paths.push_back(argv[idx] + 2);
                for(int j = idx; j + 1 < argc; ++j)
                {
                    argv[j] = argv[j + 1];
                }
                --argc;
            }
            else if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
            {
                usage(argv[0]);
                return EXIT_SUCCESS;
            }
            else if(strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
            {
                cout << ICE_STRING_VERSION << endl;
                return EXIT_SUCCESS;
            }
            else if(argv[idx][0] == '-')
            {
                cerr << argv[0] << ": unknown option `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            idx++;
        }

        if(argc < 5)
        {
            usage(argv[0]);
            return EXIT_FAILURE;
        }

        try
        {
            dbEnv = Freeze::initialize(communicator, argv[1]);
            db = dbEnv->openDB(argv[2], false);

            XMLTransform::DBTransformer transformer;
            transformer.transform(dbEnv, db, paths, argv[3], argv[4]);
        }
        catch(const Freeze::DBNotFoundException&)
        {
            cout << argv[0] << ": database `" << argv[2] << "' not found in environment `" << argv[1] << "'" << endl;
            failure = true;
        }
        catch(const Freeze::DBException& ex)
        {
            cout << argv[0] << ": database failure: " << ex << ": " << ex.message << endl;
            failure = true;
        }
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        failure = true;
    }

    if(db)
    {
        try
        {
            db->close();
        }
        catch(const Freeze::DBException& ex)
        {
            cerr << argv[0] << ": " << ex << ": " << ex.message << endl;
            failure = true;
        }
        catch(const Ice::Exception& ex)
        {
            cerr << argv[0] << ": " << ex << endl;
            failure = true;
        }
        catch(...)
        {
            cerr << argv[0] << ": unknown exception" << endl;
            failure = true;
        }
        db = 0;
    }

    if(dbEnv)
    {
        try
        {
            dbEnv->close();
        }
        catch(const Freeze::DBException& ex)
        {
            cerr << argv[0] << ": " << ex << ": " << ex.message << endl;
            failure = true;
        }
        catch(const Ice::Exception& ex)
        {
            cerr << argv[0] << ": " << ex << endl;
            failure = true;
        }
        catch(...)
        {
            cerr << argv[0] << ": unknown exception" << endl;
            failure = true;
        }
        dbEnv = 0;
    }

    try
    {
        communicator->destroy();
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        failure = true;
    }

    if(failure)
    {
        return EXIT_FAILURE;
    }
    else
    {
        return EXIT_SUCCESS;
    }
}
