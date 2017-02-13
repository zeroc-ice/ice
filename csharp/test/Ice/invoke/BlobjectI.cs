// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class BlobjectI : Ice.Blobject
{
    public override bool
    ice_invoke(byte[] inParams, out byte[] outParams, Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();
        Ice.InputStream inS = Ice.Util.createInputStream(communicator, inParams);
        inS.startEncapsulation();
        Ice.OutputStream outS = Ice.Util.createOutputStream(communicator);
        outS.startEncapsulation();
        if(current.operation.Equals("opOneway"))
        {
            outParams = new byte[0];
            return true;
        }
        else if(current.operation.Equals("opString"))
        {
            string s = inS.readString();
            outS.writeString(s);
            outS.writeString(s);
            outS.endEncapsulation();
            outParams = outS.finished();
            return true;
        }
        else if(current.operation.Equals("opException"))
        {
            Test.MyException ex = new Test.MyException();
            outS.writeException(ex);
            outS.endEncapsulation();
            outParams = outS.finished();
            return false;
        }
        else if(current.operation.Equals("shutdown"))
        {
            communicator.shutdown();
            outParams = null;
            return true;
        }
        else if(current.operation.Equals("ice_isA"))
        {
            string s = inS.readString();
            if(s.Equals("::Test::MyClass"))
            {
                outS.writeBool(true);
            }
            else
            {
                outS.writeBool(false);
            }
            outS.endEncapsulation();
            outParams = outS.finished();
            return true;
        }
        else
        {
            Ice.OperationNotExistException ex = new Ice.OperationNotExistException();
            ex.id = current.id;
            ex.facet = current.facet;
            ex.operation = current.operation;
            throw ex;
        }
    }
}

public class BlobjectAsyncI : Ice.BlobjectAsync
{
    public override void
    ice_invoke_async(Ice.AMD_Object_ice_invoke cb, byte[] inParams, Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();
        Ice.InputStream inS = Ice.Util.createInputStream(communicator, inParams);
        inS.startEncapsulation();
        Ice.OutputStream outS = Ice.Util.createOutputStream(communicator);
        outS.startEncapsulation();
        if(current.operation.Equals("opOneway"))
        {
            cb.ice_response(true, new byte[0]);
        }
        else if(current.operation.Equals("opString"))
        {
            string s = inS.readString();
            outS.writeString(s);
            outS.writeString(s);
            outS.endEncapsulation();
            cb.ice_response(true, outS.finished());
        }
        else if(current.operation.Equals("opException"))
        {
            Test.MyException ex = new Test.MyException();
            outS.writeException(ex);
            outS.endEncapsulation();
            cb.ice_response(false, outS.finished());
        }
        else if(current.operation.Equals("shutdown"))
        {
            communicator.shutdown();
            cb.ice_response(true, null);
        }
        else if(current.operation.Equals("ice_isA"))
        {
            string s = inS.readString();
            if(s.Equals("::Test::MyClass"))
            {
                outS.writeBool(true);
            }
            else
            {
                outS.writeBool(false);
            }
            outS.endEncapsulation();
            cb.ice_response(true, outS.finished());
        }
        else
        {
            Ice.OperationNotExistException ex = new Ice.OperationNotExistException();
            ex.id = current.id;
            ex.facet = current.facet;
            ex.operation = current.operation;
            throw ex;
        }
    }
}
