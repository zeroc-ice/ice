//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.exceptions
{
    public sealed class ServantLocator : IServantLocator
    {
        public Disp? locate(Current curr, out object cookie)
        {
            cookie = null;
            return null;
        }

        public void finished(Current curr, Disp servant, object cookie)
        {
        }

        public void deactivate(string category)
        {
        }
    }
}
