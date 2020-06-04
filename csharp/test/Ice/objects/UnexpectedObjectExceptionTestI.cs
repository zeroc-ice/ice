//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Objects
{
    public sealed class UnexpectedObjectExceptionTest : IObject
    {
        public ValueTask<OutgoingResponseFrame> DispatchAsync(IncomingRequestFrame request, Current current)
        {
            var ae = new AlsoEmpty();
            var responseFrame = OutgoingResponseFrame.WithReturnValue(current, format: null,
                ae, (OutputStream ostr, AlsoEmpty ae) => ostr.WriteClass(ae));
            return new ValueTask<OutgoingResponseFrame>(responseFrame);
        }
    }
}
