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
using System.Collections.Generic;
using Test;
using System.Threading.Tasks;

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
            int[] ii = new int[] { 1, 2 };
            Dictionary<string, int[]> i = new Dictionary<string, int[]>();
            i["a"] = ii;
            i["b"] = ii;

            Callback cb = new Callback();
            p.begin_opNDAIS(i, null, cb.opNDAISI, i);
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
    }
}
