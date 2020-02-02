//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.exceptions
{
    public sealed class ServantLocator : IServantLocator
    {
        public IObject? Locate(Current curr, out object cookie)
        {
            cookie = null;
            return null;
        }

        public void Finished(Current curr, IObject servant, object cookie)
        {
        }

        public void Deactivate(string category)
        {
        }
    }
}
