// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace defaultServant
    {
        public sealed class MyObjectI : Test.MyObjectDisp_
        {
            public override void
            ice_ping(Ice.Current current)
            {
                string name = current.id.name;

                if(name == "ObjectNotExist")
                {
                    throw new Ice.ObjectNotExistException();
                }
                else if(name == "FacetNotExist")
                {
                    throw new Ice.FacetNotExistException();
                }
            }

            public override string
            getName(Ice.Current current)
            {
                string name = current.id.name;

                if(name == "ObjectNotExist")
                {
                    throw new Ice.ObjectNotExistException();
                }
                else if(name == "FacetNotExist")
                {
                    throw new Ice.FacetNotExistException();
                }

                return name;
            }
        }
    }
}
