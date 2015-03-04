// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

    private class Callback
    {
        internal Callback()
        {
            _called = false;
        }

        public virtual void check()
        {
            _m.Lock();
            try
            {
                while(!_called)
                {
                    _m.Wait();
                }

                _called = false;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public virtual void called()
        {
            _m.Lock();
            try
            {
                Debug.Assert(!_called);
                _called = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        private bool _called;
        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }

    private class AMI_MyClass_opNVI : Test.AMI_MyClass_opNV
    {
        public AMI_MyClass_opNVI(Dictionary<int, int> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<int, int> r, Dictionary<int, int> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<int, int> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opNRI : Test.AMI_MyClass_opNR
    {
        public AMI_MyClass_opNRI(Dictionary<string, string> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<string, string> r, Dictionary<string, string> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, string> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opNDVI : Test.AMI_MyClass_opNDV
    {
        public AMI_MyClass_opNDVI(Dictionary<string, Dictionary<int, int>> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<string, Dictionary<int, int>> r,
                                          Dictionary<string, Dictionary<int, int>> o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, Dictionary<int, int>> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opNDRI : Test.AMI_MyClass_opNDR
    {
        public AMI_MyClass_opNDRI(Dictionary<string, Dictionary<string, string>> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<string, Dictionary<string, string>> r,
                                          Dictionary<string, Dictionary<string, string>> o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, Dictionary<string, string>> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opOVI : Test.AMI_MyClass_opOV
    {
        public AMI_MyClass_opOVI(OV i)
        {
            _i = i;
        }

        public override void ice_response(OV r, OV o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private OV _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opORI : Test.AMI_MyClass_opOR
    {
        public AMI_MyClass_opORI(OR i)
        {
            _i = i;
        }

        public override void ice_response(OR r, OR o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private OR _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opODVI : Test.AMI_MyClass_opODV
    {
        public AMI_MyClass_opODVI(ODV i)
        {
            _i = i;
        }

        public override void ice_response(ODV r, ODV o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private ODV _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opODRI : Test.AMI_MyClass_opODR
    {
        public AMI_MyClass_opODRI(ODR i)
        {
            _i = i;
        }

        public override void ice_response(ODR r, ODR o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private ODR _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opNODVI : Test.AMI_MyClass_opNODV
    {
        public AMI_MyClass_opNODVI(Dictionary<string, ODV> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<string, ODV> r, Dictionary<string, ODV> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, ODV> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opNODRI : Test.AMI_MyClass_opNODR
    {
        public AMI_MyClass_opNODRI(Dictionary<string, ODR> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<string, ODR> r, Dictionary<string, ODR> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, ODR> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opONDVI : Test.AMI_MyClass_opONDV
    {
        public AMI_MyClass_opONDVI(ONDV i)
        {
            _i = i;
        }

        public override void ice_response(ONDV r, ONDV o)
        {
            foreach(string key in _i.Keys)
            {
                foreach(string s in _i[key].Keys)
                {
                    test(Ice.CollectionComparer.Equals(_i[key][s], o[key][s]));
                    test(Ice.CollectionComparer.Equals(_i[key][s], r[key][s]));
                }
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private ONDV _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opONDRI : Test.AMI_MyClass_opONDR
    {
        public AMI_MyClass_opONDRI(ONDR i)
        {
            _i = i;
        }

        public override void ice_response(ONDR r, ONDR o)
        {
            foreach(string key in _i.Keys)
            {
                foreach(string s in _i[key].Keys)
                {
                    test(Ice.CollectionComparer.Equals(_i[key][s], o[key][s]));
                    test(Ice.CollectionComparer.Equals(_i[key][s], r[key][s]));
                }
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private ONDR _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opNDAISI : Test.AMI_MyClass_opNDAIS
    {
        public AMI_MyClass_opNDAISI(Dictionary<string, int[]> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<string, int[]> r, Dictionary<string, int[]> o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, int[]> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opNDCISI : Test.AMI_MyClass_opNDCIS
    {
        public AMI_MyClass_opNDCISI(Dictionary<string, CIS> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<string, CIS> r, Dictionary<string, CIS> o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, CIS> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opNDGISI : Test.AMI_MyClass_opNDGIS
    {
        public AMI_MyClass_opNDGISI(Dictionary<string, List<int>> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<string, List<int>> r, Dictionary<string, List<int>> o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, List<int>> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opNDASSI : Test.AMI_MyClass_opNDASS
    {
        public AMI_MyClass_opNDASSI(Dictionary<string, string[]> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<string, string[]> r, Dictionary<string, string[]> o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, string[]> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opNDCSSI : Test.AMI_MyClass_opNDCSS
    {
        public AMI_MyClass_opNDCSSI(Dictionary<string, CSS> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<string, CSS> r, Dictionary<string, CSS> o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, CSS> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opNDGSSI : Test.AMI_MyClass_opNDGSS
    {
        public AMI_MyClass_opNDGSSI(Dictionary<string, List<string>> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<string, List<string>> r, Dictionary<string, List<string>> o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private Dictionary<string, List<string>> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opODAISI : Test.AMI_MyClass_opODAIS
    {
        public AMI_MyClass_opODAISI(ODAIS i)
        {
            _i = i;
        }

        public override void ice_response(ODAIS r, ODAIS o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private ODAIS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opODCISI : Test.AMI_MyClass_opODCIS
    {
        public AMI_MyClass_opODCISI(ODCIS i)
        {
            _i = i;
        }

        public override void ice_response(ODCIS r, ODCIS o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private ODCIS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opODGISI : Test.AMI_MyClass_opODGIS
    {
        public AMI_MyClass_opODGISI(ODGIS i)
        {
            _i = i;
        }

        public override void ice_response(ODGIS r, ODGIS o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private ODGIS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opODASSI : Test.AMI_MyClass_opODASS
    {
        public AMI_MyClass_opODASSI(ODASS i)
        {
            _i = i;
        }

        public override void ice_response(ODASS r, ODASS o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private ODASS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opODCSSI : Test.AMI_MyClass_opODCSS
    {
        public AMI_MyClass_opODCSSI(ODCSS i)
        {
            _i = i;
        }

        public override void ice_response(ODCSS r, ODCSS o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private ODCSS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opODGSSI : Test.AMI_MyClass_opODGSS
    {
        public AMI_MyClass_opODGSSI(ODGSS i)
        {
            _i = i;
        }

        public override void ice_response(ODGSS r, ODGSS o)
        {
            foreach(string key in _i.Keys)
            {
                test(Ice.CollectionComparer.Equals(_i[key], o[key]));
                test(Ice.CollectionComparer.Equals(_i[key], r[key]));
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private ODGSS _i;
        private Callback callback = new Callback();
    }

    internal static void twowaysAMI(Ice.Communicator communicator, Test.MyClassPrx p)
    {
        {
            Dictionary<int, int> i = new Dictionary<int, int>();
            i[0] = 1;
            i[1] = 0;

            AMI_MyClass_opNVI cb = new AMI_MyClass_opNVI(i);
            p.opNV_async(cb, i);
            cb.check();
        }

        {
            Dictionary<string, string> i = new Dictionary<string, string>();
            i["a"] = "b";
            i["b"] = "a";

            AMI_MyClass_opNRI cb = new AMI_MyClass_opNRI(i);
            p.opNR_async(cb, i);
            cb.check();
        }

        {
            Dictionary<string, Dictionary<int, int>> i = new Dictionary<string, Dictionary<int, int>>();
            Dictionary<int, int> id = new Dictionary<int, int>();
            id[0] = 1;
            id[1] = 0;
            i["a"] = id;
            i["b"] = id;

            AMI_MyClass_opNDVI cb = new AMI_MyClass_opNDVI(i);
            p.opNDV_async(cb, i);
            cb.check();
        }

        {
            Dictionary<string, Dictionary<string, string>> i = new Dictionary<string, Dictionary<string, string>>();
            Dictionary<string, string> id = new Dictionary<string, string>();
            id["a"] = "b";
            id["b"] = "a";
            i["a"] = id;
            i["b"] = id;

            AMI_MyClass_opNDRI cb = new AMI_MyClass_opNDRI(i);
            p.opNDR_async(cb, i);
            cb.check();
        }

        {
            OV i = new OV();
            i[0] = 1;
            i[1] = 0;

            AMI_MyClass_opOVI cb = new AMI_MyClass_opOVI(i);
            p.opOV_async(cb, i);
            cb.check();
        }

        {
            OR i = new OR();
            i["a"] = "b";
            i["b"] = "a";

            AMI_MyClass_opORI cb = new AMI_MyClass_opORI(i);
            p.opOR_async(cb, i);
            cb.check();
        }

        {
            ODV i = new ODV();
            OV id = new OV();
            id[0] = 1;
            id[1] = 0;
            i["a"] = id;
            i["b"] = id;

            AMI_MyClass_opODVI cb = new AMI_MyClass_opODVI(i);
            p.opODV_async(cb, i);
            cb.check();
        }

        {
            ODR i = new ODR();
            OR id = new OR();
            id["a"] = "b";
            id["b"] = "a";
            i["a"] = id;
            i["b"] = id;

            AMI_MyClass_opODRI cb = new AMI_MyClass_opODRI(i);
            p.opODR_async(cb, i);
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

            AMI_MyClass_opNODVI cb = new AMI_MyClass_opNODVI(i);
            p.opNODV_async(cb, i);
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

            AMI_MyClass_opNODRI cb = new AMI_MyClass_opNODRI(i);
            p.opNODR_async(cb, i);
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

            AMI_MyClass_opONDVI cb = new AMI_MyClass_opONDVI(i);
            p.opONDV_async(cb, i);
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

            AMI_MyClass_opONDRI cb = new AMI_MyClass_opONDRI(i);
            p.opONDR_async(cb, i);
            cb.check();
        }

        {
            int[] ii = new int[] { 1, 2 };
            Dictionary<string, int[]> i = new Dictionary<string, int[]>();
            i["a"] = ii;
            i["b"] = ii;

            AMI_MyClass_opNDAISI cb = new AMI_MyClass_opNDAISI(i);
            p.opNDAIS_async(cb, i);
            cb.check();
        }

        {
            CIS ii = new CIS();
            ii.Add(1);
            ii.Add(2);
            Dictionary<string, CIS> i = new Dictionary<string, CIS>();
            i["a"] = ii;
            i["b"] = ii;

            AMI_MyClass_opNDCISI cb = new AMI_MyClass_opNDCISI(i);
            p.opNDCIS_async(cb, i);
            cb.check();
        }

        {
            List<int> ii = new List<int>();
            ii.Add(1);
            ii.Add(2);
            Dictionary<string, List<int>> i = new Dictionary<string, List<int>>();
            i["a"] = ii;
            i["b"] = ii;

            AMI_MyClass_opNDGISI cb = new AMI_MyClass_opNDGISI(i);
            p.opNDGIS_async(cb, i);
            cb.check();
        }

        {
            string[] ii = new string[] { "a", "b" };
            Dictionary<string, string[]> i = new Dictionary<string, string[]>();
            i["a"] = ii;
            i["b"] = ii;

            AMI_MyClass_opNDASSI cb = new AMI_MyClass_opNDASSI(i);
            p.opNDASS_async(cb, i);
            cb.check();
        }

        {
            CSS ii = new CSS();
            ii.Add("a");
            ii.Add("b");
            Dictionary<string, CSS> i = new Dictionary<string, CSS>();
            i["a"] = ii;
            i["b"] = ii;

            AMI_MyClass_opNDCSSI cb = new AMI_MyClass_opNDCSSI(i);
            p.opNDCSS_async(cb, i);
            cb.check();
        }

        {
            List<string> ii = new List<string>();
            ii.Add("a");
            ii.Add("b");
            Dictionary<string, List<string>> i = new Dictionary<string, List<string>>();
            i["a"] = ii;
            i["b"] = ii;

            AMI_MyClass_opNDGSSI cb = new AMI_MyClass_opNDGSSI(i);
            p.opNDGSS_async(cb, i);
            cb.check();
        }

        {
            int[] ii = new int[] { 1, 2 };
            ODAIS i = new ODAIS();
            i["a"] = ii;
            i["b"] = ii;

            AMI_MyClass_opODAISI cb = new AMI_MyClass_opODAISI(i);
            p.opODAIS_async(cb, i);
            cb.check();
        }

        {
            CIS ii = new CIS();
            ii.Add(1);
            ii.Add(2);
            ODCIS i = new ODCIS();
            i["a"] = ii;
            i["b"] = ii;

            AMI_MyClass_opODCISI cb = new AMI_MyClass_opODCISI(i);
            p.opODCIS_async(cb, i);
            cb.check();
        }

        {
            List<int> ii = new List<int>();
            ii.Add(1);
            ii.Add(2);
            ODGIS i = new ODGIS();
            i["a"] = ii;
            i["b"] = ii;

            AMI_MyClass_opODGISI cb = new AMI_MyClass_opODGISI(i);
            p.opODGIS_async(cb, i);
            cb.check();
        }

        {
            string[] ii = new string[] { "a", "b" };
            ODASS i = new ODASS();
            i["a"] = ii;
            i["b"] = ii;

            AMI_MyClass_opODASSI cb = new AMI_MyClass_opODASSI(i);
            p.opODASS_async(cb, i);
            cb.check();
        }

        {
            CSS ii = new CSS();
            ii.Add("a");
            ii.Add("b");
            ODCSS i = new ODCSS();
            i["a"] = ii;
            i["b"] = ii;

            AMI_MyClass_opODCSSI cb = new AMI_MyClass_opODCSSI(i);
            p.opODCSS_async(cb, i);
            cb.check();
        }

        {
            List<string> ii = new List<string>();
            ii.Add("a");
            ii.Add("b");
            ODGSS i = new ODGSS();
            i["a"] = ii;
            i["b"] = ii;

            AMI_MyClass_opODGSSI cb = new AMI_MyClass_opODGSSI(i);
            p.opODGSS_async(cb, i);
            cb.check();
        }
    }
}
