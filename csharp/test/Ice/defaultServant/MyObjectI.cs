//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.DefaultServant
{
    public sealed class MyObject : Test.IMyObject
    {
        public void IcePing(Current current)
        {
            string name = current.Id.Name;

            if (name == "ObjectNotExist")
            {
                throw new ObjectNotExistException();
            }
        }

        public string GetName(Current current)
        {
            string name = current.Id.Name;

            if (name == "ObjectNotExist")
            {
                throw new ObjectNotExistException();
            }
            return name;
        }
    }
}
