// **********************************************************************
//
// Copyright (c) 2003
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
#include <db_cxx.h>
#include <sys/stat.h>

#ifdef _WIN32
#   define FREEZE_DB_MODE 0
#else
#   define FREEZE_DB_MODE (S_IRUSR | S_IWUSR)
#endif

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
        "--load-old PATH      Load old schema from the given file or directory.\n"
        "--load-new PATH      Load new schema from the given file or directory.\n"
        "--path-old DIR       Add directory DIR to the old schema import search path.\n"
        "--path-new DIR       Add directory DIR to the new schema import search path.\n"
        "--force              Force removal of objects whose types cannot be found.\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
main(int argc, char* argv[])
{
    Ice::CommunicatorPtr communicator;

    bool failure = false;

    try
    {
        communicator = Ice::initialize(argc, argv);

        bool evictor = false;
        Ice::StringSeq loadOld, loadNew;
        Ice::StringSeq pathOld, pathNew;
        bool force = false;

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
            else if(strcmp(argv[idx], "--force") == 0)
            {
                force = true;
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
	    DbEnv dbEnv(0);
	    dbEnv.set_flags(DB_TXN_NOSYNC, true);
	    
	    u_int32_t flags = DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN |
		DB_PRIVATE | DB_THREAD;
	    
	    //
	    // TODO: DB_RECOVER_FATAL option
	    //
	    flags |= DB_RECOVER | DB_CREATE;

	    dbEnv.open(argv[1], flags, FREEZE_DB_MODE);
	    
	    Db db(&dbEnv, 0);
	    flags = DB_AUTO_COMMIT | DB_THREAD;
	    db.open(0, argv[2], 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 

            XMLTransform::DBTransformer transformer(dbEnv, db, loadOld, loadNew, pathOld, pathNew, force);

            if(evictor)
            {
                //
                // This is the schema definition for the database used by the
                // Freeze Evictor (a map of Ice::Identity to Freeze::ObjectRecord).
                // Note that this requires that schema files be created for
                // Identity.ice and ObjectRecord.ice, and that they be loaded using
                // --load-old and --load-new.
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
                    "    <xs:element name=\"Value\" type=\"_internal.Freeze.ObjectRecordType\"/></xs:schema>";

                transformer.transform(schema);
            }
            else
            {
                transformer.transform(argv[3], argv[4]);
            }
	    
	    db.close(0);
	    dbEnv.close(0);
	    
        }
        catch(const DbException& ex)
        {
	    //
	    // Note: from Berkeley DB 4.1 on, ~Db and ~DbEnv close their respective
	    // handles if not done before.
	    //

            cout << argv[0] << ": database failure: " << ex.what() << endl;
            failure = true;
        }
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        failure = true;
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
