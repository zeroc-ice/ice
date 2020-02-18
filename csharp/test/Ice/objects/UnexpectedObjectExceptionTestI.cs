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
            var ostr = IceInternal.Protocol.StartResponseFrame(current);
            var ae = new Test.AlsoEmpty();
            ostr.WriteClass(ae);
            ostr.EndEncapsulation();
            return ostr;
        }
    }
}
