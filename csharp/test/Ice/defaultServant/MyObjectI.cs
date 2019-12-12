//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace defaultServant
    {
        public sealed class MyObjectI : Ice.Object<Test.MyObject, Test.MyObjectTraits>, Test.MyObject
        {
            public override void
            IcePing(Ice.Current current)
            {
                string name = current.Id.name;

                if (name == "ObjectNotExist")
                {
                    throw new Ice.ObjectNotExistException();
                }
                else if (name == "FacetNotExist")
                {
                    throw new Ice.FacetNotExistException();
                }
            }

            public string
            getName(Ice.Current current)
            {
                string name = current.Id.name;

                if (name == "ObjectNotExist")
                {
                    throw new Ice.ObjectNotExistException();
                }
                else if (name == "FacetNotExist")
                {
                    throw new Ice.FacetNotExistException();
                }

                return name;
            }
        }
    }
}
