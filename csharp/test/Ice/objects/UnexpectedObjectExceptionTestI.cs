//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice.objects
{
    public sealed class UnexpectedObjectExceptionTest : IObject
    {
        public ValueTask<OutgoingResponseFrame> DispatchAsync(InputStream istr, Current current)
        {
            var ae = new Test.AlsoEmpty();
            var responseFrame = OutgoingResponseFrame.WithReturnValue(current.Encoding,
                current.Adapter.Communicator.DefaultFormat, ae,
                (OutputStream ostr, Test.AlsoEmpty ae) => ostr.WriteClass(ae));
            return new ValueTask<OutgoingResponseFrame>(responseFrame);
        }
    }
}
