// Copyright (c) ZeroC, Inc.

package test.Ice.invoke;

import com.zeroc.Ice.BlobjectAsync;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.InputStream;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.OperationNotExistException;
import com.zeroc.Ice.OutputStream;
import com.zeroc.Ice.UserException;

import test.Ice.invoke.Test.MyException;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public class BlobjectAsyncI implements BlobjectAsync {
    @Override
    public CompletionStage<Object.Ice_invokeResult> ice_invokeAsync(
            byte[] inParams, Current current) throws UserException {
        Communicator communicator = current.adapter.getCommunicator();
        InputStream in = new InputStream(communicator, inParams);
        in.startEncapsulation();
        OutputStream out = new OutputStream(communicator);
        out.startEncapsulation();
        Object.Ice_invokeResult r = new Object.Ice_invokeResult();
        if ("opOneway".equals(current.operation)) {
            r.returnValue = true;
            r.outParams = new byte[0];
            return CompletableFuture.completedFuture(r);
        } else if ("opString".equals(current.operation)) {
            String s = in.readString();
            out.writeString(s);
            out.writeString(s);
            out.endEncapsulation();
            r.returnValue = true;
            r.outParams = out.finished();
            return CompletableFuture.completedFuture(r);
        } else if ("opException".equals(current.operation)) {
            if (current.ctx.containsKey("raise")) {
                throw new MyException();
            }
            MyException ex = new MyException();
            out.writeException(ex);
            out.endEncapsulation();
            r.returnValue = false;
            r.outParams = out.finished();
            return CompletableFuture.completedFuture(r);
        } else if ("shutdown".equals(current.operation)) {
            communicator.shutdown();
            r.returnValue = true;
            r.outParams = new byte[0];
            return CompletableFuture.completedFuture(r);
        } else if ("ice_isA".equals(current.operation)) {
            String s = in.readString();
            if ("::Test::MyClass".equals(s)) {
                out.writeBool(true);
            } else {
                out.writeBool(false);
            }
            out.endEncapsulation();
            r.returnValue = true;
            r.outParams = out.finished();
            return CompletableFuture.completedFuture(r);
        } else {
            throw new OperationNotExistException();
        }
    }
}
