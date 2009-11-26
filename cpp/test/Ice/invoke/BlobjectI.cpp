// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <BlobjectI.h>
#include <Test.h>
#include <TestCommon.h>

using namespace std;

bool
invokeInternal(const Ice::InputStreamPtr& in, vector<Ice::Byte>& outParams, const Ice::Current& current)
{
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
    Ice::OutputStreamPtr out = Ice::createOutputStream(communicator);
    if(current.operation == "opString")
    {
        string s;
        in->read(s);
        out->write(s);
        out->write(s);
        out->finished(outParams);
        return true;
    }
    else if(current.operation == "opException")
    {
        Test::MyException ex;
        out->writeException(ex);
        out->finished(outParams);
        return false;
    }
    else if(current.operation == "shutdown")
    {
        communicator->shutdown();
        return true;
    }
    else if(current.operation == "ice_isA")
    {
        string s;
        in->read(s);
        if(s == "::Test::MyClass")
        {
            out->write(true);
        }
        else
        {
            out->write(false);
        }
        out->finished(outParams);
        return true;
    }
    else
    {
        Ice::OperationNotExistException ex(__FILE__, __LINE__);
        ex.id = current.id;
        ex.facet = current.facet;
        ex.operation = current.operation;
        throw ex;
    }
}

bool
BlobjectI::ice_invoke(const vector<Ice::Byte>& inParams, vector<Ice::Byte>& outParams, const Ice::Current& current)
{
    Ice::InputStreamPtr in = Ice::createInputStream(current.adapter->getCommunicator(), inParams);
    return invokeInternal(in, outParams, current);
}

bool
BlobjectArrayI::ice_invoke(const pair<const Ice::Byte*, const Ice::Byte*>& inParams, vector<Ice::Byte>& outParams,
                          const Ice::Current& current)
{
    Ice::InputStreamPtr in = Ice::createInputStream(current.adapter->getCommunicator(), inParams);
    return invokeInternal(in, outParams, current);
}


void
BlobjectAsyncI::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& cb, const vector<Ice::Byte>& inParams, 
                                const Ice::Current& current)
{
    Ice::InputStreamPtr in = Ice::createInputStream(current.adapter->getCommunicator(), inParams);
    vector<Ice::Byte> outParams;
    bool ok = invokeInternal(in, outParams, current);
    cb->ice_response(ok, outParams);
}

void
BlobjectArrayAsyncI::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& cb,
                                     const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
                                     const Ice::Current& current)
{
    Ice::InputStreamPtr in = Ice::createInputStream(current.adapter->getCommunicator(), inParams);
    vector<Ice::Byte> outParams;
    bool ok = invokeInternal(in, outParams, current);
    pair<const Ice::Byte*, const Ice::Byte*> outPair(&outParams[0], &outParams[0] + outParams.size());
    cb->ice_response(ok, outParams);
}
