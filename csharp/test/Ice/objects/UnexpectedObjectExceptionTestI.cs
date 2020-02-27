//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice.objects
{
    public sealed class UnexpectedObjectExceptionTest : IObject
    {
        public async ValueTask<OutputStream> DispatchAsync(InputStream istr, Current current)
        {
            var ae = new Test.AlsoEmpty();
            return new OutgoingResponseFrame(current, outputStream => outputStream.WriteClass(ae));
        }
    }
}
