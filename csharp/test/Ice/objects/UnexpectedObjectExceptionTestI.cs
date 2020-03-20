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
            var responseFrame = new OutgoingResponseFrame(current.Encoding);
            OutputStream ostr = responseFrame.StartPayload();
            ostr.WriteClass(ae);
            ostr.Save();
            return new ValueTask<OutgoingResponseFrame>(responseFrame);
        }
    }
}
