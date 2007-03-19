// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <DeptFactoryI.h>
#include <OCCIServantLocator.h>
#include <occi.h>
#include <DbTypesMap.h>

using namespace std;
using namespace oracle::occi;

class HRServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    HRServer app;
    return app.main(argc, argv, "config.server");
}

int
HRServer::run(int argc, char* argv[])
{
    const string username = communicator()->getProperties()->getPropertyWithDefault("Oracle.Username", "scott");
    const string password = communicator()->getProperties()->getPropertyWithDefault("Oracle.Password", "password");
    const string connectString = communicator()->getProperties()->getProperty("Oracle.ConnectString");

    //
    // We use a single servant locator and a single category for all Emp and Dept objects
    //
    const string category = "OCCI";
 
    Environment* env = 0;
    StatelessConnectionPool* pool = 0;

    try
    {
        env = Environment::createEnvironment(Environment::Mode(Environment::THREADED_MUTEXED | Environment::OBJECT));
        DbTypesMap(env);
        
        pool = env->createStatelessConnectionPool(username, password, connectString, 5, 2, 1, 
                                                  StatelessConnectionPool::HOMOGENEOUS);

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("HR");
        
        DeptFactoryIPtr factory = new DeptFactoryI(env, pool, category);
        adapter->addServantLocator(new OCCIServantLocator(factory), category);

        adapter->add(factory, communicator()->stringToIdentity("DeptFactory"));
        
        adapter->activate();
        communicator()->waitForShutdown();
    }
    catch(...)
    {
        if(pool != 0)
        {
            env->terminateStatelessConnectionPool(pool);
        }
        if(env != 0)
        {
            Environment::terminateEnvironment(env);
        }
        throw;
    }

    if(pool != 0)
    {
        env->terminateStatelessConnectionPool(pool);
    }
    if(env != 0)
    {
        Environment::terminateEnvironment(env);
    }
    return EXIT_SUCCESS;
}
