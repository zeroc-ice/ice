// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <EmpI.h>
#include <DeptI.h>
#include <DeptFactoryI.h>
#include <Ice/Application.h>
#include <Ice/ServantLocator.h>
#include <occi.h>
#include <DbTypesMap.h>

using namespace std;
using namespace oracle::occi;

class HRServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

class DefaultServantLocator : public Ice::ServantLocator
{
public:
    
    DefaultServantLocator(const Ice::ObjectPtr& servant) :
	_servant(servant)
    {
    }

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&)
    {
	return _servant;
    }

    virtual void finished(const Ice::Current& curr, 
			  const Ice::ObjectPtr& servant, 
			  const Ice::LocalObjectPtr& cookie)
    {
    }

    virtual void deactivate(const std::string&)
    {
    }

private:
    Ice::ObjectPtr _servant;
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

    const string empCategory = "Emp";
    const string deptCategory = "Dept";

    Environment* env = 0;
    StatelessConnectionPool* pool = 0;

    try
    {
	//
	// To Oracle: using an enum parameter for a bitmask is not such a good idea!
	//
	env = Environment::createEnvironment(Environment::Mode(Environment::THREADED_MUTEXED | Environment::OBJECT));
	DbTypesMap(env);
	
	pool = env->createStatelessConnectionPool(username, password, connectString, 5, 2, 1, 
						  StatelessConnectionPool::HOMOGENEOUS);

	Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("HR");
	
	DeptFactoryIPtr factory = new DeptFactoryI(env, pool, empCategory, deptCategory);
	   
	adapter->addServantLocator(new DefaultServantLocator(new EmpI(factory, env, pool, empCategory, deptCategory)),
				   empCategory);
	
	adapter->addServantLocator(new DefaultServantLocator(new DeptI(env, pool, empCategory)), deptCategory);
	
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
