// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
