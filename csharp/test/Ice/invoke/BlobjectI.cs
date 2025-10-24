// Copyright (c) ZeroC, Inc.

namespace Ice.invoke;

public class BlobjectI : Ice.Blobject
{
    public override bool
    ice_invoke(byte[] inParams, out byte[] outParams, Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();
        var inS = new Ice.InputStream(communicator, inParams);
        inS.startEncapsulation();
        var outS = new Ice.OutputStream(communicator);
        outS.startEncapsulation();
        if (current.operation == "opOneway")
        {
            outParams = [];
            return true;
        }
        else if (current.operation == "opString")
        {
            string s = inS.readString();
            outS.writeString(s);
            outS.writeString(s);
            outS.endEncapsulation();
            outParams = outS.finished();
            return true;
        }
        else if (current.operation == "opException")
        {
            if (current.ctx.ContainsKey("raise"))
            {
                throw new Test.MyException();
            }
            var ex = new Test.MyException();
            outS.writeException(ex);
            outS.endEncapsulation();
            outParams = outS.finished();
            return false;
        }
        else if (current.operation == "shutdown")
        {
            communicator.shutdown();
            outParams = null;
            return true;
        }
        else if (current.operation == "ice_isA")
        {
            string s = inS.readString();
            if (s == "::Test::MyClass")
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
            throw new Ice.OperationNotExistException();
        }
    }
}

public class BlobjectAsyncI : Ice.BlobjectAsync
{
    public override Task<Ice.Object_Ice_invokeResult>
    ice_invokeAsync(byte[] inEncaps, Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();
        var inS = new Ice.InputStream(communicator, inEncaps);
        inS.startEncapsulation();
        var outS = new Ice.OutputStream(communicator);
        outS.startEncapsulation();
        if (current.operation == "opOneway")
        {
            return Task.FromResult(new Ice.Object_Ice_invokeResult(true, []));
        }
        else if (current.operation == "opString")
        {
            string s = inS.readString();
            outS.writeString(s);
            outS.writeString(s);
            outS.endEncapsulation();
            return Task.FromResult(new Ice.Object_Ice_invokeResult(true, outS.finished()));
        }
        else if (current.operation == "opException")
        {
            var ex = new Test.MyException();
            outS.writeException(ex);
            outS.endEncapsulation();
            return Task.FromResult(new Ice.Object_Ice_invokeResult(false, outS.finished()));
        }
        else if (current.operation == "shutdown")
        {
            communicator.shutdown();
            return Task.FromResult(new Ice.Object_Ice_invokeResult(true, null));
        }
        else if (current.operation == "ice_isA")
        {
            string s = inS.readString();
            if (s == "::Test::MyClass")
            {
                outS.writeBool(true);
            }
            else
            {
                outS.writeBool(false);
            }
            outS.endEncapsulation();
            return Task.FromResult(new Ice.Object_Ice_invokeResult(true, outS.finished()));
        }
        else
        {
            throw new Ice.OperationNotExistException();
        }
    }
}
