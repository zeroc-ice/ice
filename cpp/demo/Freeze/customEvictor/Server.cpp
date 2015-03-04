// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include <Database.h>
#include <CurrentDatabase.h>
#include <Evictor.h>
#include <SimpleEvictor.h>

using namespace std;
using namespace Warehouse;

const int objectCount = 10000;
const int evictorSize = 8000;

class WarehouseServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    WarehouseServer app;
    return app.main(argc, argv, "config.server");
}

int
WarehouseServer::run(int argc, char* argv[])
{
    bool useSimpleEvictor = argc > 1 && string(argv[1]) == "simple";

    if(useSimpleEvictor)
    {
        cout << "Using SimpleEvictor" << endl;
    }
    else
    {
        cout << "Using Evictor implemented with IceUtil::Cache" << endl; 
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Warehouse");

    const string envName = "db";
    const string dbName = "items";

    {
        //
        // Open our database, a Freeze dictionary.
        //
        Freeze::ConnectionPtr connection = Freeze::createConnection(communicator(), envName);
        Database db(connection, dbName);

        if(db.empty())
        {
            cout << "Creating new database..." << flush;

            //
            // Populate database with objectCount entries.
            //
            ItemInfo info;

            connection->beginTransaction();

            for(int i = 0; i < objectCount; ++i)
            {
                ostringstream os;
                os << "P/N " << i;
                string name = os.str();
                info.description = "The description of " + name;
                info.unitPrice = i + 0.95f;
                info.quantityInStock = i;
                info.filler = string(5000, 'x');

                db.put(Database::value_type(name, info));
            }
            connection->currentTransaction()->commit();
            cout << "done" << endl;
        }
    }

    CurrentDatabase currentDb(communicator(), envName, dbName);

    //
    // This servant locator (evictor) will intercept all categories.
    //
    if(useSimpleEvictor)
    {
        adapter->addServantLocator(new SimpleEvictor(currentDb, evictorSize), "");
    }
    else
    {
        adapter->addServantLocator(new Evictor(currentDb, evictorSize), "");
    }

    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
