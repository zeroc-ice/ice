// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

        public virtual bool check()
        {
            lock(this)
            {
                while(!_called)
                {
                    Monitor.Wait(this, TimeSpan.FromMilliseconds(5000));

                    if(!_called)
                    {
                        return false; // Must be timeout.
                    }
                }

                return true;
            }
        }

        public virtual void called()
        {
            lock(this)
            {
                Debug.Assert(!_called);
                _called = true;
                Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

/*
    private class AMI_MyClass_opNVI : Test.AMI_MyClass_opNV
    {
        public AMI_MyClass_opNVI(Dictionary<int, int> i)
        {
            _i = i;
        }

        public override void ice_response(Dictionary<int, int> r, Dictionary<int, int> o)
        {
            test(Ice.Comparer.ValueEquals(_i, o));
            test(Ice.Comparer.ValueEquals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
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
            test(Ice.Comparer.ValueEquals(_i, o));
            test(Ice.Comparer.ValueEquals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
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
            test(Ice.Comparer.ValueEquals(_i, o));
            test(Ice.Comparer.ValueEquals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
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
            test(Ice.Comparer.ValueEquals(_i, o));
            test(Ice.Comparer.ValueEquals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
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
            test(Ice.Comparer.ValueEquals(_i, o));
            test(Ice.Comparer.ValueEquals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
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
            test(Ice.Comparer.ValueEquals(_i, o));
            test(Ice.Comparer.ValueEquals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
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
            test(Ice.Comparer.ValueEquals(_i, o));
            test(Ice.Comparer.ValueEquals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
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
            test(Ice.Comparer.ValueEquals(_i, o));
            test(Ice.Comparer.ValueEquals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
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
            test(Ice.Comparer.ValueEquals(_i, o));
            test(Ice.Comparer.ValueEquals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
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
            test(Ice.Comparer.ValueEquals(_i, o));
            test(Ice.Comparer.ValueEquals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Dictionary<string, ODR> _i;
        private Callback callback = new Callback();
    }
    */

    internal static void twowaysAMI(Ice.Communicator communicator, Test.MyClassPrx p)
    {
    /*
        {
            Dictionary<int, int> i = new Dictionary<int, int>();
            i[0] = 1;
            i[1] = 0;

            AMI_MyClass_opNVI cb = new AMI_MyClass_opNVI(i);
            p.opNV_async(cb, i);
            test(cb.check());
        }

        {
            Dictionary<string, string> i = new Dictionary<string, string>();
            i["a"] = "b";
            i["b"] = "a";

            AMI_MyClass_opNRI cb = new AMI_MyClass_opNRI(i);
            p.opNR_async(cb, i);
            test(cb.check());
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
            test(cb.check());
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
            test(cb.check());
        }

        {
            OV i = new OV();
            i[0] = 1;
            i[1] = 0;

            AMI_MyClass_opOVI cb = new AMI_MyClass_opOVI(i);
            p.opOV_async(cb, i);
            test(cb.check());
        }

        {
            OR i = new OR();
            i["a"] = "b";
            i["b"] = "a";

            AMI_MyClass_opORI cb = new AMI_MyClass_opORI(i);
            p.opOR_async(cb, i);
            test(cb.check());
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
            test(cb.check());
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
            test(cb.check());
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
            test(cb.check());
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
            test(cb.check());
        }
    */
    }
}
