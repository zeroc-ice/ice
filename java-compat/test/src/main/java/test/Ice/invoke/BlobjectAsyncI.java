// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.invoke;

import test.Ice.invoke.Test.MyException;

public class BlobjectAsyncI extends Ice.BlobjectAsync
{
    @Override
    public void
    ice_invoke_async(Ice.AMD_Object_ice_invoke cb, byte[] inParams, Ice.Current current)
        throws Ice.UserException
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();
        Ice.InputStream in = new Ice.InputStream(communicator, inParams);
        in.startEncapsulation();
        Ice.OutputStream out = new Ice.OutputStream(communicator);
        out.startEncapsulation();
        if(current.operation.equals("opOneway"))
        {
            cb.ice_response(true, new byte[0]);
        }
        else if(current.operation.equals("opString"))
        {
            String s = in.readString();
            out.writeString(s);
            out.writeString(s);
            out.endEncapsulation();
            cb.ice_response(true, out.finished());
        }
        else if(current.operation.equals("opException"))
        {
            if(current.ctx.containsKey("raise"))
            {
                throw new MyException();
            }
            MyException ex = new MyException();
            out.writeException(ex);
            out.endEncapsulation();
            cb.ice_response(false, out.finished());
        }
        else if(current.operation.equals("shutdown"))
        {
            communicator.shutdown();
            cb.ice_response(true, null);
        }
        else if(current.operation.equals("ice_isA"))
        {
            String s = in.readString();
            if(s.equals("::Test::MyClass"))
            {
                out.writeBool(true);
            }
            else
            {
                out.writeBool(false);
            }
            out.endEncapsulation();
            cb.ice_response(true, out.finished());
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
