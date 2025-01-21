// Copyright (c) ZeroC, Inc.

#include "BlobjectI.h"
#include "Ice/Ice.h"
#include "Test.h"

using namespace std;
using namespace Test;
using namespace Ice;

bool
invokeInternal(InputStream& in, vector<byte>& outEncaps, const Current& current)
{
    CommunicatorPtr communicator = current.adapter->getCommunicator();
    OutputStream out(communicator);
    out.startEncapsulation();
    if (current.operation == "opOneway")
    {
        return true;
    }
    else if (current.operation == "opString")
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
    else if (current.operation == "opException")
    {
        if (current.ctx.find("raise") != current.ctx.end())
        {
            throw MyException();
        }
        MyException ex;
        out.writeException(ex);
        out.endEncapsulation();
        out.finished(outEncaps);
        return false;
    }
    else if (current.operation == "shutdown")
    {
        out.endEncapsulation();
        out.finished(outEncaps);
        communicator->shutdown();
        return true;
    }
    else if (current.operation == "ice_isA")
    {
        string s;
        in.startEncapsulation();
        in.read(s);
        in.endEncapsulation();
        if (s == "::Test::MyClass")
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
        throw OperationNotExistException{__FILE__, __LINE__};
    }
}

bool
BlobjectI::ice_invoke(vector<byte> inEncaps, vector<byte>& outEncaps, const Current& current)
{
    InputStream in(current.adapter->getCommunicator(), current.encoding, inEncaps);
    return invokeInternal(in, outEncaps, current);
}

bool
BlobjectArrayI::ice_invoke(pair<const byte*, const byte*> inEncaps, vector<byte>& outEncaps, const Current& current)
{
    InputStream in(current.adapter->getCommunicator(), current.encoding, inEncaps);
    return invokeInternal(in, outEncaps, current);
}

void
BlobjectAsyncI::ice_invokeAsync(
    vector<byte> inEncaps,
    function<void(bool, const vector<byte>&)> response,
    function<void(exception_ptr)>,
    const Current& current)
{
    InputStream in(current.adapter->getCommunicator(), inEncaps);
    vector<byte> outEncaps;
    bool ok = invokeInternal(in, outEncaps, current);
    response(ok, outEncaps);
}

void
BlobjectArrayAsyncI::ice_invokeAsync(
    pair<const byte*, const byte*> inEncaps,
    function<void(bool, pair<const byte*, const byte*>)> response,
    function<void(exception_ptr)>,
    const Current& current)
{
    InputStream in(current.adapter->getCommunicator(), inEncaps);
    vector<byte> outEncaps;
    bool ok = invokeInternal(in, outEncaps, current);
    pair<const byte*, const byte*> outPair(static_cast<const byte*>(nullptr), static_cast<const byte*>(nullptr));
    if (outEncaps.size() != 0)
    {
        outPair.first = &outEncaps[0];
        outPair.second = &outEncaps[0] + outEncaps.size();
    }
    response(ok, outPair);
}
