//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice.objects
{
    public sealed class UnexpectedObjectExceptionTest : IObject
    {
        public ValueTask<OutputStream> DispatchAsync(InputStream istr, Current current)
        {
            var ae = new Test.AlsoEmpty();
            var responseFrame = new OutgoingResponseFrame(current);
            responseFrame.StartReturnValue();
            responseFrame.WriteClass(ae);
            responseFrame.EndReturnValue();
            return new ValueTask<OutputStream>(responseFrame);
        }
    }
}
