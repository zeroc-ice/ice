// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.invoke;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import test.Ice.invoke.Test.MyException;

public class BlobjectAsyncI implements com.zeroc.Ice.BlobjectAsync
{
    @Override
    public CompletionStage<com.zeroc.Ice.Object.Ice_invokeResult> ice_invokeAsync(byte[] inParams,
                                                                                  com.zeroc.Ice.Current current)
        throws com.zeroc.Ice.UserException
    {
        com.zeroc.Ice.Communicator communicator = current.adapter.getCommunicator();
        com.zeroc.Ice.InputStream in = new com.zeroc.Ice.InputStream(communicator, inParams);
        in.startEncapsulation();
        com.zeroc.Ice.OutputStream out = new com.zeroc.Ice.OutputStream(communicator);
        out.startEncapsulation();
        com.zeroc.Ice.Object.Ice_invokeResult r = new com.zeroc.Ice.Object.Ice_invokeResult();
        if(current.operation.equals("opOneway"))
        {
            r.returnValue = true;
            r.outParams = new byte[0];
            return CompletableFuture.completedFuture(r);
        }
        else if(current.operation.equals("opString"))
        {
            String s = in.readString();
            out.writeString(s);
            out.writeString(s);
            out.endEncapsulation();
            r.returnValue = true;
            r.outParams = out.finished();
            return CompletableFuture.completedFuture(r);
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
            r.returnValue = false;
            r.outParams = out.finished();
            return CompletableFuture.completedFuture(r);
        }
        else if(current.operation.equals("shutdown"))
        {
            communicator.shutdown();
            r.returnValue = true;
            r.outParams = new byte[0];
            return CompletableFuture.completedFuture(r);
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
            r.returnValue = true;
            r.outParams = out.finished();
            return CompletableFuture.completedFuture(r);
        }
        else
        {
            com.zeroc.Ice.OperationNotExistException ex = new com.zeroc.Ice.OperationNotExistException();
            ex.id = current.id;
            ex.facet = current.facet;
            ex.operation = current.operation;
            throw ex;
        }
    }
}
