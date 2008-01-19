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
using Test;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
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
                
                _called = false;
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
    
    private class AMI_Test_baseAsBaseI : AMI_TestIntf_baseAsBase
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Base b)
            {
                AllTests.test(b.b.Equals("Base.b"));
                AllTests.test(b.GetType().Name.Equals("Base"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_unknownDerivedAsBaseI : AMI_TestIntf_unknownDerivedAsBase
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Base b)
            {
                AllTests.test(b.b.Equals("UnknownDerived.b"));
                AllTests.test(b.GetType().Name.Equals("Base"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_knownDerivedAsBaseI : AMI_TestIntf_knownDerivedAsBase
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownDerived k)
            {
                AllTests.test(k.b.Equals("KnownDerived.b"));
                AllTests.test(k.kd.Equals("KnownDerived.kd"));
                AllTests.test(k.GetType().Name.Equals("KnownDerived"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_knownDerivedAsKnownDerivedI : AMI_TestIntf_knownDerivedAsKnownDerived
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownDerived k)
            {
                AllTests.test(k.b.Equals("KnownDerived.b"));
                AllTests.test(k.kd.Equals("KnownDerived.kd"));
                AllTests.test(k.GetType().Name.Equals("KnownDerived"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_unknownIntermediateAsBaseI : AMI_TestIntf_unknownIntermediateAsBase
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Base b)
            {
                AllTests.test(b.b.Equals("UnknownIntermediate.b"));
                AllTests.test(b.GetType().Name.Equals("Base"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_knownIntermediateAsBaseI : AMI_TestIntf_knownIntermediateAsBase
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownIntermediate ki)
            {
                AllTests.test(ki.b.Equals("KnownIntermediate.b"));
                AllTests.test(ki.ki.Equals("KnownIntermediate.ki"));
                AllTests.test(ki.GetType().Name.Equals("KnownIntermediate"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_knownMostDerivedAsBaseI : AMI_TestIntf_knownMostDerivedAsBase
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownMostDerived kmd)
            {
                AllTests.test(kmd.b.Equals("KnownMostDerived.b"));
                AllTests.test(kmd.ki.Equals("KnownMostDerived.ki"));
                AllTests.test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                AllTests.test(kmd.GetType().Name.Equals("KnownMostDerived"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_knownIntermediateAsKnownIntermediateI : AMI_TestIntf_knownIntermediateAsKnownIntermediate
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownIntermediate ki)
            {
                AllTests.test(ki.b.Equals("KnownIntermediate.b"));
                AllTests.test(ki.ki.Equals("KnownIntermediate.ki"));
                AllTests.test(ki.GetType().Name.Equals("KnownIntermediate"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_knownMostDerivedAsKnownIntermediateI : AMI_TestIntf_knownMostDerivedAsKnownIntermediate
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownMostDerived kmd)
            {
                AllTests.test(kmd.b.Equals("KnownMostDerived.b"));
                AllTests.test(kmd.ki.Equals("KnownMostDerived.ki"));
                AllTests.test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                AllTests.test(kmd.GetType().Name.Equals("KnownMostDerived"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_knownMostDerivedAsKnownMostDerivedI : AMI_TestIntf_knownMostDerivedAsKnownMostDerived
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownMostDerived kmd)
            {
                AllTests.test(kmd.b.Equals("KnownMostDerived.b"));
                AllTests.test(kmd.ki.Equals("KnownMostDerived.ki"));
                AllTests.test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                AllTests.test(kmd.GetType().Name.Equals("KnownMostDerived"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_unknownMostDerived1AsBaseI : AMI_TestIntf_unknownMostDerived1AsBase
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownIntermediate ki)
            {
                AllTests.test(ki.b.Equals("UnknownMostDerived1.b"));
                AllTests.test(ki.ki.Equals("UnknownMostDerived1.ki"));
                AllTests.test(ki.GetType().Name.Equals("KnownIntermediate"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_unknownMostDerived1AsKnownIntermediateI : AMI_TestIntf_unknownMostDerived1AsKnownIntermediate
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownIntermediate ki)
            {
                AllTests.test(ki.b.Equals("UnknownMostDerived1.b"));
                AllTests.test(ki.ki.Equals("UnknownMostDerived1.ki"));
                AllTests.test(ki.GetType().Name.Equals("KnownIntermediate"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_unknownMostDerived2AsBaseI : AMI_TestIntf_unknownMostDerived2AsBase
    {
        public override void ice_response()
        {
            AllTests.test(false);
        }
        
        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Base b)
            {
                AllTests.test(b.b.Equals("UnknownMostDerived2.b"));
                AllTests.test(b.GetType().Name.Equals("Base"));
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        private Callback callback = new Callback();
    }
    
    public static TestIntfPrx allTests(Ice.Communicator communicator, bool collocated)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        String @ref = "Test:default -p 12010 -t 2000";
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestIntfPrx testPrx = TestIntfPrxHelper.checkedCast(@base);
        test(testPrx != null);
        test(testPrx.Equals(@base));
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("base... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.baseAsBase();
                test(false);
            }
            catch(Base b)
            {
                test(b.b.Equals("Base.b"));
                test(b.GetType().FullName.Equals("Test.Base"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
    
        Console.Out.Write("base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_baseAsBaseI cb = new AMI_Test_baseAsBaseI();
            testPrx.baseAsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of unknown derived... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.unknownDerivedAsBase();
                test(false);
            }
            catch(Base b)
            {
                test(b.b.Equals("UnknownDerived.b"));
                test(b.GetType().FullName.Equals("Test.Base"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of unknown derived (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_unknownDerivedAsBaseI cb = new AMI_Test_unknownDerivedAsBaseI();
            testPrx.unknownDerivedAsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("non-slicing of known derived as base... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.knownDerivedAsBase();
                test(false);
            }
            catch(KnownDerived k)
            {
                test(k.b.Equals("KnownDerived.b"));
                test(k.kd.Equals("KnownDerived.kd"));
                test(k.GetType().FullName.Equals("Test.KnownDerived"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("non-slicing of known derived as base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownDerivedAsBaseI cb = new AMI_Test_knownDerivedAsBaseI();
            testPrx.knownDerivedAsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("non-slicing of known derived as derived... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.knownDerivedAsKnownDerived();
                test(false);
            }
            catch(KnownDerived k)
            {
                test(k.b.Equals("KnownDerived.b"));
                test(k.kd.Equals("KnownDerived.kd"));
                test(k.GetType().FullName.Equals("Test.KnownDerived"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("non-slicing of known derived as derived (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownDerivedAsKnownDerivedI cb = new AMI_Test_knownDerivedAsKnownDerivedI();
            testPrx.knownDerivedAsKnownDerived_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of unknown intermediate as base... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.unknownIntermediateAsBase();
                test(false);
            }
            catch(Base b)
            {
                test(b.b.Equals("UnknownIntermediate.b"));
                test(b.GetType().FullName.Equals("Test.Base"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of unknown intermediate as base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_unknownIntermediateAsBaseI cb = new AMI_Test_unknownIntermediateAsBaseI();
            testPrx.unknownIntermediateAsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of known intermediate as base... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.knownIntermediateAsBase();
                test(false);
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.Equals("KnownIntermediate.b"));
                test(ki.ki.Equals("KnownIntermediate.ki"));
                test(ki.GetType().FullName.Equals("Test.KnownIntermediate"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of known intermediate as base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownIntermediateAsBaseI cb = new AMI_Test_knownIntermediateAsBaseI();
            testPrx.knownIntermediateAsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of known most derived as base... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.knownMostDerivedAsBase();
                test(false);
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.Equals("KnownMostDerived.b"));
                test(kmd.ki.Equals("KnownMostDerived.ki"));
                test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                test(kmd.GetType().FullName.Equals("Test.KnownMostDerived"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of known most derived as base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownMostDerivedAsBaseI cb = new AMI_Test_knownMostDerivedAsBaseI();
            testPrx.knownMostDerivedAsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("non-slicing of known intermediate as intermediate... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.knownIntermediateAsKnownIntermediate();
                test(false);
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.Equals("KnownIntermediate.b"));
                test(ki.ki.Equals("KnownIntermediate.ki"));
                test(ki.GetType().FullName.Equals("Test.KnownIntermediate"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("non-slicing of known intermediate as intermediate (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownIntermediateAsKnownIntermediateI cb = new AMI_Test_knownIntermediateAsKnownIntermediateI();
            testPrx.knownIntermediateAsKnownIntermediate_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("non-slicing of known most derived as intermediate... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.knownMostDerivedAsKnownIntermediate();
                test(false);
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.Equals("KnownMostDerived.b"));
                test(kmd.ki.Equals("KnownMostDerived.ki"));
                test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                test(kmd.GetType().FullName.Equals("Test.KnownMostDerived"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("non-slicing of known most derived as intermediate (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownMostDerivedAsKnownIntermediateI cb = new AMI_Test_knownMostDerivedAsKnownIntermediateI();
            testPrx.knownMostDerivedAsKnownIntermediate_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("non-slicing of known most derived as most derived... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.knownMostDerivedAsKnownMostDerived();
                test(false);
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.Equals("KnownMostDerived.b"));
                test(kmd.ki.Equals("KnownMostDerived.ki"));
                test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                test(kmd.GetType().FullName.Equals("Test.KnownMostDerived"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("non-slicing of known most derived as most derived (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownMostDerivedAsKnownMostDerivedI cb = new AMI_Test_knownMostDerivedAsKnownMostDerivedI();
            testPrx.knownMostDerivedAsKnownMostDerived_async(cb);
            AllTests.test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of unknown most derived, known intermediate as base... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.unknownMostDerived1AsBase();
                test(false);
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.Equals("UnknownMostDerived1.b"));
                test(ki.ki.Equals("UnknownMostDerived1.ki"));
                test(ki.GetType().FullName.Equals("Test.KnownIntermediate"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of unknown most derived, known intermediate as base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_unknownMostDerived1AsBaseI cb = new AMI_Test_unknownMostDerived1AsBaseI();
            testPrx.unknownMostDerived1AsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of unknown most derived, known intermediate as intermediate... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.unknownMostDerived1AsKnownIntermediate();
                test(false);
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.Equals("UnknownMostDerived1.b"));
                test(ki.ki.Equals("UnknownMostDerived1.ki"));
                test(ki.GetType().FullName.Equals("Test.KnownIntermediate"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of unknown most derived, known intermediate as intermediate (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_unknownMostDerived1AsKnownIntermediateI cb = new AMI_Test_unknownMostDerived1AsKnownIntermediateI();
            testPrx.unknownMostDerived1AsKnownIntermediate_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of unknown most derived, unknown intermediate thrown as base... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.unknownMostDerived2AsBase();
                test(false);
            }
            catch(Base b)
            {
                test(b.b.Equals("UnknownMostDerived2.b"));
                test(b.GetType().FullName.Equals("Test.Base"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("slicing of unknown most derived, unknown intermediate thrown as base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_unknownMostDerived2AsBaseI cb = new AMI_Test_unknownMostDerived2AsBaseI();
            testPrx.unknownMostDerived2AsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
        
        return testPrx;
    }
}
