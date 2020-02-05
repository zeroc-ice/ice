//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.defaultServant
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
            else if (name == "FacetNotExist")
            {
                throw new ObjectNotExistException();
            }
        }

        public string getName(Current current)
        {
            string name = current.Id.Name;

            if (name == "ObjectNotExist")
            {
                throw new ObjectNotExistException();
            }
            else if (name == "FacetNotExist")
            {
                throw new ObjectNotExistException();
            }

            return name;
        }
    }
}
