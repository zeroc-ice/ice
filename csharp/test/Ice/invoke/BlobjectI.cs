//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Threading.Tasks;

namespace Ice.invoke
{
    public class BlobjectI : IObject
    {
        public async ValueTask<OutputStream> DispatchAsync(Ice.InputStream istr, Current current)
        {
            if (current.Operation.Equals("opOneway"))
            {
                Debug.Assert(current.IsOneway);
                // TODO: replace by OutputStream.Empty
                return IceInternal.Protocol.CreateEmptyResponseFrame(current);
            }
            else if (current.Operation.Equals("opString"))
            {
                string s = istr.ReadString();
                var ostr = IceInternal.Protocol.StartResponseFrame(current);
                ostr.WriteString(s);
                ostr.WriteString(s);
                ostr.EndEncapsulation();
                return ostr;
            }
            else if (current.Operation.Equals("opException"))
            {
                if (current.Context.ContainsKey("raise"))
                {
                    throw new Test.MyException();
                }
                var ex = new Test.MyException();
                var ostr = IceInternal.Protocol.StartFailureResponseFrame(current);
                ostr.WriteException(ex);
                ostr.EndEncapsulation();
                return ostr;
            }
            else if (current.Operation.Equals("shutdown"))
            {
                current.Adapter.Communicator.Shutdown();
                return IceInternal.Protocol.CreateEmptyResponseFrame(current);
            }
            else if (current.Operation.Equals("ice_isA"))
            {
                string s = istr.ReadString();
                var ostr = IceInternal.Protocol.StartResponseFrame(current);
                if (s.Equals("::Test::MyClass"))
                {
                    ostr.WriteBool(true);
                }
                else
                {
                    ostr.WriteBool(false);
                }
                ostr.EndEncapsulation();
                return ostr;
            }
            else
            {
                throw new OperationNotExistException(current.Id, current.Facet, current.Operation);
            }
        }
    }

    public class BlobjectAsyncI : BlobjectAsync
    {
        public override Task<Object_Ice_invokeResult>
        IceInvokeAsync(byte[] inParams, Current current)
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
                communicator.Shutdown();
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
                ex.Id = current.Id;
                ex.Facet = current.Facet;
                ex.Operation = current.Operation;
                throw ex;
            }
        }
    }
}
