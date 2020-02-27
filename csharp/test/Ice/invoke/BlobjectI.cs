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
                return OutgoingResponseFrame.Empty(current);
            }
            else if (current.Operation.Equals("opString"))
            {
                string s = istr.ReadString();
                var responseFrame = OutgoingResponseFrame.Start(current);
                responseFrame.WriteString(s);
                responseFrame.WriteString(s);
                responseFrame.EndPayload();
                return responseFrame;
            }
            else if (current.Operation.Equals("opException"))
            {
                if (current.Context.ContainsKey("raise"))
                {
                    throw new Test.MyException();
                }
                var ex = new Test.MyException();
                var responseFrame = OutgoingResponseFrame.StartFailure(current);
                responseFrame.WriteException(ex);
                responseFrame.EndPayload();
                return responseFrame;
            }
            else if (current.Operation.Equals("shutdown"))
            {
                current.Adapter.Communicator.Shutdown();
                return OutgoingResponseFrame.Empty(current);
            }
            else if (current.Operation.Equals("ice_isA"))
            {
                string s = istr.ReadString();
                var responseFrame = OutgoingResponseFrame.Start(current);
                if (s.Equals("::Test::MyClass"))
                {
                    responseFrame.WriteBool(true);
                }
                else
                {
                    responseFrame.WriteBool(false);
                }
                responseFrame.EndPayload();
                return responseFrame;
            }
            else
            {
                throw new OperationNotExistException(current.Id, current.Facet, current.Operation);
            }
        }
    }
}
