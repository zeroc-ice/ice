//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace exceptions
    {
        public sealed class ServantLocatorI : Ice.ServantLocator
        {
            public Disp? locate(Ice.Current curr, out object cookie)
            {
                cookie = null;
                return null;
            }

            public void finished(Ice.Current curr, Disp servant, object cookie)
            {
            }

            public void deactivate(string category)
            {
            }
        }
    }
}
