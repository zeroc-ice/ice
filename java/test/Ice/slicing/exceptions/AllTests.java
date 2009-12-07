// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.slicing.exceptions;

import test.Ice.slicing.exceptions.Test.Callback_TestIntf_baseAsBase;
import test.Ice.slicing.exceptions.Test.Callback_TestIntf_knownDerivedAsBase;
import test.Ice.slicing.exceptions.Test.Callback_TestIntf_knownDerivedAsKnownDerived;
import test.Ice.slicing.exceptions.Test.Callback_TestIntf_knownIntermediateAsBase;
import test.Ice.slicing.exceptions.Test.Callback_TestIntf_knownIntermediateAsKnownIntermediate;
import test.Ice.slicing.exceptions.Test.Callback_TestIntf_knownMostDerivedAsBase;
import test.Ice.slicing.exceptions.Test.Callback_TestIntf_knownMostDerivedAsKnownIntermediate;
import test.Ice.slicing.exceptions.Test.Callback_TestIntf_knownMostDerivedAsKnownMostDerived;
import test.Ice.slicing.exceptions.Test.Callback_TestIntf_unknownDerivedAsBase;
import test.Ice.slicing.exceptions.Test.Callback_TestIntf_unknownIntermediateAsBase;
import test.Ice.slicing.exceptions.Test.Callback_TestIntf_unknownMostDerived1AsBase;
import test.Ice.slicing.exceptions.Test.Callback_TestIntf_unknownMostDerived1AsKnownIntermediate;
import test.Ice.slicing.exceptions.Test.Callback_TestIntf_unknownMostDerived2AsBase;
import test.Ice.slicing.exceptions.Test.Base;
import test.Ice.slicing.exceptions.Test.KnownDerived;
import test.Ice.slicing.exceptions.Test.KnownIntermediate;
import test.Ice.slicing.exceptions.Test.KnownMostDerived;
import test.Ice.slicing.exceptions.Test.TestIntfPrx;
import test.Ice.slicing.exceptions.Test.TestIntfPrxHelper;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class Callback
    {
        Callback()
        {
            _called = false;
        }

        public synchronized void
        check()
        {
            while(!_called)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            _called = false;
        }
        
        public synchronized void
        called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static class Callback_TestIntf_baseAsBaseI extends Callback_TestIntf_baseAsBase
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(Base b)
            {
                test(b.b.equals("Base.b"));
                test(b.ice_name().equals("Test::Base"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_unknownDerivedAsBaseI extends Callback_TestIntf_unknownDerivedAsBase
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(Base b)
            {
                test(b.b.equals("UnknownDerived.b"));
                test(b.ice_name().equals("Test::Base"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_knownDerivedAsBaseI extends Callback_TestIntf_knownDerivedAsBase
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownDerived k)
            {
                test(k.b.equals("KnownDerived.b"));
                test(k.kd.equals("KnownDerived.kd"));
                test(k.ice_name().equals("Test::KnownDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_knownDerivedAsKnownDerivedI
        extends Callback_TestIntf_knownDerivedAsKnownDerived
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownDerived k)
            {
                test(k.b.equals("KnownDerived.b"));
                test(k.kd.equals("KnownDerived.kd"));
                test(k.ice_name().equals("Test::KnownDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_unknownIntermediateAsBaseI 
        extends Callback_TestIntf_unknownIntermediateAsBase
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(Base b)
            {
                test(b.b.equals("UnknownIntermediate.b"));
                test(b.ice_name().equals("Test::Base"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_knownIntermediateAsBaseI extends Callback_TestIntf_knownIntermediateAsBase
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.equals("KnownIntermediate.b"));
                test(ki.ki.equals("KnownIntermediate.ki"));
                test(ki.ice_name().equals("Test::KnownIntermediate"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_knownMostDerivedAsBaseI extends Callback_TestIntf_knownMostDerivedAsBase
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.equals("KnownMostDerived.b"));
                test(kmd.ki.equals("KnownMostDerived.ki"));
                test(kmd.kmd.equals("KnownMostDerived.kmd"));
                test(kmd.ice_name().equals("Test::KnownMostDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_knownIntermediateAsKnownIntermediateI
        extends Callback_TestIntf_knownIntermediateAsKnownIntermediate
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.equals("KnownIntermediate.b"));
                test(ki.ki.equals("KnownIntermediate.ki"));
                test(ki.ice_name().equals("Test::KnownIntermediate"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_knownMostDerivedAsKnownIntermediateI
        extends Callback_TestIntf_knownMostDerivedAsKnownIntermediate
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.equals("KnownMostDerived.b"));
                test(kmd.ki.equals("KnownMostDerived.ki"));
                test(kmd.kmd.equals("KnownMostDerived.kmd"));
                test(kmd.ice_name().equals("Test::KnownMostDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_knownMostDerivedAsKnownMostDerivedI
        extends Callback_TestIntf_knownMostDerivedAsKnownMostDerived
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.equals("KnownMostDerived.b"));
                test(kmd.ki.equals("KnownMostDerived.ki"));
                test(kmd.kmd.equals("KnownMostDerived.kmd"));
                test(kmd.ice_name().equals("Test::KnownMostDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_unknownMostDerived1AsBaseI 
        extends Callback_TestIntf_unknownMostDerived1AsBase
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.equals("UnknownMostDerived1.b"));
                test(ki.ki.equals("UnknownMostDerived1.ki"));
                test(ki.ice_name().equals("Test::KnownIntermediate"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_unknownMostDerived1AsKnownIntermediateI
        extends Callback_TestIntf_unknownMostDerived1AsKnownIntermediate
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.equals("UnknownMostDerived1.b"));
                test(ki.ki.equals("UnknownMostDerived1.ki"));
                test(ki.ice_name().equals("Test::KnownIntermediate"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_unknownMostDerived2AsBaseI 
        extends Callback_TestIntf_unknownMostDerived2AsBase
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(Base b)
            {
                test(b.b.equals("UnknownMostDerived2.b"));
                test(b.ice_name().equals("Test::Base"));
            }
            catch(Exception ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    public static TestIntfPrx
    allTests(Ice.Communicator communicator, boolean collocated, java.io.PrintWriter out)
    {
		out.print("testing stringToProxy... ");
        out.flush();
        String ref = "Test:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx test = TestIntfPrxHelper.checkedCast(base);
        test(test != null);
        test(test.equals(base));
        out.println("ok");

        out.print("base... ");
        out.flush();
        {
            try
            {
                test.baseAsBase();
                test(false);
            }
            catch(Base b)
            {
                test(b.b.equals("Base.b"));
                test(b.ice_name().equals("Test::Base"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("base (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_baseAsBaseI cb = new Callback_TestIntf_baseAsBaseI();
            test.begin_baseAsBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown derived... ");
        out.flush();
        {
            try
            {
                test.unknownDerivedAsBase();
                test(false);
            }
            catch(Base b)
            {
                test(b.b.equals("UnknownDerived.b"));
                test(b.ice_name().equals("Test::Base"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown derived (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_unknownDerivedAsBaseI cb = new Callback_TestIntf_unknownDerivedAsBaseI();
            test.begin_unknownDerivedAsBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known derived as base... ");
        out.flush();
        {
            try
            {
                test.knownDerivedAsBase();
                test(false);
            }
            catch(KnownDerived k)
            {
                test(k.b.equals("KnownDerived.b"));
                test(k.kd.equals("KnownDerived.kd"));
                test(k.ice_name().equals("Test::KnownDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known derived as base (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_knownDerivedAsBaseI cb = new Callback_TestIntf_knownDerivedAsBaseI();
            test.begin_knownDerivedAsBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known derived as derived... ");
        out.flush();
        {
            try
            {
                test.knownDerivedAsKnownDerived();
                test(false);
            }
            catch(KnownDerived k)
            {
                test(k.b.equals("KnownDerived.b"));
                test(k.kd.equals("KnownDerived.kd"));
                test(k.ice_name().equals("Test::KnownDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known derived as derived (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_knownDerivedAsKnownDerivedI cb = new Callback_TestIntf_knownDerivedAsKnownDerivedI();
            test.begin_knownDerivedAsKnownDerived(cb);
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown intermediate as base... ");
        out.flush();
        {
            try
            {
                test.unknownIntermediateAsBase();
                test(false);
            }
            catch(Base b)
            {
                test(b.b.equals("UnknownIntermediate.b"));
                test(b.ice_name().equals("Test::Base"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown intermediate as base (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_unknownIntermediateAsBaseI cb = new Callback_TestIntf_unknownIntermediateAsBaseI();
            test.begin_unknownIntermediateAsBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("slicing of known intermediate as base... ");
        out.flush();
        {
            try
            {
                test.knownIntermediateAsBase();
                test(false);
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.equals("KnownIntermediate.b"));
                test(ki.ki.equals("KnownIntermediate.ki"));
                test(ki.ice_name().equals("Test::KnownIntermediate"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of known intermediate as base (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_knownIntermediateAsBaseI cb = new Callback_TestIntf_knownIntermediateAsBaseI();
            test.begin_knownIntermediateAsBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("slicing of known most derived as base... ");
        out.flush();
        {
            try
            {
                test.knownMostDerivedAsBase();
                test(false);
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.equals("KnownMostDerived.b"));
                test(kmd.ki.equals("KnownMostDerived.ki"));
                test(kmd.kmd.equals("KnownMostDerived.kmd"));
                test(kmd.ice_name().equals("Test::KnownMostDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of known most derived as base (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_knownMostDerivedAsBaseI cb = new Callback_TestIntf_knownMostDerivedAsBaseI();
            test.begin_knownMostDerivedAsBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known intermediate as intermediate... ");
        out.flush();
        {
            try
            {
                test.knownIntermediateAsKnownIntermediate();
                test(false);
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.equals("KnownIntermediate.b"));
                test(ki.ki.equals("KnownIntermediate.ki"));
                test(ki.ice_name().equals("Test::KnownIntermediate"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known intermediate as intermediate (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_knownIntermediateAsKnownIntermediateI cb = 
                new Callback_TestIntf_knownIntermediateAsKnownIntermediateI();
            test.begin_knownIntermediateAsKnownIntermediate(cb);
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known most derived as intermediate... ");
        out.flush();
        {
            try
            {
                test.knownMostDerivedAsKnownIntermediate();
                test(false);
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.equals("KnownMostDerived.b"));
                test(kmd.ki.equals("KnownMostDerived.ki"));
                test(kmd.kmd.equals("KnownMostDerived.kmd"));
                test(kmd.ice_name().equals("Test::KnownMostDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known most derived as intermediate (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_knownMostDerivedAsKnownIntermediateI cb = 
                new Callback_TestIntf_knownMostDerivedAsKnownIntermediateI();
            test.begin_knownMostDerivedAsKnownIntermediate(cb);
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known most derived as most derived... ");
        out.flush();
        {
            try
            {
                test.knownMostDerivedAsKnownMostDerived();
                test(false);
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.equals("KnownMostDerived.b"));
                test(kmd.ki.equals("KnownMostDerived.ki"));
                test(kmd.kmd.equals("KnownMostDerived.kmd"));
                test(kmd.ice_name().equals("Test::KnownMostDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known most derived as most derived (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_knownMostDerivedAsKnownMostDerivedI cb = 
                new Callback_TestIntf_knownMostDerivedAsKnownMostDerivedI();
            test.begin_knownMostDerivedAsKnownMostDerived(cb);
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown most derived, known intermediate as base... ");
        out.flush();
        {
            try
            {
                test.unknownMostDerived1AsBase();
                test(false);
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.equals("UnknownMostDerived1.b"));
                test(ki.ki.equals("UnknownMostDerived1.ki"));
                test(ki.ice_name().equals("Test::KnownIntermediate"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown most derived, known intermediate as base (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_unknownMostDerived1AsBaseI cb = new Callback_TestIntf_unknownMostDerived1AsBaseI();
            test.begin_unknownMostDerived1AsBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown most derived, known intermediate as intermediate... ");
        out.flush();
        {
            try
            {
                test.unknownMostDerived1AsKnownIntermediate();
                test(false);
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.equals("UnknownMostDerived1.b"));
                test(ki.ki.equals("UnknownMostDerived1.ki"));
                test(ki.ice_name().equals("Test::KnownIntermediate"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown most derived, known intermediate as intermediate (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_unknownMostDerived1AsKnownIntermediateI cb =
                new Callback_TestIntf_unknownMostDerived1AsKnownIntermediateI();
            test.begin_unknownMostDerived1AsKnownIntermediate(cb);
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown most derived, unknown intermediate thrown as base... ");
        out.flush();
        {
            try
            {
                test.unknownMostDerived2AsBase();
                test(false);
            }
            catch(Base b)
            {
                test(b.b.equals("UnknownMostDerived2.b"));
                test(b.ice_name().equals("Test::Base"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown most derived, unknown intermediate thrown as base (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_unknownMostDerived2AsBaseI cb = new Callback_TestIntf_unknownMostDerived2AsBaseI();
            test.begin_unknownMostDerived2AsBase(cb);
            cb.check();
        }
        out.println("ok");

        return test;
    }
}
