// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.DefaultServant
{
    public sealed class MyObject : IMyObject
    {
        public void IcePing(Current current, CancellationToken cancel)
        {
            string name = current.Identity.Name;

            if (name == "ObjectNotExist")
            {
                throw new ObjectNotExistException();
            }
        }

        public string GetName(Current current, CancellationToken cancel)
        {
            string name = current.Identity.Name;

            if (name == "ObjectNotExist")
            {
                throw new ObjectNotExistException();
            }
            return name;
        }
    }
}
