// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;
using System.Collections.Generic;
using Test;

public class TwowaysAMI
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.SystemException();
        }
    }

    private class CallbackBase
    {
        internal CallbackBase()
        {
            _called = false;
        }

        public virtual void check()
        {
            lock(this)
            {
                while(!_called)
                {
                    System.Threading.Monitor.Wait(this);
                }

                _called = false;
            }
        }

        public virtual void called()
        {
            lock(this)
            {
                Debug.Assert(!_called);
                _called = true;
                System.Threading.Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

    private class Callback
    {
        public void opNVI(Ice.AsyncResult result)
        {
            Dictionary<int, int> i = (Dictionary<int, int>)result.AsyncState;
            Dictionary<int, int> o;
            Dictionary<int, int> r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opNV(out o, result);
            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
            callback.called();
        }

        public void opNRI(Ice.AsyncResult result)
        {
            Dictionary<string, string> i = (Dictionary<string, string>)result.AsyncState;
            Dictionary<string, string> o;
            Dictionary<string, string> r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opNR(out o, result);
            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
            callback.called();
        }

        public void opNDVI(Ice.AsyncResult result)
        {
            Dictionary<string, Dictionary<int, int>> i = (Dictionary<string, Dictionary<int, int>>)result.AsyncState;
            Dictionary<string, Dictionary<int, int>> o;
            Dictionary<string, Dictionary<int, int>> r = 
                MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opNDV(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opNDRI(Ice.AsyncResult result)
        {
            Dictionary<string, Dictionary<string, string>> i = 
                (Dictionary<string, Dictionary<string, string>>)result.AsyncState;
            Dictionary<string, Dictionary<string, string>> o;
            Dictionary<string, Dictionary<string, string>> r = 
                MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opNDR(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opOVI(Ice.AsyncResult result)
        {
            OV i = (OV)result.AsyncState;
            OV o;
            OV r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opOV(out o, result);
            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
            callback.called();
        }

        public void opORI(Ice.AsyncResult result)
        {
            OR i = (OR)result.AsyncState;
            OR o;
            OR r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opOR(out o, result);
            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
            callback.called();
        }

        public void opODVI(Ice.AsyncResult result)
        {
            ODV i = (ODV)result.AsyncState;
            ODV o;
            ODV r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opODV(out o, result);
            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
            callback.called();
        }

        public void opODRI(Ice.AsyncResult result)
        {
            ODR i = (ODR)result.AsyncState;
            ODR o;
            ODR r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opODR(out o, result);
            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
            callback.called();
        }

        public void opNODVI(Ice.AsyncResult result)
        {
            Dictionary<string, ODV> i = (Dictionary<string, ODV>)result.AsyncState;
            Dictionary<string, ODV> o;
            Dictionary<string, ODV> r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opNODV(out o, result);
            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
            callback.called();
        }

        public void opNODRI(Ice.AsyncResult result)
        {
            Dictionary<string, ODR> i = (Dictionary<string, ODR>)result.AsyncState;
            Dictionary<string, ODR> o;
            Dictionary<string, ODR> r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opNODR(out o, result);
            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
            callback.called();
        }

        public void opONDVI(Ice.AsyncResult result)
        {
            ONDV i = (ONDV)result.AsyncState;
            ONDV o;
            ONDV r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opONDV(out o, result);
            foreach(string key in i.Keys)
            {
                foreach(string s in i[key].Keys)
                {
                    test(Ice.CollectionComparer.Equals(i[key][s], o[key][s]));
                    test(Ice.CollectionComparer.Equals(i[key][s], r[key][s]));
                }
            }
            callback.called();
        }

        public void opONDRI(Ice.AsyncResult result)
        {
            ONDR i = (ONDR)result.AsyncState;
            ONDR o;
            ONDR r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opONDR(out o, result);
            foreach(string key in i.Keys)
            {
                foreach(string s in i[key].Keys)
                {
                    test(Ice.CollectionComparer.Equals(i[key][s], o[key][s]));
                    test(Ice.CollectionComparer.Equals(i[key][s], r[key][s]));
                }
            }
            callback.called();
        }

        public void opNDAISI(Ice.AsyncResult result)
        {
            Dictionary<string, int[]> i = (Dictionary<string, int[]>)result.AsyncState;
            Dictionary<string, int[]> o;
            Dictionary<string, int[]> r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opNDAIS(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opNDCISI(Ice.AsyncResult result)
        {
            Dictionary<string, CIS> i = (Dictionary<string, CIS>)result.AsyncState;
            Dictionary<string, CIS> o;
            Dictionary<string, CIS> r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opNDCIS(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opNDGISI(Ice.AsyncResult result)
        {
            Dictionary<string, List<int>> i = (Dictionary<string, List<int>>)result.AsyncState;
            Dictionary<string, List<int>> o;
            Dictionary<string, List<int>> r = 
                MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opNDGIS(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opNDASSI(Ice.AsyncResult result)
        {
            Dictionary<string, string[]> i = (Dictionary<string,string[]>)result.AsyncState;
            Dictionary<string, string[]> o;
            Dictionary<string, string[]> r = 
                MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opNDASS(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opNDCSSI(Ice.AsyncResult result)
        {
            Dictionary<string, CSS> i = (Dictionary<string,CSS>)result.AsyncState;
            Dictionary<string, CSS> o;
            Dictionary<string, CSS> r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opNDCSS(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opNDGSSI(Ice.AsyncResult result)
        {
            Dictionary<string, List<string>> i = (Dictionary<string,List<string>>)result.AsyncState;
            Dictionary<string, List<string>> o;
            Dictionary<string, List<string>> r = 
                MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opNDGSS(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opODAISI(Ice.AsyncResult result)
        {
            ODAIS i = (ODAIS)result.AsyncState;
            ODAIS o;
            ODAIS r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opODAIS(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opODCISI(Ice.AsyncResult result)
        {
            ODCIS i = (ODCIS)result.AsyncState;
            ODCIS o;
            ODCIS r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opODCIS(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opODGISI(Ice.AsyncResult result)
        {
            ODGIS i = (ODGIS)result.AsyncState;
            ODGIS o;
            ODGIS r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opODGIS(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opODASSI(Ice.AsyncResult result)
        {
            ODASS i = (ODASS)result.AsyncState;
            ODASS o;
            ODASS r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opODASS(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opODCSSI(Ice.AsyncResult result)
        {
            ODCSS i = (ODCSS)result.AsyncState;
            ODCSS o;
            ODCSS r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opODCSS(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public void opODGSSI(Ice.AsyncResult result)
        {
            ODGSS i = (ODGSS)result.AsyncState;
            ODGSS o;
            ODGSS r = MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opODGSS(out o, result);
            foreach(string key in i.Keys)
            {
                test(Ice.CollectionComparer.Equals(i[key], o[key]));
                test(Ice.CollectionComparer.Equals(i[key], r[key]));
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private CallbackBase callback = new CallbackBase();
    }

    internal static void twowaysAMI(Ice.Communicator communicator, Test.MyClassPrx p)
    {
        {
            Dictionary<int, int> i = new Dictionary<int, int>();
            i[0] = 1;
            i[1] = 0;

            Callback cb = new Callback();
            p.begin_opNV(i, null, cb.opNVI, i);
            cb.check();
        }

        {
            Dictionary<string, string> i = new Dictionary<string, string>();
            i["a"] = "b";
            i["b"] = "a";

            Callback cb = new Callback();
            p.begin_opNR(i, null, cb.opNRI, i);
            cb.check();
        }

        {
            Dictionary<string, Dictionary<int, int>> i = new Dictionary<string, Dictionary<int, int>>();
            Dictionary<int, int> id = new Dictionary<int, int>();
            id[0] = 1;
            id[1] = 0;
            i["a"] = id;
            i["b"] = id;

            Callback cb = new Callback();
            p.begin_opNDV(i, null, cb.opNDVI, i);
            cb.check();
        }

        {
            Dictionary<string, Dictionary<string, string>> i = new Dictionary<string, Dictionary<string, string>>();
            Dictionary<string, string> id = new Dictionary<string, string>();
            id["a"] = "b";
            id["b"] = "a";
            i["a"] = id;
            i["b"] = id;

            Callback cb = new Callback();
            p.begin_opNDR(i, null, cb.opNDRI, i);
            cb.check();
        }

        {
            OV i = new OV();
            i[0] = 1;
            i[1] = 0;

            Callback cb = new Callback();
            p.begin_opOV(i, null, cb.opOVI, i);
            cb.check();
        }

        {
            OR i = new OR();
            i["a"] = "b";
            i["b"] = "a";

            Callback cb = new Callback();
            p.begin_opOR(i, null, cb.opORI, i);
            cb.check();
        }

        {
            ODV i = new ODV();
            OV id = new OV();
            id[0] = 1;
            id[1] = 0;
            i["a"] = id;
            i["b"] = id;

            Callback cb = new Callback();
            p.begin_opODV(i, null, cb.opODVI, i);
            cb.check();
        }

        {
            ODR i = new ODR();
            OR id = new OR();
            id["a"] = "b";
            id["b"] = "a";
            i["a"] = id;
            i["b"] = id;

            Callback cb = new Callback();
            p.begin_opODR(i, null, cb.opODRI, i);
            cb.check();
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

            Callback cb = new Callback();
            p.begin_opNODV(i, null, cb.opNODVI, i);
            cb.check();
        }

        {
            Dictionary<string, ODR> i = new Dictionary<string, ODR>();
            OR iid = new OR();
            iid["a"] = "b";
            iid["b"] = "a";
            ODR id = new ODR();
            id["a"] = iid;
            id["b"] = iid;
            i["a"] = id;
            i["b"] = id;

            Callback cb = new Callback();
            p.begin_opNODR(i, null, cb.opNODRI, i);
            cb.check();
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

            Callback cb = new Callback();
            p.begin_opONDV(i, null, cb.opONDVI, i);
            cb.check();
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

            Callback cb = new Callback();
            p.begin_opONDR(i, null, cb.opONDRI, i);
            cb.check();
        }

        {
            int[] ii = new int[] { 1, 2 };
            Dictionary<string, int[]> i = new Dictionary<string, int[]>();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opNDAIS(i, null, cb.opNDAISI, i);
            cb.check();
        }

        {
            CIS ii = new CIS();
            ii.Add(1);
            ii.Add(2);
            Dictionary<string, CIS> i = new Dictionary<string, CIS>();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opNDCIS(i, null, cb.opNDCISI, i);
            cb.check();
        }

        {
            List<int> ii = new List<int>();
            ii.Add(1);
            ii.Add(2);
            Dictionary<string, List<int>> i = new Dictionary<string, List<int>>();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opNDGIS(i, null, cb.opNDGISI, i);
            cb.check();
        }

        {
            string[] ii = new string[] { "a", "b" };
            Dictionary<string, string[]> i = new Dictionary<string, string[]>();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opNDASS(i, null, cb.opNDASSI, i);
            cb.check();
        }

        {
            CSS ii = new CSS();
            ii.Add("a");
            ii.Add("b");
            Dictionary<string, CSS> i = new Dictionary<string, CSS>();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opNDCSS(i, null, cb.opNDCSSI, i);
            cb.check();
        }

        {
            List<string> ii = new List<string>();
            ii.Add("a");
            ii.Add("b");
            Dictionary<string, List<string>> i = new Dictionary<string, List<string>>();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opNDGSS(i, null, cb.opNDGSSI, i);
            cb.check();
        }

        {
            int[] ii = new int[] { 1, 2 };
            ODAIS i = new ODAIS();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opODAIS(i, null, cb.opODAISI, i);
            cb.check();
        }

        {
            CIS ii = new CIS();
            ii.Add(1);
            ii.Add(2);
            ODCIS i = new ODCIS();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opODCIS(i, null, cb.opODCISI, i);
            cb.check();
        }

        {
            List<int> ii = new List<int>();
            ii.Add(1);
            ii.Add(2);
            ODGIS i = new ODGIS();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opODGIS(i, null, cb.opODGISI, i);
            cb.check();
        }

        {
            string[] ii = new string[] { "a", "b" };
            ODASS i = new ODASS();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opODASS(i, null, cb.opODASSI, i);
            cb.check();
        }

        {
            CSS ii = new CSS();
            ii.Add("a");
            ii.Add("b");
            ODCSS i = new ODCSS();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opODCSS(i, null, cb.opODCSSI, i);
            cb.check();
        }

        {
            List<string> ii = new List<string>();
            ii.Add("a");
            ii.Add("b");
            ODGSS i = new ODGSS();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opODGSS(i, null, cb.opODGSSI, i);
            cb.check();
        }
    }
}
