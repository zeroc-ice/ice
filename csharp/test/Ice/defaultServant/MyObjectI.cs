//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.defaultServant
{
    public sealed class MyObject : Object<Test.IMyObject, Test.MyObjectTraits>, Test.IMyObject
    {
        public override void
        IcePing(Current current)
        {
            string name = current.Id.name;

            if (name == "ObjectNotExist")
            {
                throw new ObjectNotExistException();
            }
            else if (name == "FacetNotExist")
            {
                throw new FacetNotExistException();
            }
        }

        public string
        getName(Current current)
        {
            string name = current.Id.name;

            if (name == "ObjectNotExist")
            {
                throw new ObjectNotExistException();
            }
            else if (name == "FacetNotExist")
            {
                throw new FacetNotExistException();
            }

            return name;
        }
    }
}
