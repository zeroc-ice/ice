//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.exceptions
{
    public sealed class ServantLocator : IServantLocator
    {
        public Disp? Locate(Current curr, out object cookie)
        {
            cookie = null;
            return null;
        }

        public void Finished(Current curr, Disp servant, object cookie)
        {
        }

        public void Deactivate(string category)
        {
        }
    }
}
