// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
    oneways(Ice.Communicator communicator, Test.MyClassPrx p)
    {
        p = Test.MyClassPrxHelper.uncheckedCast(p.ice_oneway());

        {
            p.opVoid();
        }

        {

            Ice.ByteHolder b = new Ice.ByteHolder();
            byte r;
            try
            {
                r = p.opByte((byte)0xff, (byte)0x0f, b);
                test(false);
            }
            catch(Ice.TwowayOnlyException ex)
            {
            }
        }
    }
}