// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
    cerr << "       " << appName << " [options] --evictor db-env db\n";
    cerr << "\n";
    cerr <<        
        "Options:\n"
        "-e,--evictor         Use the Freeze Evictor schema.\n"
        "--load-old DIR       Load all old schema files in directory DIR.\n"
        "--load-new DIR       Load all new schema files in directory DIR.\n"
        "--path-old DIR       Add directory DIR to the old schema import search path.\n"
        "--path-new DIR       Add directory DIR to the new schema import search path.\n"
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

        bool evictor = false;
        Ice::StringSeq loadOld, loadNew;
        Ice::StringSeq pathOld, pathNew;

        int idx = 1;
        while(idx < argc)
        {
            if(strcmp(argv[idx], "--load-old") == 0)
            {
                if(idx + 1 >= argc)
                {
                    cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                    usage(argv[0]);
                    return EXIT_FAILURE;
                }

                loadOld.push_back(argv[idx + 1]);
                for(int i = idx ; i + 2 < argc ; ++i)
                {
                    argv[i] = argv[i + 2];
                }
                argc -= 2;
            }
            else if(strcmp(argv[idx], "--load-new") == 0)
            {
                if(idx + 1 >= argc)
                {
                    cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                    usage(argv[0]);
                    return EXIT_FAILURE;
                }

                loadNew.push_back(argv[idx + 1]);
                for(int i = idx ; i + 2 < argc ; ++i)
                {
                    argv[i] = argv[i + 2];
                }
                argc -= 2;
            }
            else if(strcmp(argv[idx], "--path-old") == 0)
            {
                if(idx + 1 >= argc)
                {
                    cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                    usage(argv[0]);
                    return EXIT_FAILURE;
                }

                pathOld.push_back(argv[idx + 1]);
                for(int i = idx ; i + 2 < argc ; ++i)
                {
                    argv[i] = argv[i + 2];
                }
                argc -= 2;
            }
            else if(strcmp(argv[idx], "--path-new") == 0)
            {
                if(idx + 1 >= argc)
                {
                    cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                    usage(argv[0]);
                    return EXIT_FAILURE;
                }

                pathNew.push_back(argv[idx + 1]);
                for(int i = idx ; i + 2 < argc ; ++i)
                {
                    argv[i] = argv[i + 2];
                }
                argc -= 2;
            }
            else if(strcmp(argv[idx], "-e") == 0 || strcmp(argv[idx], "--evictor") == 0)
            {
                evictor = true;
                for(int i = idx ; i + 1 < argc ; ++i)
                {
                    argv[i] = argv[i + 1];
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
	    else
	    {
		idx++;
	    }
        }

        if((evictor && argc != 3) || (!evictor && argc != 5))
        {
            usage(argv[0]);
            return EXIT_FAILURE;
        }

        try
        {
            dbEnv = Freeze::initialize(communicator, argv[1]);
            db = dbEnv->openDB(argv[2], false);

            XMLTransform::DBTransformer transformer;

            if(evictor)
            {
                //
                // This is the schema definition for the database used by the
                // Freeze Evictor (a map of Ice::Identity to Ice::Object).
                // Note that this requires that a schema file be created for
                // Identity.ice, and that it be loaded using --load-old and
                // --load-new.
                //
                static string schema =
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    "<xs:schema xmlns:xs=\"http://www.w3.org/2002/XMLSchema\""
                    "           elementFormDefault=\"qualified\""
                    "           xmlns:tns=\"http://www.noorg.org/schemas\""
                    "           xmlns:ice=\"http://www.zeroc.com/schemas\""
                    "           targetNamespace=\"http://www.noorg.org/schemas\">"
                    "    <xs:import namespace=\"http://www.zeroc.com/schemas\" schemaLocation=\"ice.xsd\"/>"
                    "    <xs:element name=\"Key\" type=\"_internal.Ice.IdentityType\"/>"
                    "    <xs:element name=\"Value\" type=\"ice:_internal.objectType\"/></xs:schema>";

                transformer.transform(dbEnv, db, loadOld, loadNew, pathOld, pathNew, schema);
            }
            else
            {
                transformer.transform(dbEnv, db, loadOld, loadNew, pathOld, pathNew, argv[3], argv[4]);
            }
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
