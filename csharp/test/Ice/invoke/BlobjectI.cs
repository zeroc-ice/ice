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
                if (!current.IsOneway)
                {
                    // If called two-way, return exception to caller.
                    throw new Test.MyException();
                }
                return OutgoingResponseFrame.Empty(current);
            }
            else if (current.Operation.Equals("opString"))
            {
                string s = istr.ReadString();
                var responseFrame = new OutgoingResponseFrame(current);
                responseFrame.StartReturnValue();
                responseFrame.WriteString(s);
                responseFrame.WriteString(s);
                responseFrame.EndReturnValue();
                return responseFrame;
            }
            else if (current.Operation.Equals("opException"))
            {
                if (current.Context.ContainsKey("raise"))
                {
                    throw new Test.MyException();
                }
                var ex = new Test.MyException();
                return new OutgoingResponseFrame(current, ex);
            }
            else if (current.Operation.Equals("shutdown"))
            {
                current.Adapter.Communicator.Shutdown();
                return OutgoingResponseFrame.Empty(current);
            }
            else if (current.Operation.Equals("ice_isA"))
            {
                string s = istr.ReadString();
                var responseFrame = new OutgoingResponseFrame(current);
                responseFrame.StartReturnValue();
                if (s.Equals("::Test::MyClass"))
                {
                    responseFrame.WriteBool(true);
                }
                else
                {
                    responseFrame.WriteBool(false);
                }
                responseFrame.EndReturnValue();
                return responseFrame;
            }
            else
            {
                throw new OperationNotExistException(current.Id, current.Facet, current.Operation);
            }
        }
    }
}
