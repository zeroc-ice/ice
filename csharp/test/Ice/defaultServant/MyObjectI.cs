// Copyright (c) ZeroC, Inc.

namespace Ice.defaultServant
{
        public sealed class MyObjectI : Test.MyObjectDisp_, Ice.Object
        {
            public void
            ice_ping(Ice.Current current)
            {
                string name = current.id.name;

                if (name == "ObjectNotExist")
                {
                    throw new Ice.ObjectNotExistException();
                }
                else if (name == "FacetNotExist")
                {
                    throw new Ice.FacetNotExistException();
                }
            }

            public override string
            getName(Ice.Current current)
            {
                string name = current.id.name;

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

