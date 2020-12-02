// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Invoke
{
    public class BlobjectI : IObject
    {
        public ValueTask<OutgoingResponseFrame> DispatchAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
        {
            if (current.Operation.Equals("opOneway"))
            {
                if (!current.IsOneway)
                {
                    // If called two-way, return exception to caller.
                    throw new MyException();
                }
                return new ValueTask<OutgoingResponseFrame>(OutgoingResponseFrame.WithVoidReturnValue(current));
            }
            else if (current.Operation.Equals("opString"))
            {
                string s = request.ReadArgs(current.Connection, InputStream.IceReaderIntoString);
                var responseFrame = OutgoingResponseFrame.WithReturnValue(
                    current,
                    compress: false,
                    format: default,
                    (s, s),
                    (OutputStream ostr, (string ReturnValue, string s2) value) =>
                    {
                        ostr.WriteString(value.ReturnValue);
                        ostr.WriteString(value.s2);
                    });
                return new ValueTask<OutgoingResponseFrame>(responseFrame);
            }
            else if (current.Operation.Equals("opException"))
            {
                if (current.Context.ContainsKey("raise"))
                {
                    throw new MyException();
                }
                return new ValueTask<OutgoingResponseFrame>(
                    new OutgoingResponseFrame(request, new MyException()));
            }
            else if (current.Operation.Equals("shutdown"))
            {
                current.Communicator.ShutdownAsync();
                return new ValueTask<OutgoingResponseFrame>(OutgoingResponseFrame.WithVoidReturnValue(current));
            }
            else if (current.Operation.Equals("ice_isA"))
            {
                string s = request.ReadArgs(current.Connection, InputStream.IceReaderIntoString);
                var responseFrame = OutgoingResponseFrame.WithReturnValue(current,
                                                                          compress: false,
                                                                          format: default,
                                                                          s == "::ZeroC::Ice::Test::Invoke::MyClass",
                                                                          OutputStream.IceWriterFromBool);
                return new ValueTask<OutgoingResponseFrame>(responseFrame);
            }
            else
            {
                throw new OperationNotExistException();
            }
        }
    }
}
