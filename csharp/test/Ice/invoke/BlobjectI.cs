//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice
{
    namespace invoke
    {
        public class BlobjectI : Blobject
        {
            public override bool
            ice_invoke(byte[] inParams, out byte[]? outParams, Current current)
            {
                Communicator communicator = current.Adapter.Communicator;
                InputStream inS = new InputStream(communicator, inParams);
                inS.StartEncapsulation();
                OutputStream outS = new OutputStream(communicator);
                outS.StartEncapsulation();
                if (current.Operation.Equals("opOneway"))
                {
                    outParams = new byte[0];
                    return true;
                }
                else if (current.Operation.Equals("opString"))
                {
                    string s = inS.ReadString();
                    outS.WriteString(s);
                    outS.WriteString(s);
                    outS.EndEncapsulation();
                    outParams = outS.Finished();
                    return true;
                }
                else if (current.Operation.Equals("opException"))
                {
                    if (current.Context.ContainsKey("raise"))
                    {
                        throw new Test.MyException();
                    }
                    var ex = new Test.MyException();
                    outS.WriteException(ex);
                    outS.EndEncapsulation();
                    outParams = outS.Finished();
                    return false;
                }
                else if (current.Operation.Equals("shutdown"))
                {
                    communicator.shutdown();
                    outParams = null;
                    return true;
                }
                else if (current.Operation.Equals("ice_isA"))
                {
                    string s = inS.ReadString();
                    if (s.Equals("::Test::MyClass"))
                    {
                        outS.WriteBool(true);
                    }
                    else
                    {
                        outS.WriteBool(false);
                    }
                    outS.EndEncapsulation();
                    outParams = outS.Finished();
                    return true;
                }
                else
                {
                    OperationNotExistException ex = new OperationNotExistException();
                    ex.id = current.Id;
                    ex.facet = current.Facet;
                    ex.operation = current.Operation;
                    throw ex;
                }
            }
        }

        public class BlobjectAsyncI : BlobjectAsync
        {
            public override Task<Object_Ice_invokeResult>
            ice_invokeAsync(byte[] inParams, Current current)
            {
                Communicator communicator = current.Adapter.Communicator;
                InputStream inS = new InputStream(communicator, inParams);
                inS.StartEncapsulation();
                OutputStream outS = new OutputStream(communicator);
                outS.StartEncapsulation();
                if (current.Operation.Equals("opOneway"))
                {
                    return Task.FromResult(new Object_Ice_invokeResult(true, new byte[0]));
                }
                else if (current.Operation.Equals("opString"))
                {
                    string s = inS.ReadString();
                    outS.WriteString(s);
                    outS.WriteString(s);
                    outS.EndEncapsulation();
                    return Task.FromResult(new Object_Ice_invokeResult(true, outS.Finished()));
                }
                else if (current.Operation.Equals("opException"))
                {
                    Test.MyException ex = new Test.MyException();
                    outS.WriteException(ex);
                    outS.EndEncapsulation();
                    return Task.FromResult(new Object_Ice_invokeResult(false, outS.Finished()));
                }
                else if (current.Operation.Equals("shutdown"))
                {
                    communicator.shutdown();
                    return Task.FromResult(new Object_Ice_invokeResult(true, null));
                }
                else if (current.Operation.Equals("ice_isA"))
                {
                    string s = inS.ReadString();
                    if (s.Equals("::Test::MyClass"))
                    {
                        outS.WriteBool(true);
                    }
                    else
                    {
                        outS.WriteBool(false);
                    }
                    outS.EndEncapsulation();
                    return Task.FromResult(new Object_Ice_invokeResult(true, outS.Finished()));
                }
                else
                {
                    OperationNotExistException ex = new OperationNotExistException();
                    ex.id = current.Id;
                    ex.facet = current.Facet;
                    ex.operation = current.Operation;
                    throw ex;
                }
            }
        }
    }
}
