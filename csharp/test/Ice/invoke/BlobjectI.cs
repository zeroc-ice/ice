//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice.invoke
{
    public class BlobjectI : IObject
    {
        public async ValueTask<OutgoingResponseFrame> DispatchAsync(IncomingRequestFrame request, Current current)
        {
            if (current.Operation.Equals("opOneway"))
            {
                if (!current.IsOneway)
                {
                    // If called two-way, return exception to caller.
                    throw new Test.MyException();
                }
                return OutgoingResponseFrame.WithVoidReturnValue(current);
            }
            else if (current.Operation.Equals("opString"))
            {
                string s = request.ReadParamList(InputStream.IceReaderIntoString);
                var responseFrame = OutgoingResponseFrame.WithReturnValue(current, format: null, (s, s),
                    (OutputStream ostr, (string ReturnValue, string s2) value) =>
                    {
                        ostr.WriteString(value.ReturnValue);
                        ostr.WriteString(value.s2);
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
                return OutgoingResponseFrame.WithVoidReturnValue(current);
            }
            else if (current.Operation.Equals("ice_isA"))
            {
                string s = request.ReadParamList(InputStream.IceReaderIntoString);
                OutgoingResponseFrame responseFrame = OutgoingResponseFrame.WithReturnValue(current, format: null,
                    s.Equals("::Test::MyClass"), OutputStream.IceWriterFromBool);
                return responseFrame;
            }
            else
            {
                throw new OperationNotExistException(current.Id, current.Facet, current.Operation);
            }
        }
    }
}
