// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class Oneways
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.SystemException();
        }
    }

    internal static void oneways(Test.TestHelper helper, Test.MyClassPrx p)
    {
        Ice.Communicator communicator = helper.communicator();
        p = Test.MyClassPrxHelper.uncheckedCast(p.ice_oneway());

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
            byte b;
            try
            {
                p.opByte((byte)0xff, (byte)0x0f, out b);
                test(false);
            }
            catch(Ice.TwowayOnlyException)
            {
            }
        }
    }
}
