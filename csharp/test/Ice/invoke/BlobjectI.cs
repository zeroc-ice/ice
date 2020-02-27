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
                // TODO: replace by shared "fake" empty
                return new OutgoingResponseFrame(current);
            }
            else if (current.Operation.Equals("opString"))
            {
                string s = istr.ReadString();
                var responseFrame = new OutgoingResponseFrame(current,
                    outputStream =>
                    {
                        outputStream.WriteString(s);
                        outputStream.WriteString(s);
                    });
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
                return new OutgoingResponseFrame(current);
            }
            else if (current.Operation.Equals("ice_isA"))
            {
                string s = istr.ReadString();
                var responseFrame = new OutgoingResponseFrame(current,
                    outputStream =>
                    {
                        if (s.Equals("::Test::MyClass"))
                        {
                            outputStream.WriteBool(true);
                        }
                        else
                        {
                            outputStream.WriteBool(false);
                        }
                    });
                return responseFrame;
            }
            else
            {
                throw new OperationNotExistException(current.Id, current.Facet, current.Operation);
            }
        }
    }
}
