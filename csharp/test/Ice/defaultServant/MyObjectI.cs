//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.defaultServant
{
    public sealed class MyObject : Object<Test.IMyObject>, Test.IMyObject
    {
        public override void
        IcePing(Current current)
        {
            string name = current.Id.Name;

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
            string name = current.Id.Name;

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
