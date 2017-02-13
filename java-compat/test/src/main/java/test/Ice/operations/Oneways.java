// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyClassPrxHelper;

class Oneways
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static void
    oneways(test.Util.Application app, MyClassPrx p)
    {
        p = MyClassPrxHelper.uncheckedCast(p.ice_oneway());

        {
            p.ice_ping();
        }

        {
            p.opVoid();
        }

        {
            p.opIdempotent();
        }

        {
            p.opNonmutating();
        }

        {

            Ice.ByteHolder b = new Ice.ByteHolder();
            try
            {
                p.opByte((byte)0xff, (byte)0x0f, b);
                test(false);
            }
            catch(Ice.TwowayOnlyException ex)
            {
            }
        }
    }
}
