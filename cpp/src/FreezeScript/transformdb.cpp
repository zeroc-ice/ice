// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/Transformer.h>
#include <FreezeScript/TransformAnalyzer.h>
#include <FreezeScript/Exception.h>
#include <FreezeScript/Util.h>
#include <Freeze/Initialize.h>
#include <Freeze/Transaction.h>
#include <Freeze/Catalog.h>
#include <db_cxx.h>
#include <sys/stat.h>
#include <fstream>
#include <algorithm>

using namespace std;

#ifdef _WIN32
#   define FREEZE_SCRIPT_DB_MODE 0
#else
#   define FREEZE_SCRIPT_DB_MODE (S_IRUSR | S_IWUSR)
#endif

static void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] [dbenv db newdbenv | dbenv newdbenv]\n";
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
        "-a                    Transform all databases in the dbenv\n"
        "-i                    Ignore incompatible type changes.\n"
        "-p                    Purge objects whose types no longer exist.\n"
        "-c                    Use catastrophic recovery on the old database environment.\n"
        "-w                    Suppress duplicate warnings during transformation.\n"
        "-f FILE               Execute the transformation descriptors in the file FILE.\n"
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

static Slice::TypePtr
findType(const string& prog, const Slice::UnitPtr& u, const string& type)
{
    Slice::TypeList l;

    l = u->lookupType(type, false);
    if(l.empty())
    {
        cerr << prog << ": error: unknown type `" << type << "'" << endl;
        return 0;
    }

    return l.front();
}

static void
transformDb(bool evictor,  const Ice::CommunicatorPtr& communicator, 
	    DbEnv& dbEnv, DbEnv& dbEnvNew, const string& dbName, 
	    const Freeze::ConnectionPtr& connectionNew, vector<Db*>& dbs,
	    const Slice::UnitPtr& oldUnit, const Slice::UnitPtr& newUnit, 
	    DbTxn* txnNew, bool purgeObjects, bool suppress, string descriptors)
{
    if(evictor)
    {
	//
	// The evictor database file contains multiple databases. We must first
	// determine the names of those databases, ignoring any whose names
	// begin with "$index:". Each database represents a separate facet, with
	// the facet name used as the database name. The database named "$default"
	// represents the main object.
	//
	vector<string> dbNames;
	{
	    Db db(&dbEnv, 0);
	    db.open(0, dbName.c_str(), 0, DB_UNKNOWN, DB_RDONLY, 0);
	    Dbt dbKey, dbValue;
	    dbKey.set_flags(DB_DBT_MALLOC);
	    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
	    
	    Dbc* dbc = 0;
	    db.cursor(0, &dbc, 0);
	    
	    while(dbc->get(&dbKey, &dbValue, DB_NEXT) == 0)
	    {
		string s(static_cast<char*>(dbKey.get_data()), dbKey.get_size());
		if(s.find("$index:") != 0)
		{
		    dbNames.push_back(s);
		}
		free(dbKey.get_data());
	    }
	    
	    dbc->close();
	    db.close(0);
	}
	
	//
	// Transform each database. We must delay closing the new databases
	// until after the transaction is committed or aborted.
	//
	for(vector<string>::iterator p = dbNames.begin(); p != dbNames.end(); ++p)
	{
	    string name = p->c_str();
	    
	    Db db(&dbEnv, 0);
	    db.open(0, dbName.c_str(), name.c_str(), DB_BTREE, DB_RDONLY, FREEZE_SCRIPT_DB_MODE);
	    
	    Db* dbNew = new Db(&dbEnvNew, 0);
	    dbs.push_back(dbNew);
	    dbNew->open(txnNew, dbName.c_str(), name.c_str(), DB_BTREE, DB_CREATE | DB_EXCL, FREEZE_SCRIPT_DB_MODE);
	    
	    //
	    // Execute the transformation descriptors.
	    //
	    istringstream istr(descriptors);
	    string facet = (name == "$default" ? "" : name);
	    FreezeScript::transformDatabase(communicator, oldUnit, newUnit, &db, dbNew, txnNew, 0, dbName,
					    facet, purgeObjects, cerr, suppress, istr);
	    
	    db.close(0);
	}
	
	Freeze::Catalog catalogNew(connectionNew, Freeze::catalogName());
	Freeze::CatalogData catalogData;
	catalogData.evictor = true;
	catalogNew.put(Freeze::Catalog::value_type(dbName, catalogData));
    }
    else
    {
	//
	// Transform a map database.
	//
	Db db(&dbEnv, 0);
	db.open(0, dbName.c_str(), 0, DB_BTREE, DB_RDONLY, FREEZE_SCRIPT_DB_MODE);
	
	Db* dbNew = new Db(&dbEnvNew, 0);
	dbs.push_back(dbNew);
	dbNew->open(txnNew, dbName.c_str(), 0, DB_BTREE, DB_CREATE | DB_EXCL, FREEZE_SCRIPT_DB_MODE);
	
	//
	// Execute the transformation descriptors.
	//
	istringstream istr(descriptors);
	FreezeScript::transformDatabase(communicator, oldUnit, newUnit, &db, dbNew, txnNew, connectionNew, 
					dbName, "", purgeObjects, cerr, suppress, istr);
	
	db.close(0);
    }
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
    bool catastrophicRecover = false;
    bool suppress = false;
    bool allDb = false;
    string inputFile;
    vector<string> oldSlice;
    vector<string> newSlice;
    bool evictor = false;
    string keyTypeNames;
    string valueTypeNames;
    string dbEnvName, dbName, dbEnvNameNew;

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
	else if(strcmp(argv[idx], "-a") == 0)
        {
            allDb = true;
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
        else if(strcmp(argv[idx], "-c") == 0)
        {
            catastrophicRecover = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "-w") == 0)
        {
            suppress = true;
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

    if(outputFile.empty() && (allDb && argc != 3 || !allDb && argc != 4))
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
	if(allDb)
	{
	    dbEnvNameNew = argv[2];
	}
	else
	{
	    dbName = argv[2];
	}
    }
    if(argc > 3)
    {
        dbEnvNameNew = argv[3];
    }

    Slice::UnitPtr oldUnit = Slice::Unit::createUnit(true, true, ice, caseSensitive);
    FreezeScript::Destroyer<Slice::UnitPtr> oldD(oldUnit);
    if(!FreezeScript::parseSlice(argv[0], oldUnit, oldSlice, oldCppArgs, debug))
    {
        return EXIT_FAILURE;
    }

    Slice::UnitPtr newUnit = Slice::Unit::createUnit(true, true, ice, caseSensitive);
    FreezeScript::Destroyer<Slice::UnitPtr> newD(newUnit);
    if(!FreezeScript::parseSlice(argv[0], newUnit, newSlice, newCppArgs, debug))
    {
        return EXIT_FAILURE;
    }

    //
    // Install the evictor types in the Slice units.
    //
    FreezeScript::createEvictorSliceTypes(oldUnit);
    FreezeScript::createEvictorSliceTypes(newUnit);

    //
    // If no input file was provided, then we need to analyze the Slice types.
    //
    string descriptors;
    
    if(inputFile.empty())
    {
        ostringstream out;
        vector<string> missingTypes;
        vector<string> analyzeErrors;

        string oldKeyName, newKeyName, oldValueName, newValueName;

	if(evictor)
        {
            oldKeyName = newKeyName = "::Ice::Identity";
            oldValueName = newValueName = "::Freeze::ObjectRecord";
        }
        else
        {
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
        }

        Slice::TypePtr oldKeyType = findType(argv[0], oldUnit, oldKeyName);
        Slice::TypePtr newKeyType = findType(argv[0], newUnit, newKeyName);
        Slice::TypePtr oldValueType = findType(argv[0], oldUnit, oldValueName);
        Slice::TypePtr newValueType = findType(argv[0], newUnit, newValueName);
        if(!oldKeyType || !newKeyType || !oldValueType || !newValueType)
        {   
            return EXIT_FAILURE;
        }

        FreezeScript::TransformAnalyzer analyzer(oldUnit, newUnit, ignoreTypeChanges);
        analyzer.analyze(oldKeyType, newKeyType, oldValueType, newValueType, out, missingTypes, analyzeErrors);

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
        //
        // Read the input file.
        //
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

    if(dbEnvName == dbEnvNameNew)
    {
        cerr << argv[0] << ": database environment names must be different" << endl;
        return EXIT_FAILURE;
    }

    //
    // Transform the database.
    //
    DbEnv dbEnv(0);
    DbEnv dbEnvNew(0);
    Freeze::TransactionPtr txNew = 0;
    vector<Db*> dbs;
    int status = EXIT_SUCCESS;
    try
    {
#ifdef _WIN32
        //
        // Berkeley DB may use a different C++ runtime.
        //
        dbEnv.set_alloc(::malloc, ::realloc, ::free);
        dbEnvNew.set_alloc(::malloc, ::realloc, ::free);
#endif

        //
        // Open the old database environment. Use DB_RECOVER_FATAL if -c is specified.
        // No transaction is created for the old environment.
        //
	// DB_THREAD is for compatibility with Freeze (the catalog)
        {
            u_int32_t flags = DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN | DB_CREATE | DB_THREAD;
            if(catastrophicRecover)
            {
                flags |= DB_RECOVER_FATAL;
            }
            else
            {
                flags |= DB_RECOVER;
            }
            dbEnv.open(dbEnvName.c_str(), flags, FREEZE_SCRIPT_DB_MODE);
        }

        //
        // Open the new database environment and start a transaction.
        //
	//
	// DB_THREAD is for compatibility with Freeze (the catalog)
	//
        {
            u_int32_t flags = DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN | DB_RECOVER | DB_CREATE
		| DB_THREAD;
            dbEnvNew.open(dbEnvNameNew.c_str(), flags, FREEZE_SCRIPT_DB_MODE);
        }

	//
	// TODO: handle properly DbHome config (curremtly it will break if it's set for the new env)
	//

	//
	// Open the catalog of the new environment, and start a transaction
	//
	Freeze::ConnectionPtr connectionNew = Freeze::createConnection(communicator, dbEnvNameNew, dbEnvNew);
	txNew = connectionNew->beginTransaction();
	DbTxn* txnNew = Freeze::getTxn(txNew);


	if(allDb)
	{
	    //
	    // Transform all the databases listed in the old catalog
	    //
	    //
	    // Create each time a new descriptor with the appropriate default database element
	    //

	    Freeze::ConnectionPtr connection = Freeze::createConnection(communicator, dbEnvName, dbEnv);
	    Freeze::Catalog catalog(connection, Freeze::catalogName());
	    for(Freeze::Catalog::const_iterator p = catalog.begin(); p != catalog.end(); ++p)
	    {
		string dbElement;
		if(p->second.evictor)
		{
		    dbElement = "<database key=\"::Ice::Identity\" value=\"::Freeze::ObjectRecord\"/>";
		}
		else
		{
		    //
		    // Temporary: FreezeScript should accept type-ids
		    //
		    string value = p->second.value;
		    if(value == "::Ice::Object")
		    {
			value = "Object";
		    }
		    else if(value == "::Ice::Object*")
		    {
			value = "Object*";
		    }

		    dbElement = "<database key=\"" + p->second.key + "\" value=\"" + value + "\"/>"; 
		}
		
		string localDescriptor = descriptors;
		string transformdbTag = "<transformdb>";
		size_t pos = localDescriptor.find(transformdbTag);
		if(pos == string::npos)
		{
		    cerr << argv[0] << ": Invalid descriptor " << endl;
		    status = EXIT_FAILURE;
		    break;
		}
		localDescriptor.replace(pos, transformdbTag.size(), transformdbTag + dbElement); 
		
		if(debug)
		{
		    cout << "Processing database " << p->first << " with descriptor: " << localDescriptor << endl;
		}

		transformDb(p->second.evictor, communicator, dbEnv, dbEnvNew, p->first, connectionNew, dbs, 
			oldUnit, newUnit, txnNew, purgeObjects, suppress, localDescriptor);
	    }
	}
	else
	{
	    transformDb(evictor, communicator, dbEnv, dbEnvNew, dbName, connectionNew, dbs, 
			oldUnit, newUnit, txnNew, purgeObjects, suppress, descriptors);
	}
    }
    catch(const DbException& ex)
    {
        cerr << argv[0] << ": database error: " << ex.what() << endl;
        status = EXIT_FAILURE;
    }
    catch(...)
    {
        try
        {
            if(txNew != 0)
            {
                txNew->rollback();
            }
            for(vector<Db*>::iterator p = dbs.begin(); p != dbs.end(); ++p)
            {
                Db* db = *p;
                db->close(0);
                delete db;
            }
            dbEnv.close(0);
            dbEnvNew.close(0);
        }
        catch(const DbException& ex)
        {
            cerr << argv[0] << ": database error: " << ex.what() << endl;
        }
        throw;
    }

    if(txNew != 0)
    {
        try
        {
            if(status == EXIT_FAILURE)
            {
                txNew->rollback();
            }
            else
            {
                txNew->commit();

                //
                // Checkpoint to migrate changes from the log to the database(s).
                //
                dbEnvNew.txn_checkpoint(0, 0, DB_FORCE);

                for(vector<Db*>::iterator p = dbs.begin(); p != dbs.end(); ++p)
                {
                    Db* db = *p;
                    db->close(0);
                    delete db;
                }
            }
        }
        catch(const DbException& ex)
        {
            cerr << argv[0] << ": database error: " << ex.what() << endl;
            status = EXIT_FAILURE;
        }
    }
    dbEnv.close(0);
    dbEnvNew.close(0);

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
    catch(const FreezeScript::FailureException& ex)
    {
        string reason = ex.reason();
        cerr << argv[0] << ": " << reason;
        if(reason[reason.size() - 1] != '\n')
        {
            cerr << endl;
        }
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
