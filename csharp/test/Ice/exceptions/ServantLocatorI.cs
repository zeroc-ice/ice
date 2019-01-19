//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace exceptions
    {
        public sealed class ServantLocatorI : Ice.ServantLocator
        {
            public Ice.Object locate(Ice.Current curr, out System.Object cookie)
            {
                cookie = null;
                return null;
            }

            public void finished(Ice.Current curr, Ice.Object servant, System.Object cookie)
            {
            }

            public void deactivate(string category)
            {
            }
        }
    }
}
