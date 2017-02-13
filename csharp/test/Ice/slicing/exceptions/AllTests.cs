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
using Test;

public class AllTests : TestCommon.AllTests
{
    private class Callback
    {
        internal Callback()
        {
            _called = false;
        }

        public virtual void check()
        {
            lock(this)
            {
                while(!_called)
                {
                    Monitor.Wait(this);
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
                Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

    private class RelayI : RelayDisp_
    {
        public override void knownPreservedAsBase(Ice.Current current)
        {
            KnownPreservedDerived ex = new KnownPreservedDerived();
            ex.b = "base";
            ex.kp = "preserved";
            ex.kpd = "derived";
            throw ex;
        }

        public override void knownPreservedAsKnownPreserved(Ice.Current current)
        {
            KnownPreservedDerived ex = new KnownPreservedDerived();
            ex.b = "base";
            ex.kp = "preserved";
            ex.kpd = "derived";
            throw ex;
        }

        public override void unknownPreservedAsBase(Ice.Current current)
        {
            Preserved2 ex = new Preserved2();
            ex.b = "base";
            ex.kp = "preserved";
            ex.kpd = "derived";
            ex.p1 = new PreservedClass("bc", "pc");
            ex.p2 = ex.p1;
            throw ex;
        }

        public override void unknownPreservedAsKnownPreserved(Ice.Current current)
        {
            Preserved2 ex = new Preserved2();
            ex.b = "base";
            ex.kp = "preserved";
            ex.kpd = "derived";
            ex.p1 = new PreservedClass("bc", "pc");
            ex.p2 = ex.p1;
            throw ex;
        }
    }

    public static TestIntfPrx allTests(TestCommon.Application app, bool collocated)
    {
        Ice.Communicator communicator = app.communicator();
        Write("testing stringToProxy... ");
        Flush();
        String @ref = "Test:" + app.getTestEndpoint(0) + " -t 2000";
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        WriteLine("ok");

        Write("testing checked cast... ");
        Flush();
        TestIntfPrx testPrx = TestIntfPrxHelper.checkedCast(@base);
        test(testPrx != null);
        test(testPrx.Equals(@base));
        WriteLine("ok");

        Write("base... ");
        Flush();
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
        WriteLine("ok");

        Write("base (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_baseAsBase().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(Base b)
                    {
                        test(b.b.Equals("Base.b"));
                        test(b.GetType().Name.Equals("Base"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.baseAsBaseAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(Base b)
                {
                    test(b.b.Equals("Base.b"));
                    test(b.GetType().Name.Equals("Base"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("slicing of unknown derived... ");
        Flush();
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
        WriteLine("ok");

        Write("slicing of unknown derived (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_unknownDerivedAsBase().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(Base b)
                    {
                        test(b.b.Equals("UnknownDerived.b"));
                        test(b.GetType().Name.Equals("Base"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.unknownDerivedAsBaseAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(Base b)
                {
                    test(b.b.Equals("UnknownDerived.b"));
                    test(b.GetType().Name.Equals("Base"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("non-slicing of known derived as base... ");
        Flush();
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
        WriteLine("ok");

        Write("non-slicing of known derived as base (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_knownDerivedAsBase().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(KnownDerived k)
                    {
                        test(k.b.Equals("KnownDerived.b"));
                        test(k.kd.Equals("KnownDerived.kd"));
                        test(k.GetType().Name.Equals("KnownDerived"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.knownDerivedAsBaseAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(KnownDerived k)
                {
                    test(k.b.Equals("KnownDerived.b"));
                    test(k.kd.Equals("KnownDerived.kd"));
                    test(k.GetType().Name.Equals("KnownDerived"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("non-slicing of known derived as derived... ");
        Flush();
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
        WriteLine("ok");

        Write("non-slicing of known derived as derived (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_knownDerivedAsKnownDerived().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(KnownDerived k)
                    {
                        test(k.b.Equals("KnownDerived.b"));
                        test(k.kd.Equals("KnownDerived.kd"));
                        test(k.GetType().Name.Equals("KnownDerived"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.knownDerivedAsKnownDerivedAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(KnownDerived k)
                {
                    test(k.b.Equals("KnownDerived.b"));
                    test(k.kd.Equals("KnownDerived.kd"));
                    test(k.GetType().Name.Equals("KnownDerived"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("slicing of unknown intermediate as base... ");
        Flush();
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
        WriteLine("ok");

        Write("slicing of unknown intermediate as base (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_unknownIntermediateAsBase().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(Base b)
                    {
                        test(b.b.Equals("UnknownIntermediate.b"));
                        test(b.GetType().Name.Equals("Base"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.unknownIntermediateAsBaseAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(Base b)
                {
                    test(b.b.Equals("UnknownIntermediate.b"));
                    test(b.GetType().Name.Equals("Base"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("slicing of known intermediate as base... ");
        Flush();
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
        WriteLine("ok");

        Write("slicing of known intermediate as base (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_knownIntermediateAsBase().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(KnownIntermediate ki)
                    {
                        test(ki.b.Equals("KnownIntermediate.b"));
                        test(ki.ki.Equals("KnownIntermediate.ki"));
                        test(ki.GetType().Name.Equals("KnownIntermediate"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.knownIntermediateAsBaseAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(KnownIntermediate ki)
                {
                    test(ki.b.Equals("KnownIntermediate.b"));
                    test(ki.ki.Equals("KnownIntermediate.ki"));
                    test(ki.GetType().Name.Equals("KnownIntermediate"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("slicing of known most derived as base... ");
        Flush();
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
        WriteLine("ok");

        Write("slicing of known most derived as base (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_knownMostDerivedAsBase().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(KnownMostDerived kmd)
                    {
                        test(kmd.b.Equals("KnownMostDerived.b"));
                        test(kmd.ki.Equals("KnownMostDerived.ki"));
                        test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                        test(kmd.GetType().Name.Equals("KnownMostDerived"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.knownMostDerivedAsBaseAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(KnownMostDerived kmd)
                {
                    test(kmd.b.Equals("KnownMostDerived.b"));
                    test(kmd.ki.Equals("KnownMostDerived.ki"));
                    test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                    test(kmd.GetType().Name.Equals("KnownMostDerived"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("non-slicing of known intermediate as intermediate... ");
        Flush();
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
        WriteLine("ok");

        Write("non-slicing of known intermediate as intermediate (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_knownIntermediateAsKnownIntermediate().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(KnownIntermediate ki)
                    {
                        test(ki.b.Equals("KnownIntermediate.b"));
                        test(ki.ki.Equals("KnownIntermediate.ki"));
                        test(ki.GetType().Name.Equals("KnownIntermediate"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.knownIntermediateAsKnownIntermediateAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(KnownIntermediate ki)
                {
                    test(ki.b.Equals("KnownIntermediate.b"));
                    test(ki.ki.Equals("KnownIntermediate.ki"));
                    test(ki.GetType().Name.Equals("KnownIntermediate"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("non-slicing of known most derived as intermediate... ");
        Flush();
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
        WriteLine("ok");

        Write("non-slicing of known most derived as intermediate (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_knownMostDerivedAsKnownIntermediate().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(KnownMostDerived kmd)
                    {
                        test(kmd.b.Equals("KnownMostDerived.b"));
                        test(kmd.ki.Equals("KnownMostDerived.ki"));
                        test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                        test(kmd.GetType().Name.Equals("KnownMostDerived"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.knownMostDerivedAsKnownIntermediateAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(KnownMostDerived kmd)
                {
                    test(kmd.b.Equals("KnownMostDerived.b"));
                    test(kmd.ki.Equals("KnownMostDerived.ki"));
                    test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                    test(kmd.GetType().Name.Equals("KnownMostDerived"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("non-slicing of known most derived as most derived... ");
        Flush();
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
        WriteLine("ok");

        Write("non-slicing of known most derived as most derived (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_knownMostDerivedAsKnownMostDerived().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(KnownMostDerived kmd)
                    {
                        test(kmd.b.Equals("KnownMostDerived.b"));
                        test(kmd.ki.Equals("KnownMostDerived.ki"));
                        test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                        test(kmd.GetType().Name.Equals("KnownMostDerived"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.knownMostDerivedAsKnownMostDerivedAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(KnownMostDerived kmd)
                {
                    test(kmd.b.Equals("KnownMostDerived.b"));
                    test(kmd.ki.Equals("KnownMostDerived.ki"));
                    test(kmd.kmd.Equals("KnownMostDerived.kmd"));
                    test(kmd.GetType().Name.Equals("KnownMostDerived"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("slicing of unknown most derived, known intermediate as base... ");
        Flush();
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
        WriteLine("ok");

        Write("slicing of unknown most derived, known intermediate as base (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_unknownMostDerived1AsBase().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(KnownIntermediate ki)
                    {
                        test(ki.b.Equals("UnknownMostDerived1.b"));
                        test(ki.ki.Equals("UnknownMostDerived1.ki"));
                        test(ki.GetType().Name.Equals("KnownIntermediate"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.unknownMostDerived1AsBaseAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(KnownIntermediate ki)
                {
                    test(ki.b.Equals("UnknownMostDerived1.b"));
                    test(ki.ki.Equals("UnknownMostDerived1.ki"));
                    test(ki.GetType().Name.Equals("KnownIntermediate"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("slicing of unknown most derived, known intermediate as intermediate... ");
        Flush();
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
        WriteLine("ok");

        Write("slicing of unknown most derived, known intermediate as intermediate (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_unknownMostDerived1AsKnownIntermediate().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(KnownIntermediate ki)
                    {
                        test(ki.b.Equals("UnknownMostDerived1.b"));
                        test(ki.ki.Equals("UnknownMostDerived1.ki"));
                        test(ki.GetType().Name.Equals("KnownIntermediate"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.unknownMostDerived1AsKnownIntermediateAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(KnownIntermediate ki)
                {
                    test(ki.b.Equals("UnknownMostDerived1.b"));
                    test(ki.ki.Equals("UnknownMostDerived1.ki"));
                    test(ki.GetType().Name.Equals("KnownIntermediate"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("slicing of unknown most derived, unknown intermediate thrown as base... ");
        Flush();
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
        WriteLine("ok");

        Write("slicing of unknown most derived, unknown intermediate thrown as base (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_unknownMostDerived2AsBase().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        throw ex;
                    }
                    catch(Base b)
                    {
                        test(b.b.Equals("UnknownMostDerived2.b"));
                        test(b.GetType().Name.Equals("Base"));
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();

            try
            {
                testPrx.unknownMostDerived2AsBaseAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                try
                {
                    throw ae.InnerException;
                }
                catch(Base b)
                {
                    test(b.b.Equals("UnknownMostDerived2.b"));
                    test(b.GetType().Name.Equals("Base"));
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("unknown most derived in compact format... ");
        Flush();
        {
            try
            {
                testPrx.unknownMostDerived2AsBaseCompact();
                test(false);
            }
            catch(Base)
            {
                //
                // For the 1.0 encoding, the unknown exception is sliced to Base.
                //
                test(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0));
            }
            catch(Ice.UnknownUserException)
            {
                //
                // A MarshalException is raised for the compact format because the
                // most-derived type is unknown and the exception cannot be sliced.
                //
                test(!testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0));
            }
            catch(Ice.OperationNotExistException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("preserved exceptions... ");
        Flush();
        {
            Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("Relay", "default");
            RelayPrx relay = RelayPrxHelper.uncheckedCast(adapter.addWithUUID(new RelayI()));
            adapter.activate();

            try
            {
                testPrx.relayKnownPreservedAsBase(relay);
                test(false);
            }
            catch(KnownPreservedDerived ex)
            {
                test(ex.b.Equals("base"));
                test(ex.kp.Equals("preserved"));
                test(ex.kpd.Equals("derived"));
            }
            catch(Ice.OperationNotExistException)
            {
            }
            catch(Exception)
            {
                test(false);
            }

            try
            {
                testPrx.relayKnownPreservedAsKnownPreserved(relay);
                test(false);
            }
            catch(KnownPreservedDerived ex)
            {
                test(ex.b.Equals("base"));
                test(ex.kp.Equals("preserved"));
                test(ex.kpd.Equals("derived"));
            }
            catch(Ice.OperationNotExistException)
            {
            }
            catch(Exception)
            {
                test(false);
            }

            try
            {
                testPrx.relayUnknownPreservedAsBase(relay);
                test(false);
            }
            catch(Preserved2 ex)
            {
                test(ex.b.Equals("base"));
                test(ex.kp.Equals("preserved"));
                test(ex.kpd.Equals("derived"));
                test(ex.p1.ice_id().Equals(PreservedClass.ice_staticId()));
                PreservedClass pc = ex.p1 as PreservedClass;
                test(pc.bc.Equals("bc"));
                test(pc.pc.Equals("pc"));
                test(ex.p2 == ex.p1);
            }
            catch(KnownPreservedDerived ex)
            {
                //
                // For the 1.0 encoding, the unknown exception is sliced to KnownPreserved.
                //
                test(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0));
                test(ex.b.Equals("base"));
                test(ex.kp.Equals("preserved"));
                test(ex.kpd.Equals("derived"));
            }
            catch(Ice.OperationNotExistException)
            {
            }
            catch(Exception)
            {
                test(false);
            }

            try
            {
                testPrx.relayUnknownPreservedAsKnownPreserved(relay);
                test(false);
            }
            catch(Preserved2 ex)
            {
                test(ex.b.Equals("base"));
                test(ex.kp.Equals("preserved"));
                test(ex.kpd.Equals("derived"));
                test(ex.p1.ice_id().Equals(PreservedClass.ice_staticId()));
                PreservedClass pc = ex.p1 as PreservedClass;
                test(pc.bc.Equals("bc"));
                test(pc.pc.Equals("pc"));
                test(ex.p2 == ex.p1);
            }
            catch(KnownPreservedDerived ex)
            {
                //
                // For the 1.0 encoding, the unknown exception is sliced to KnownPreserved.
                //
                test(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0));
                test(ex.b.Equals("base"));
                test(ex.kp.Equals("preserved"));
                test(ex.kpd.Equals("derived"));
            }
            catch(Ice.OperationNotExistException)
            {
            }
            catch(Exception)
            {
                test(false);
            }

            adapter.destroy();
        }
        WriteLine("ok");
        return testPrx;
    }
}
