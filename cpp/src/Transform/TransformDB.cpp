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

#include <Slice/Preprocessor.h>
#include <Transform/Transformer.h>
#include <Transform/TransformUtil.h>
#include <db_cxx.h>
#include <sys/stat.h>
#include <fstream>
#include <algorithm>

using namespace std;

#ifdef _WIN32
#   define TRANSFORM_DB_MODE 0
#else
#   define TRANSFORM_DB_MODE (S_IRUSR | S_IWUSR)
#endif

static void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] [dbenv db newdb]\n";
    cerr <<
        "Options:\n"
        "-h, --help            Show this message.\n"
        "-v, --version         Display the Ice version.\n"
        "-DNAME                Define NAME as 1.\n"
        "-DNAME=DEF            Define NAME as DEF.\n"
        "-UNAME                Remove any definition for NAME.\n"
        "-d, --debug           Print debug messages.\n"
        "--ice                 Permit `Ice' prefix (for building Ice source code only)\n"
        "-o FILE               Output transformation descriptors into the file FILE.\n"
        "                      Database transformation is not performed.\n"
        "-i                    Ignore incompatible type changes.\n"
        "-p                    Purge objects whose types no longer exist.\n"
        "-f FILE               Execute the transformation descriptors in the file FILE.\n"
        "                      Database transformation is not performed.\n"
        "--include-old DIR     Put DIR in the include file search path for old Slice\n"
        "                      definitions.\n"
        "--include-new DIR     Put DIR in the include file search path for new Slice\n"
        "                      definitions.\n"
        "--old SLICE           Load old Slice definitions from the file SLICE.\n"
        "--new SLICE           Load new Slice definitions from the file SLICE.\n"
        "-e                    Indicates the database is an Evictor database.\n"
        "--key TYPE[,TYPE]     Specifies the Slice types of the database key. If the\n"
        "                      type names have not changed, only one needs to be\n"
        "                      specified. Otherwise, the type names are specified as\n"
        "                      old-type,new-type.\n"
        "--value TYPE[,TYPE]   Specifies the Slice types of the database value. If the\n"
        "                      type names have not changed, only one needs to be\n"
        "                      specified. Otherwise, the type names are specified as\n"
        "                      old-type,new-type.\n"
        ;
    // Note: --case-sensitive is intentionally not shown here!
}

static bool
parseSlice(const string& n, const Slice::UnitPtr& u, const vector<string>& files, const string& cppArgs, bool debug)
{
    //
    // Parse the Slice files.
    //
    for(vector<string>::const_iterator p = files.begin(); p != files.end(); ++p)
    {
        Slice::Preprocessor icecpp(n, *p, cppArgs);

        FILE* cppHandle = icecpp.preprocess(false);

        if(cppHandle == 0)
        {
            return false;
        }

        int status = u->parse(cppHandle, debug);

        if(!icecpp.close())
        {
            return false;            
        }

        if(status != EXIT_SUCCESS)
        {
            return false;
        }
    }

    u->mergeModules();

    return true;
}

static int
run(int argc, char** argv, const Ice::CommunicatorPtr& communicator)
{
    string oldCppArgs;
    string newCppArgs;
    bool debug = false;
    bool ice = true; // Needs to be true in order to create default definitions.
    bool caseSensitive = false;
    string outputFile;
    bool ignoreTypeChanges = false;
    bool purgeObjects = false;
    string inputFile;
    vector<string> oldSlice;
    vector<string> newSlice;
    bool evictor = false;
    string keyTypeNames;
    string valueTypeNames;
    string dbEnvName, dbName, dbNameNew;

    int idx = 1;
    while(idx < argc)
    {
        if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
        {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }
        else if(strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
        {
            cout << ICE_STRING_VERSION << endl;
            return EXIT_SUCCESS;
        }
        else if(strncmp(argv[idx], "-D", 2) == 0 || strncmp(argv[idx], "-U", 2) == 0)
        {
            oldCppArgs += ' ';
            oldCppArgs += argv[idx];
            newCppArgs += ' ';
            newCppArgs += argv[idx];

            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "-d") == 0 || strcmp(argv[idx], "--debug") == 0)
        {
            debug = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "--ice") == 0)
        {
            ice = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "--case-sensitive") == 0)
        {
            caseSensitive = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "-o") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            outputFile = argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "-i") == 0)
        {
            ignoreTypeChanges = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "-p") == 0)
        {
            purgeObjects = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "-f") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            inputFile = argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "--include-old") == 0)
        {
            oldCppArgs += " -I";
            oldCppArgs += argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "--include-new") == 0)
        {
            newCppArgs += " -I";
            newCppArgs += argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "--old") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            oldSlice.push_back(argv[idx + 1]);

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "--new") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            newSlice.push_back(argv[idx + 1]);

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "-e") == 0)
        {
            evictor = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "--key") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            keyTypeNames = argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "--value") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            valueTypeNames = argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(argv[idx][0] == '-')
        {
            cerr << argv[0] << ": unknown option `" << argv[idx] << "'" << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        else
        {
            ++idx;
        }
    }

    if(outputFile.empty() && argc < 4)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(argc > 1)
    {
        dbEnvName = argv[1];
    }
    if(argc > 2)
    {
        dbName = argv[2];
    }
    if(argc > 3)
    {
        dbNameNew = argv[3];
    }

    Slice::UnitPtr oldUnit = Slice::Unit::createUnit(true, true, ice, caseSensitive);
    Transform::Destroyer<Slice::UnitPtr> oldD(oldUnit);
    if(!parseSlice(argv[0], oldUnit, oldSlice, oldCppArgs, debug))
    {
        return EXIT_FAILURE;
    }

    Slice::UnitPtr newUnit = Slice::Unit::createUnit(true, true, ice, caseSensitive);
    Transform::Destroyer<Slice::UnitPtr> newD(newUnit);
    if(!parseSlice(argv[0], newUnit, newSlice, newCppArgs, debug))
    {
        return EXIT_FAILURE;
    }

    //
    // Create the Transformer.
    //
    Transform::Transformer transformer(communicator, oldUnit, newUnit, ignoreTypeChanges, purgeObjects);

    //
    // If no input file was provided, then we need to analyze the Slice types.
    //
    string descriptors;
    if(inputFile.empty())
    {
        ostringstream out;
        vector<string> missingTypes;
        vector<string> analyzeErrors;

        if(evictor)
        {
            transformer.analyze(out, missingTypes, analyzeErrors);
        }
        else
        {
            string oldKeyName, newKeyName, oldValueName, newValueName;
            string::size_type pos;

            if(keyTypeNames.empty() || valueTypeNames.empty())
            {
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            pos = keyTypeNames.find(',');
            if(pos == 0 || pos == keyTypeNames.size())
            {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            if(pos == string::npos)
            {
                oldKeyName = keyTypeNames;
                newKeyName = keyTypeNames;
            }
            else
            {
                oldKeyName = keyTypeNames.substr(0, pos);
                newKeyName = keyTypeNames.substr(pos + 1);
            }

            pos = valueTypeNames.find(',');
            if(pos == 0 || pos == valueTypeNames.size())
            {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            if(pos == string::npos)
            {
                oldValueName = valueTypeNames;
                newValueName = valueTypeNames;
            }
            else
            {
                oldValueName = valueTypeNames.substr(0, pos);
                newValueName = valueTypeNames.substr(pos + 1);
            }

            transformer.analyze(oldKeyName, newKeyName, oldValueName, newValueName, out, missingTypes, analyzeErrors);
        }

        if(!analyzeErrors.empty())
        {
            for(vector<string>::const_iterator p = analyzeErrors.begin(); p != analyzeErrors.end(); ++p)
            {
                cerr << argv[0] << ": " << *p << endl;
            }
        }

        if(!missingTypes.empty())
        {
            sort(missingTypes.begin(), missingTypes.end());
            unique(missingTypes.begin(), missingTypes.end());
            if(!analyzeErrors.empty())
            {
                cerr << endl;
            }
            cerr << "The following types had no matching definitions in the new Slice:" << endl;
            for(vector<string>::const_iterator p = missingTypes.begin(); p != missingTypes.end(); ++p)
            {
                cerr << "  " << *p << endl;
            }
        }

        if(!analyzeErrors.empty())
        {
            return EXIT_FAILURE;
        }

        descriptors = out.str();

        if(!outputFile.empty())
        {
            ofstream of(outputFile.c_str());
            if(!of.good())
            {
                cerr << argv[0] << ": unable to open file `" << outputFile << "'" << endl;
                return EXIT_FAILURE;
            }
            of << descriptors;
            of.close();
            return EXIT_SUCCESS;
        }
    }
    else
    {
        ifstream in(inputFile.c_str());
        char buff[1024];
        while(true)
        {
            in.read(buff, 1024);
            descriptors.append(buff, in.gcount());
            if(in.gcount() < 1024)
            {
                break;
            }
        }
        in.close();
    }

    DbEnv dbEnv(0);
    Db* db = 0;
    Db* dbNew = 0;
    int status = EXIT_SUCCESS;
    try
    {
#ifdef _WIN32
        //
        // Berkeley DB may use a different C++ runtime.
        //
        dbEnv.set_alloc(::malloc, ::realloc, ::free);
#endif

        u_int32_t flags = DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN | DB_RECOVER | DB_CREATE;
        dbEnv.open(dbEnvName.c_str(), flags, TRANSFORM_DB_MODE);
        db = new Db(&dbEnv, 0);
        db->open(0, dbName.c_str(), 0, DB_BTREE, DB_RDONLY, TRANSFORM_DB_MODE);
        dbNew = new Db(&dbEnv, 0);
        dbNew->open(0, dbNameNew.c_str(), 0, DB_BTREE, DB_CREATE | DB_EXCL, TRANSFORM_DB_MODE);

        istringstream istr(descriptors);
        transformer.transform(istr, db, dbNew, cerr);
    }
    catch(const DbException& ex)
    {
        cerr << argv[0] << ": database error: " << ex.what() << endl;
        status = EXIT_FAILURE;
    }
    catch(...)
    {
        if(db)
        {
            db->close(0);
            delete db;
        }
        if(dbNew)
        {
            dbNew->close(0);
            delete dbNew;
        }
        dbEnv.close(0);
        throw;
    }

    if(db)
    {
        db->close(0);
        delete db;
    }
    if(dbNew)
    {
        dbNew->close(0);
        delete dbNew;
    }
    dbEnv.close(0);

    return status;
}

int
main(int argc, char* argv[])
{
    Ice::CommunicatorPtr communicator;
    int status = EXIT_SUCCESS;
    try
    {
        communicator = Ice::initialize(argc, argv);
        status = run(argc, argv, communicator);
    }
    catch(const Transform::TransformException& ex)
    {
        cerr << ex.reason();
        return EXIT_FAILURE;
    }
    catch(const IceUtil::Exception& ex)
    {
        cerr << argv[0] << ": " << ex << endl;
        return EXIT_FAILURE;
    }

    if(communicator)
    {
        communicator->destroy();
    }

    return status;
}
