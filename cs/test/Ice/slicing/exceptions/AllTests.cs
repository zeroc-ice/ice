// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;

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
        internal Callback(object obj)
        {
            _called = false;
	    _obj = obj;
        }
        
        public virtual bool check()
        {
            lock(this)
            {
                while(!_called)
                {
                    try
                    {
                        Monitor.Wait(_obj, TimeSpan.FromMilliseconds(5000));
                    }
                    catch(ThreadInterruptedException)
                    {
                        continue;
                    }
                    
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
                Monitor.Pulse(_obj);
            }
        }
        
        private bool _called;
	private object _obj;
    }
    
/*
    private class AMI_Test_baseAsBaseI:AMI_Test_baseAsBase
    {
        public AMI_Test_baseAsBaseI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(Base b)
            {
                AllTests.test(b.b.Equals("Base.b"));
                AllTests.test(b.ice_name().Equals("Base"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
    
    private class AMI_Test_unknownDerivedAsBaseI:AMI_Test_unknownDerivedAsBase
    {
        public AMI_Test_unknownDerivedAsBaseI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(Base b)
            {
                AllTests.test(b.b.Equals("UnknownDerived.b"));
                AllTests.test(b.ice_name().Equals("Base"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
    
    private class AMI_Test_knownDerivedAsBaseI:AMI_Test_knownDerivedAsBase
    {
        public AMI_Test_knownDerivedAsBaseI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownDerived k)
            {
                AllTests.test(k.b.Equals("KnownDerived.b"));
                AllTests.test(k.kd.Equals("KnownDerived.kd"));
                AllTests.test(k.ice_name().Equals("KnownDerived"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
    
    private class AMI_Test_knownDerivedAsKnownDerivedI:AMI_Test_knownDerivedAsKnownDerived
    {
        public AMI_Test_knownDerivedAsKnownDerivedI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownDerived k)
            {
                AllTests.test(k.b.Equals("KnownDerived.b"));
                AllTests.test(k.kd.Equals("KnownDerived.kd"));
                AllTests.test(k.ice_name().Equals("KnownDerived"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
    
    private class AMI_Test_unknownIntermediateAsBaseI:AMI_Test_unknownIntermediateAsBase
    {
        public AMI_Test_unknownIntermediateAsBaseI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(Base b)
            {
                AllTests.test(b.b.Equals("UnknownIntermediate.b"));
                AllTests.test(b.ice_name().Equals("Base"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
    
    private class AMI_Test_knownIntermediateAsBaseI:AMI_Test_knownIntermediateAsBase
    {
        public AMI_Test_knownIntermediateAsBaseI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownIntermediate ki)
            {
                AllTests.test(ki.b.Equals("KnownIntermediate.b"));
                AllTests.test(ki.ki.Equals("KnownIntermediate.ki"));
                AllTests.test(ki.ice_name().Equals("KnownIntermediate"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
    
    private class AMI_Test_knownMostDerivedAsBaseI:AMI_Test_knownMostDerivedAsBase
    {
        public AMI_Test_knownMostDerivedAsBaseI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
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
                AllTests.test(kmd.ice_name().Equals("KnownMostDerived"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
    
    private class AMI_Test_knownIntermediateAsKnownIntermediateI:AMI_Test_knownIntermediateAsKnownIntermediate
    {
        public AMI_Test_knownIntermediateAsKnownIntermediateI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownIntermediate ki)
            {
                AllTests.test(ki.b.Equals("KnownIntermediate.b"));
                AllTests.test(ki.ki.Equals("KnownIntermediate.ki"));
                AllTests.test(ki.ice_name().Equals("KnownIntermediate"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
    
    private class AMI_Test_knownMostDerivedAsKnownIntermediateI:AMI_Test_knownMostDerivedAsKnownIntermediate
    {
        public AMI_Test_knownMostDerivedAsKnownIntermediateI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
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
                AllTests.test(kmd.ice_name().Equals("KnownMostDerived"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
    
    private class AMI_Test_knownMostDerivedAsKnownMostDerivedI:AMI_Test_knownMostDerivedAsKnownMostDerived
    {
        public AMI_Test_knownMostDerivedAsKnownMostDerivedI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
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
                AllTests.test(kmd.ice_name().Equals("KnownMostDerived"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
    
    private class AMI_Test_unknownMostDerived1AsBaseI:AMI_Test_unknownMostDerived1AsBase
    {
        public AMI_Test_unknownMostDerived1AsBaseI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownIntermediate ki)
            {
                AllTests.test(ki.b.Equals("UnknownMostDerived1.b"));
                AllTests.test(ki.ki.Equals("UnknownMostDerived1.ki"));
                AllTests.test(ki.ice_name().Equals("KnownIntermediate"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
    
    private class AMI_Test_unknownMostDerived1AsKnownIntermediateI:AMI_Test_unknownMostDerived1AsKnownIntermediate
    {
        public AMI_Test_unknownMostDerived1AsKnownIntermediateI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownIntermediate ki)
            {
                AllTests.test(ki.b.Equals("UnknownMostDerived1.b"));
                AllTests.test(ki.ki.Equals("UnknownMostDerived1.ki"));
                AllTests.test(ki.ice_name().Equals("KnownIntermediate"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
    
    private class AMI_Test_unknownMostDerived2AsBaseI:AMI_Test_unknownMostDerived2AsBase
    {
        public AMI_Test_unknownMostDerived2AsBaseI()
        {
            InitBlock();
        }
        private void  InitBlock()
        {
            callback = new Callback();
        }
        public virtual void  ice_response()
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.LocalException exc)
        {
            AllTests.test(false);
        }
        
        public virtual void  ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(Base b)
            {
                AllTests.test(b.b.Equals("UnknownMostDerived2.b"));
                AllTests.test(b.ice_name().Equals("Base"));
            }
            catch(Exception ex)
            {
                AllTests.test(false);
            }
            callback.called();
        }
        
        public virtual bool check()
        {
            return callback.check();
        }
        
        //UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
        private Callback callback;
    }
*/
    
    public static TestPrx allTests(Ice.Communicator communicator, bool collocated)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        String ref_Renamed = "Test:default -p 12345 -t 2000";
        Ice.ObjectPrx base_Renamed = communicator.stringToProxy(ref_Renamed);
        test(base_Renamed != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestPrx testPrx = TestPrxHelper.checkedCast(base_Renamed);
        test(testPrx != null);
        test(testPrx.Equals(base_Renamed));
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("base... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.baseAsBase();
            }
            catch(Base b)
            {
                test(b.b.Equals("Base.b"));
                test(b.GetType().FullName.Equals("Base"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
    
	/*
        Console.Out.Write("base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_baseAsBaseI cb = new AMI_Test_baseAsBaseI();
            test.baseAsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        Console.Out.Write("slicing of unknown derived... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.unknownDerivedAsBase();
            }
            catch(Base b)
            {
                test(b.b.Equals("UnknownDerived.b"));
                test(b.GetType().FullName.Equals("Base"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
        
	/*
        Console.Out.Write("slicing of unknown derived (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_unknownDerivedAsBaseI cb = new AMI_Test_unknownDerivedAsBaseI();
            test.unknownDerivedAsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        Console.Out.Write("non-slicing of known derived as base... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.knownDerivedAsBase();
            }
            catch(KnownDerived k)
            {
                test(k.b.Equals("KnownDerived.b"));
                test(k.kd.Equals("KnownDerived.kd"));
                test(k.GetType().FullName.Equals("KnownDerived"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
        
	/*
        Console.Out.Write("non-slicing of known derived as base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownDerivedAsBaseI cb = new AMI_Test_knownDerivedAsBaseI();
            test.knownDerivedAsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        Console.Out.Write("non-slicing of known derived as derived... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.knownDerivedAsKnownDerived();
            }
            catch(KnownDerived k)
            {
                test(k.b.Equals("KnownDerived.b"));
                test(k.kd.Equals("KnownDerived.kd"));
                test(k.GetType().FullName.Equals("KnownDerived"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
        
	/*
        Console.Out.Write("non-slicing of known derived as derived (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownDerivedAsKnownDerivedI cb = new AMI_Test_knownDerivedAsKnownDerivedI();
            test.knownDerivedAsKnownDerived_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        Console.Out.Write("slicing of unknown intermediate as base... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.unknownIntermediateAsBase();
            }
            catch(Base b)
            {
                test(b.b.Equals("UnknownIntermediate.b"));
                test(b.GetType().FullName.Equals("Base"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
        
	/*
        Console.Out.Write("slicing of unknown intermediate as base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_unknownIntermediateAsBaseI cb = new AMI_Test_unknownIntermediateAsBaseI();
            test.unknownIntermediateAsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        Console.Out.Write("slicing of known intermediate as base... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.knownIntermediateAsBase();
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.Equals("KnownIntermediate.b"));
                test(ki.ki.Equals("KnownIntermediate.ki"));
                test(ki.GetType().FullName.Equals("KnownIntermediate"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
        
	/*
        Console.Out.Write("slicing of known intermediate as base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownIntermediateAsBaseI cb = new AMI_Test_knownIntermediateAsBaseI();
            test.knownIntermediateAsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        Console.Out.Write("slicing of known most derived as base... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.knownMostDerivedAsBase();
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.Equals("KnownMostDerived.b"));
                test(kmd.ki.Equals("KnownMostDerived.ki"));
                test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                test(kmd.GetType().FullName.Equals("KnownMostDerived"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
        
	/*
        Console.Out.Write("slicing of known most derived as base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownMostDerivedAsBaseI cb = new AMI_Test_knownMostDerivedAsBaseI();
            test.knownMostDerivedAsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        Console.Out.Write("non-slicing of known intermediate as intermediate... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.knownIntermediateAsKnownIntermediate();
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.Equals("KnownIntermediate.b"));
                test(ki.ki.Equals("KnownIntermediate.ki"));
                test(ki.GetType().FullName.Equals("KnownIntermediate"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
        
	/*
        Console.Out.Write("non-slicing of known intermediate as intermediate (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownIntermediateAsKnownIntermediateI cb = new AMI_Test_knownIntermediateAsKnownIntermediateI();
            test.knownIntermediateAsKnownIntermediate_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        Console.Out.Write("non-slicing of known most derived as intermediate... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.knownMostDerivedAsKnownIntermediate();
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.Equals("KnownMostDerived.b"));
                test(kmd.ki.Equals("KnownMostDerived.ki"));
                test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                test(kmd.GetType().FullName.Equals("KnownMostDerived"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
        
	/*
        Console.Out.Write("non-slicing of known most derived as intermediate (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownMostDerivedAsKnownIntermediateI cb = new AMI_Test_knownMostDerivedAsKnownIntermediateI();
            test.knownMostDerivedAsKnownIntermediate_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        Console.Out.Write("non-slicing of known most derived as most derived... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.knownMostDerivedAsKnownMostDerived();
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.Equals("KnownMostDerived.b"));
                test(kmd.ki.Equals("KnownMostDerived.ki"));
                test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                test(kmd.GetType().FullName.Equals("KnownMostDerived"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
        
	/*
        Console.Out.Write("non-slicing of known most derived as most derived (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_knownMostDerivedAsKnownMostDerivedI cb = new AMI_Test_knownMostDerivedAsKnownMostDerivedI();
            test.knownMostDerivedAsKnownMostDerived_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        Console.Out.Write("slicing of unknown most derived, known intermediate as base... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.unknownMostDerived1AsBase();
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.Equals("UnknownMostDerived1.b"));
                test(ki.ki.Equals("UnknownMostDerived1.ki"));
                test(ki.GetType().FullName.Equals("KnownIntermediate"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
        
	/*
        Console.Out.Write("slicing of unknown most derived, known intermediate as base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_unknownMostDerived1AsBaseI cb = new AMI_Test_unknownMostDerived1AsBaseI();
            test.unknownMostDerived1AsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        Console.Out.Write("slicing of unknown most derived, known intermediate as intermediate... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.unknownMostDerived1AsKnownIntermediate();
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.Equals("UnknownMostDerived1.b"));
                test(ki.ki.Equals("UnknownMostDerived1.ki"));
                test(ki.GetType().FullName.Equals("KnownIntermediate"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
        
	/*
        Console.Out.Write("slicing of unknown most derived, known intermediate as intermediate (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_unknownMostDerived1AsKnownIntermediateI cb = new AMI_Test_unknownMostDerived1AsKnownIntermediateI();
            test.unknownMostDerived1AsKnownIntermediate_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        Console.Out.Write("slicing of unknown most derived, unknown intermediate thrown as base... ");
        Console.Out.Flush();
        {
            bool gotException = false;
            try
            {
                testPrx.unknownMostDerived2AsBase();
            }
            catch(Base b)
            {
                test(b.b.Equals("UnknownMostDerived2.b"));
                test(b.GetType().FullName.Equals("Base"));
                gotException = true;
            }
            catch(Exception)
            {
                test(false);
            }
            test(gotException);
        }
        Console.Out.WriteLine("ok");
        
	/*
        Console.Out.Write("slicing of unknown most derived, unknown intermediate thrown as base (AMI)... ");
        Console.Out.Flush();
        {
            AMI_Test_unknownMostDerived2AsBaseI cb = new AMI_Test_unknownMostDerived2AsBaseI();
            test.unknownMostDerived2AsBase_async(cb);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");
	*/
        
        return testPrx;
    }
}
