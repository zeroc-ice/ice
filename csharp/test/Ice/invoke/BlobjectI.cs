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
}
