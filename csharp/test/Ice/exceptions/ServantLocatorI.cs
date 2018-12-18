// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    namespace exceptions
    {
        public sealed class ServantLocatorI : Ice.ServantLocator
        {
            public Ice.Object locate(Ice.Current curr, out System.Object cookie)
            {
                cookie = null;
                return null;
            }

            public void finished(Ice.Current curr, Ice.Object servant, System.Object cookie)
            {
            }

            public void deactivate(string category)
            {
            }
        }
    }
}
