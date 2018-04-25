// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
invokeInternal(Ice::InputStream& in, vector<Ice::Byte>& outEncaps, const Ice::Current& current)
{
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
    Ice::OutputStream out(communicator);
    out.startEncapsulation();
    if(current.operation == "opOneway")
    {
        return true;
    }
    else if(current.operation == "opString")
    {
        string s;
        in.startEncapsulation();
        in.read(s);
        in.endEncapsulation();
        out.write(s);
        out.write(s);
        out.endEncapsulation();
        out.finished(outEncaps);
        return true;
    }
    else if(current.operation == "opException")
    {
        if(current.ctx.find("raise") != current.ctx.end())
        {
            throw Test::MyException();
        }
        Test::MyException ex;
        out.writeException(ex);
        out.endEncapsulation();
        out.finished(outEncaps);
        return false;
    }
    else if(current.operation == "shutdown")
    {
        out.endEncapsulation();
        out.finished(outEncaps);
        communicator->shutdown();
        return true;
    }
    else if(current.operation == "ice_isA")
    {
        string s;
        in.startEncapsulation();
        in.read(s);
        in.endEncapsulation();
        if(s == "::Test::MyClass")
        {
            out.write(true);
        }
        else
        {
            out.write(false);
        }
        out.endEncapsulation();
        out.finished(outEncaps);
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
BlobjectI::ice_invoke(ICE_IN(vector<Ice::Byte>) inEncaps, vector<Ice::Byte>& outEncaps, const Ice::Current& current)
{
    Ice::InputStream in(current.adapter->getCommunicator(), current.encoding, inEncaps);
    return invokeInternal(in, outEncaps, current);
}

bool
BlobjectArrayI::ice_invoke(ICE_IN(pair<const Ice::Byte*, const Ice::Byte*>) inEncaps, vector<Ice::Byte>& outEncaps,
                           const Ice::Current& current)
{
    Ice::InputStream in(current.adapter->getCommunicator(), current.encoding, inEncaps);
    return invokeInternal(in, outEncaps, current);
}

#ifdef ICE_CPP11_MAPPING
void
BlobjectAsyncI::ice_invokeAsync(vector<Ice::Byte> inEncaps,
                                function<void(bool, const vector<Ice::Byte>&)> response,
                                function<void(exception_ptr)>,
                                const Ice::Current& current)
{
    Ice::InputStream in(current.adapter->getCommunicator(), inEncaps);
    vector<Ice::Byte> outEncaps;
    bool ok = invokeInternal(in, outEncaps, current);
    response(ok, outEncaps);
}

void
BlobjectArrayAsyncI::ice_invokeAsync(pair<const Ice::Byte*, const Ice::Byte*> inEncaps,
                                     function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                                     function<void(exception_ptr)>,
                                     const Ice::Current& current)
{
    Ice::InputStream in(current.adapter->getCommunicator(), inEncaps);
    vector<Ice::Byte> outEncaps;
    bool ok = invokeInternal(in, outEncaps, current);
#if (defined(_MSC_VER) && (_MSC_VER >= 1600))
    pair<const Ice::Byte*, const Ice::Byte*> outPair(static_cast<const Ice::Byte*>(nullptr), static_cast<const Ice::Byte*>(nullptr));
#else
    pair<const Ice::Byte*, const Ice::Byte*> outPair(0, 0);
#endif
    if(outEncaps.size() != 0)
    {
        outPair.first = &outEncaps[0];
        outPair.second = &outEncaps[0] + outEncaps.size();
    }
    response(ok, outPair);
}
#else
void
BlobjectAsyncI::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& cb, const vector<Ice::Byte>& inEncaps,
                                const Ice::Current& current)
{
    Ice::InputStream in(current.adapter->getCommunicator(), current.encoding, inEncaps);
    vector<Ice::Byte> outEncaps;
    bool ok = invokeInternal(in, outEncaps, current);
    cb->ice_response(ok, outEncaps);
}

void
BlobjectArrayAsyncI::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& cb,
                                     const pair<const Ice::Byte*, const Ice::Byte*>& inEncaps,
                                     const Ice::Current& current)
{
    Ice::InputStream in(current.adapter->getCommunicator(), current.encoding, inEncaps);
    vector<Ice::Byte> outEncaps;
    bool ok = invokeInternal(in, outEncaps, current);
#if (defined(_MSC_VER) && (_MSC_VER >= 1600))
    pair<const Ice::Byte*, const Ice::Byte*> outPair(static_cast<const Ice::Byte*>(nullptr), static_cast<const Ice::Byte*>(nullptr));
#else
    pair<const Ice::Byte*, const Ice::Byte*> outPair(0, 0);
#endif
    if(outEncaps.size() != 0)
    {
        outPair.first = &outEncaps[0];
        outPair.second = &outEncaps[0] + outEncaps.size();
    }
    cb->ice_response(ok, outPair);
}
#endif
