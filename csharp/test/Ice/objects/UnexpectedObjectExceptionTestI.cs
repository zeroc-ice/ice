//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice.objects
{
    public sealed class UnexpectedObjectExceptionTest : IObject
    {
        public ValueTask<OutputStream>? Dispatch(InputStream istr, Current current)
        {
            var ostr = IceInternal.Protocol.StartResponseFrame(current);
            var ae = new Test.AlsoEmpty();
            ostr.WriteClass(ae);
            ostr.EndEncapsulation();
            return new ValueTask<OutputStream>(ostr);
        }
    }
}
