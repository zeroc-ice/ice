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

#include <Freeze/Freeze.h>
#include <IceUtil/AbstractMutex.h>
#include <TestOld.h>

using namespace std;

class MainObjectI : public MainObject, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
};

class DerivedMainObjectI : public DerivedMainObject, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
};

class FacetObjectI : public FacetObject, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
};

class DerivedFacetObjectI : public DerivedFacetObject, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
};

class Factory : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr
    create(const string& type)
    {
        if(type == "::MainObject")
        {
            return new MainObjectI;
        }
        else if(type == "::DerivedMainObject")
        {
            return new DerivedMainObjectI;
        }
        else if(type == "::FacetObject")
        {
            return new FacetObjectI;
        }
        else if(type == "::DerivedFacetObject")
        {
            return new DerivedFacetObjectI;
        }
        assert(false);
        return 0;
    }

    virtual void
    destroy()
    {
    }
};

int
run(const Ice::CommunicatorPtr& communicator, const string& envName, const string& dbName)
{
    Ice::ObjectFactoryPtr factory = new Factory;
    communicator->addObjectFactory(factory, "");

    Freeze::EvictorPtr evictor = Freeze::createEvictor(communicator, envName, dbName);

    for(int i = 0; i < 10; ++i)
    {
        Ice::FacetPath path;
        path.push_back("theFacet");

        Ice::Identity id;
        ostringstream ostr;
        ostr << "mainObject" << i;
        id.name = ostr.str();

        if((i % 2) == 0)
        {
            MainObjectPtr obj = new MainObjectI;
            obj->boolToString = true;
            obj->byteToShort = 0;
            obj->shortToByte = 255;
            obj->intToShort = 32767;
            obj->longToInt = 2147483647;
            obj->floatToString = 4567.8f;
            obj->doubleToFloat = 8765.4;
            obj->stringToEnum = "E1";
            obj->renamed = E2;
            evictor->createObject(id, obj);
            FacetObjectPtr facet = new FacetObjectI;
            facet->doubleToString = 901234.5;
            evictor->addFacet(id, path, facet);
        }
        else
        {
            DerivedMainObjectPtr obj = new DerivedMainObjectI;
            obj->boolToString = false;
            obj->byteToShort = 0;
            obj->shortToByte = -1;
            obj->intToShort = -32768;
            obj->longToInt = -2147483647 - 1;
            obj->floatToString = -4567.8f;
            obj->doubleToFloat = -8765.4;
            obj->stringToEnum = "E3";
            obj->renamed = E1;
            obj->name = id.name;
            evictor->createObject(id, obj);
            DerivedFacetObjectPtr facet = new DerivedFacetObjectI;
            facet->doubleToString = -901234.5;
            facet->count = i;
            evictor->addFacet(id, path, facet);
        }
    }

    evictor->deactivate("");

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;
 
    string envName = "db";

    try
    {
        communicator = Ice::initialize(argc, argv);
        if(argc != 1)
        {
            envName = argv[1];
            envName += "/";
            envName += "db";
        }
       
        status = run(communicator, envName, "evictor.db");
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
