// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/ExceptionFactory.h>
#include <IcePack/Admin.h>

using namespace std;

IcePack::ExceptionFactory::ExceptionFactory(const Ice::CommunicatorPtr& communicator)
{
    communicator->addUserExceptionFactory(this, "::IcePack::DeploymentException");
    communicator->addUserExceptionFactory(this, "::IcePack::ParserDeploymentException");
    communicator->addUserExceptionFactory(this, "::IcePack::AdapterDeploymentException");
    communicator->addUserExceptionFactory(this, "::IcePack::ServerDeploymentException");
    communicator->addUserExceptionFactory(this, "::IcePack::OfferDeploymentException");
}

void
IcePack::ExceptionFactory::createAndThrow(const string& type)
{
    if(type == "::IcePack::DeploymentException")
    {
	throw DeploymentException();
    }
    else if(type == "::IcePack::ParserDeploymentException")
    {
	throw ParserDeploymentException();
    }
    else if(type == "::IcePack::AdapterDeploymentException")
    {
	throw AdapterDeploymentException();
    }
    else if(type == "::IcePack::OfferDeploymentException")
    {
	throw OfferDeploymentException();
    }
    else if(type == "::IcePack::ServerDeploymentException")
    {
	throw ServerDeploymentException();
    }
}

void
IcePack::ExceptionFactory::destroy()
{
}
