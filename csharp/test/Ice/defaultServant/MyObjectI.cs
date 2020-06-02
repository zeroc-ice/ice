//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.DefaultServant
{
    public sealed class MyObject : IMyObject
    {
        public void IcePing(Current current)
        {
            string name = current.Identity.Name;

            if (name == "ObjectNotExist")
            {
                throw new ObjectNotExistException(current);
            }
        }

        public string GetName(Current current)
        {
            string name = current.Identity.Name;

            if (name == "ObjectNotExist")
            {
                throw new ObjectNotExistException(current);
            }
            return name;
        }
    }
}
