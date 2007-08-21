// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using Test;

class Twoways
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    internal static void twoways(Ice.Communicator communicator, Test.MyClassPrx p)
    {
        {
            Dictionary<int, int> i = new Dictionary<int, int>();
            i[0] = 1;
            i[1] = 0;

            Dictionary<int, int> o;
            Dictionary<int, int> r;
            r = p.opNV(i, out o);

            test(Ice.Comparer.ValueEquals(i, o));
            test(Ice.Comparer.ValueEquals(i, r));
        }

        {
            Dictionary<string, string> i = new Dictionary<string, string>();
            i["a"] = "b";
            i["b"] = "a";

            Dictionary<string, string> o;
            Dictionary<string, string> r;
            r = p.opNR(i, out o);

            test(Ice.Comparer.ValueEquals(i, o));
            test(Ice.Comparer.ValueEquals(i, r));
        }

        {
            Dictionary<string, Dictionary<int, int>> i = new Dictionary<string, Dictionary<int, int>>();
            Dictionary<int, int> id = new Dictionary<int, int>();
            id[0] = 1;
            id[1] = 0;
            i["a"] = id;
            i["b"] = id;

            Dictionary<string, Dictionary<int, int>> o;
            Dictionary<string, Dictionary<int, int>> r;
            r = p.opNDV(i, out o);

            test(Ice.Comparer.ValueEquals(i, o));
            test(Ice.Comparer.ValueEquals(i, r));
        }

        {
            Dictionary<string, Dictionary<string, string>> i = new Dictionary<string, Dictionary<string, string>>();
            Dictionary<string, string> id = new Dictionary<string, string>();
            id["a"] = "b";
            id["b"] = "a";
            i["a"] = id;
            i["b"] = id;

            Dictionary<string, Dictionary<string, string>> o;
            Dictionary<string, Dictionary<string, string>> r;
            r = p.opNDR(i, out o);

            test(Ice.Comparer.ValueEquals(i, o));
            test(Ice.Comparer.ValueEquals(i, r));
        }

        {
            OV i = new OV();
            i[0] = 1;
            i[1] = 0;

            OV o;
            OV r;
            r = p.opOV(i, out o);

            test(OV.Equals(i, o));
            test(OV.Equals(i, r));
        }

        {
            OR i = new OR();
            i["a"] = "b";
            i["b"] = "a";

            OR o;
            OR r;
            r = p.opOR(i, out o);

            test(OR.Equals(i, o));
            test(OR.Equals(i, r));
        }

        {
            ODV i = new ODV();
            OV id = new OV();
            id[0] = 1;
            id[1] = 0;
            i["a"] = id;
            i["b"] = id;

            ODV o;
            ODV r;
            r = p.opODV(i, out o);

            test(ODV.Equals(i, o));
            test(ODV.Equals(i, r));
        }

        {
            ODR i = new ODR();
            OR id = new OR();
            id["a"] = "b";
            id["b"] = "a";
            i["a"] = id;
            i["b"] = id;

            ODR o;
            ODR r;
            r = p.opODR(i, out o);

            test(ODR.Equals(i, o));
            test(ODR.Equals(i, r));
        }

        {
            Dictionary<string, ODV> i = new Dictionary<string, ODV>();
            OV iid = new OV();
            iid[0] = 1;
            iid[1] = 0;
            ODV id = new ODV();
            id["a"] = iid;
            id["b"] = iid;
            i["a"] = id;
            i["b"] = id;

            Dictionary<string, ODV> o;
            Dictionary<string, ODV> r;
            r = p.opNODV(i, out o);

            test(Ice.Comparer.ValueEquals(i, o));
            test(Ice.Comparer.ValueEquals(i, r));
        }

        {
            Dictionary<string, ODR> i = new Dictionary<string, ODR>();
            OR iid = new OR();
            iid["a"] = "b";
            iid["a"] = "b";
            ODR id = new ODR();
            id["a"] = iid;
            id["b"] = iid;
            i["a"] = id;
            i["b"] = id;

            Dictionary<string, ODR> o;
            Dictionary<string, ODR> r;
            r = p.opNODR(i, out o);

            test(Ice.Comparer.ValueEquals(i, o));
            test(Ice.Comparer.ValueEquals(i, r));
        }

        {
            ONDV i = new ONDV();
            Dictionary<int, int> iid = new Dictionary<int, int>();
            iid[0] = 1;
            iid[1] = 0;
            Dictionary<string, Dictionary<int, int>> id
                = new Dictionary<string, Dictionary<int, int>>();
            id["a"] = iid;
            id["b"] = iid;
            i["a"] = id;
            i["b"] = id;

            ONDV o;
            ONDV r;
            r = p.opONDV(i, out o);

            test(Ice.Comparer.ValueEquals(i, o));
            test(Ice.Comparer.ValueEquals(i, r));
        }

        {
            ONDR i = new ONDR();
            Dictionary<string, string> iid = new Dictionary<string, string>();
            iid["a"] = "b";
            iid["b"] = "a";
            Dictionary<string, Dictionary<string, string>> id
                = new Dictionary<string, Dictionary<string, string>>();
            id["a"] = iid;
            id["b"] = iid;
            i["a"] = id;
            i["b"] = id;

            ONDR o;
            ONDR r;
            r = p.opONDR(i, out o);

            test(Ice.Comparer.ValueEquals(i, o));
            test(Ice.Comparer.ValueEquals(i, r));
        }
    }
}
