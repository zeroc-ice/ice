// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using Test;

#if SILVERLIGHT
using System.Windows.Controls;
#endif

public class AllTests : TestCommon.TestApp
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

    private class PNodeI : PNode
    {
        public PNodeI()
        {
            ++counter;
        }

        internal static int counter = 0;
    }

    private class NodeFactoryI : Ice.ObjectFactory
    {
        public Ice.Object create(string id)
        {
            if(id.Equals(PNode.ice_staticId()))
            {
                return new PNodeI();
            }
            return null;
        }

        public void destroy()
        {
        }
    }

    private class PreservedI : Preserved
    {
        public PreservedI()
        {
            ++counter;
        }

        internal static int counter = 0;
    }

    private class PreservedFactoryI : Ice.ObjectFactory
    {
        public Ice.Object create(string id)
        {
            if(id.Equals(Preserved.ice_staticId()))
            {
                return new PreservedI();
            }
            return null;
        }

        public void destroy()
        {
        }
    }

#if SILVERLIGHT
    public override Ice.InitializationData initData()
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        initData.properties.setProperty("Ice.FactoryAssemblies", "objects,version=1.0.0.0");
        return initData;
    }

    override
    public void run(Ice.Communicator communicator)
#else
    public static TestIntfPrx allTests(Ice.Communicator communicator, bool collocated)
#endif
    {
        Write("testing stringToProxy... ");
        Flush();
        Ice.ObjectPrx basePrx = communicator.stringToProxy("Test:default -p 12010 -t 2000");
        test(basePrx != null);
        WriteLine("ok");

        Write("testing checked cast... ");
        Flush();
        TestIntfPrx testPrx = TestIntfPrxHelper.checkedCast(basePrx);
        test(testPrx != null);
        test(testPrx.Equals(basePrx));
        WriteLine("ok");

        Write("base as Object... ");
        Flush();
        {
            Ice.Object o;
            SBase sb = null;
            try
            {
                o = testPrx.SBaseAsObject();
                test(o != null);
                test(o.ice_id().Equals("::Test::SBase"));
                sb = (SBase) o;
            }
            catch(Exception)
            {
                test(false);
            }
            test(sb != null);
            test(sb.sb.Equals("SBase.sb"));
        }
        WriteLine("ok");

        Write("base as Object (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_SBaseAsObject().whenCompleted(
                (Ice.Object o) =>
                {
                    test(o != null);
                    test(o.ice_id().Equals("::Test::SBase"));
                    SBase sb = (SBase) o;
                    test(sb != null);
                    test(sb.sb.Equals("SBase.sb"));
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("base as base... ");
        Flush();
        {
            SBase sb;
            try
            {
                sb = testPrx.SBaseAsSBase();
                test(sb.sb.Equals("SBase.sb"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("base as base (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_SBaseAsSBase().whenCompleted(
                (SBase sb) =>
                {
                    test(sb.sb.Equals("SBase.sb"));
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("base with known derived as base... ");
        Flush();
        {
            SBase sb;
            SBSKnownDerived sbskd = null;
            try
            {
                sb = testPrx.SBSKnownDerivedAsSBase();
                test(sb.sb.Equals("SBSKnownDerived.sb"));
                sbskd = (SBSKnownDerived) sb;
            }
            catch(Exception)
            {
                test(false);
            }
            test(sbskd != null);
            test(sbskd.sbskd.Equals("SBSKnownDerived.sbskd"));
        }
        WriteLine("ok");

        Write("base with known derived as base (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_SBSKnownDerivedAsSBase().whenCompleted(
                (SBase sb) =>
                {
                    test(sb.sb.Equals("SBSKnownDerived.sb"));
                    SBSKnownDerived sbskd = (SBSKnownDerived) sb;
                    test(sbskd != null);
                    test(sbskd.sbskd.Equals("SBSKnownDerived.sbskd"));
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("base with known derived as known derived... ");
        Flush();
        {
            SBSKnownDerived sbskd;
            try
            {
                sbskd = testPrx.SBSKnownDerivedAsSBSKnownDerived();
                test(sbskd.sbskd.Equals("SBSKnownDerived.sbskd"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("base with known derived as known derived (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_SBSKnownDerivedAsSBSKnownDerived().whenCompleted(
                (SBSKnownDerived sbskd) =>
                {
                    test(sbskd.sbskd.Equals("SBSKnownDerived.sbskd"));
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("base with unknown derived as base... ");
        Flush();
        {
            SBase sb;
            try
            {
                sb = testPrx.SBSUnknownDerivedAsSBase();
                test(sb.sb.Equals("SBSUnknownDerived.sb"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        if(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0))
        {
            try
            {
                SBase sb = testPrx.SBSUnknownDerivedAsSBaseCompact();
                test(sb.sb.Equals("SBSUnknownDerived.sb"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        else
        {
            try
            {
                //
                // This test fails when using the compact format because the instance cannot
                // be sliced to a known type.
                //
                testPrx.SBSUnknownDerivedAsSBaseCompact();
                test(false);
            }
            catch(Ice.NoObjectFactoryException)
            {
                // Expected.
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("base with unknown derived as base (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_SBSUnknownDerivedAsSBase().whenCompleted(
                (SBase sb) =>
                {
                    test(sb.sb.Equals("SBSUnknownDerived.sb"));
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        if(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0))
        {
            //
            // This test succeeds for the 1.0 encoding.
            //
            Callback cb = new Callback();
            testPrx.begin_SBSUnknownDerivedAsSBaseCompact().whenCompleted(
                (SBase sb) =>
                {
                    test(sb.sb.Equals("SBSUnknownDerived.sb"));
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        else
        {
            //
            // This test fails when using the compact format because the instance cannot
            // be sliced to a known type.
            //
            Callback cb = new Callback();
            testPrx.begin_SBSUnknownDerivedAsSBaseCompact().whenCompleted(
                (SBase sb) =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    test(ex is Ice.NoObjectFactoryException);
                    cb.called();
                });
            cb.check();
        }
        WriteLine("ok");

        Write("unknown with Object as Object... ");
        Flush();
        {
            try
            {
                Ice.Object o = testPrx.SUnknownAsObject();
                test(!testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0));
                test(o is Ice.UnknownSlicedObject);
                test((o as Ice.UnknownSlicedObject).getUnknownTypeId().Equals("::Test::SUnknown"));
                testPrx.checkSUnknown(o);
            }
            catch(Ice.NoObjectFactoryException)
            {
                test(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("unknown with Object as Object (AMI)... ");
        Flush();
        {
            try
            {
                Callback cb = new Callback();
                if(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0))
                {
                    testPrx.begin_SUnknownAsObject().whenCompleted(
                        (Ice.Object o) =>
                        {
                            test(false);
                        },
                        (Ice.Exception ex) =>
                        {
                            test(ex.GetType().FullName.Equals("Ice.NoObjectFactoryException"));
                            cb.called();
                        });
                }
                else
                {
                    testPrx.begin_SUnknownAsObject().whenCompleted(
                        (Ice.Object o) =>
                        {
                            test(o is Ice.UnknownSlicedObject);
                            test((o as Ice.UnknownSlicedObject).getUnknownTypeId().Equals("::Test::SUnknown"));
                            cb.called();
                        },
                        (Ice.Exception ex) =>
                        {
                            test(false);
                        });
                }
                cb.check();
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("one-element cycle... ");
        Flush();
        {
            try
            {
                B b = testPrx.oneElementCycle();
                test(b != null);
                test(b.ice_id().Equals("::Test::B"));
                test(b.sb.Equals("B1.sb"));
                test(b.pb == b);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("one-element cycle (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_oneElementCycle().whenCompleted(
                (B b) =>
                {
                    test(b != null);
                    test(b.ice_id().Equals("::Test::B"));
                    test(b.sb.Equals("B1.sb"));
                    test(b.pb == b);
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("two-element cycle... ");
        Flush();
        {
            try
            {
                B b1 = testPrx.twoElementCycle();
                test(b1 != null);
                test(b1.ice_id().Equals("::Test::B"));
                test(b1.sb.Equals("B1.sb"));

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.ice_id().Equals("::Test::B"));
                test(b2.sb.Equals("B2.sb"));
                test(b2.pb == b1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("two-element cycle (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_twoElementCycle().whenCompleted(
                (B b1) =>
                {
                    test(b1 != null);
                    test(b1.ice_id().Equals("::Test::B"));
                    test(b1.sb.Equals("B1.sb"));

                    B b2 = b1.pb;
                    test(b2 != null);
                    test(b2.ice_id().Equals("::Test::B"));
                    test(b2.sb.Equals("B2.sb"));
                    test(b2.pb == b1);
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("known derived pointer slicing as base... ");
        Flush();
        {
            try
            {
                B b1;
                b1 = testPrx.D1AsB();
                test(b1 != null);
                test(b1.ice_id().Equals("::Test::D1"));
                test(b1.sb.Equals("D1.sb"));
                test(b1.pb != null);
                test(b1.pb != b1);
                D1 d1 = (D1) b1;
                test(d1 != null);
                test(d1.sd1.Equals("D1.sd1"));
                test(d1.pd1 != null);
                test(d1.pd1 != b1);
                test(b1.pb == d1.pd1);

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.pb == b1);
                test(b2.sb.Equals("D2.sb"));
                test(b2.ice_id().Equals("::Test::B"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("known derived pointer slicing as base (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_D1AsB().whenCompleted(
                (B b1) =>
                {
                    test(b1 != null);
                    test(b1.ice_id().Equals("::Test::D1"));
                    test(b1.sb.Equals("D1.sb"));
                    test(b1.pb != null);
                    test(b1.pb != b1);
                    D1 d1 = (D1) b1;
                    test(d1 != null);
                    test(d1.sd1.Equals("D1.sd1"));
                    test(d1.pd1 != null);
                    test(d1.pd1 != b1);
                    test(b1.pb == d1.pd1);

                    B b2 = b1.pb;
                    test(b2 != null);
                    test(b2.pb == b1);
                    test(b2.sb.Equals("D2.sb"));
                    test(b2.ice_id().Equals("::Test::B"));
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("known derived pointer slicing as derived... ");
        Flush();
        {
            try
            {
                D1 d1;
                d1 = testPrx.D1AsD1();
                test(d1 != null);
                test(d1.ice_id().Equals("::Test::D1"));
                test(d1.sb.Equals("D1.sb"));
                test(d1.pb != null);
                test(d1.pb != d1);

                B b2 = d1.pb;
                test(b2 != null);
                test(b2.ice_id().Equals("::Test::B"));
                test(b2.sb.Equals("D2.sb"));
                test(b2.pb == d1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("known derived pointer slicing as derived (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_D1AsD1().whenCompleted(
                (D1 d1) =>
                {
                    test(d1 != null);
                    test(d1.ice_id().Equals("::Test::D1"));
                    test(d1.sb.Equals("D1.sb"));
                    test(d1.pb != null);
                    test(d1.pb != d1);

                    B b2 = d1.pb;
                    test(b2 != null);
                    test(b2.ice_id().Equals("::Test::B"));
                    test(b2.sb.Equals("D2.sb"));
                    test(b2.pb == d1);
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("unknown derived pointer slicing as base... ");
        Flush();
        {
            try
            {
                B b2;
                b2 = testPrx.D2AsB();
                test(b2 != null);
                test(b2.ice_id().Equals("::Test::B"));
                test(b2.sb.Equals("D2.sb"));
                test(b2.pb != null);
                test(b2.pb != b2);

                B b1 = b2.pb;
                test(b1 != null);
                test(b1.ice_id().Equals("::Test::D1"));
                test(b1.sb.Equals("D1.sb"));
                test(b1.pb == b2);
                D1 d1 = (D1) b1;
                test(d1 != null);
                test(d1.sd1.Equals("D1.sd1"));
                test(d1.pd1 == b2);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("unknown derived pointer slicing as base (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_D2AsB().whenCompleted(
                (B b2) =>
                {
                    test(b2 != null);
                    test(b2.ice_id().Equals("::Test::B"));
                    test(b2.sb.Equals("D2.sb"));
                    test(b2.pb != null);
                    test(b2.pb != b2);

                    B b1 = b2.pb;
                    test(b1 != null);
                    test(b1.ice_id().Equals("::Test::D1"));
                    test(b1.sb.Equals("D1.sb"));
                    test(b1.pb == b2);
                    D1 d1 = (D1) b1;
                    test(d1 != null);
                    test(d1.sd1.Equals("D1.sd1"));
                    test(d1.pd1 == b2);
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("param ptr slicing with known first... ");
        Flush();
        {
            try
            {
                B b1;
                B b2;
                testPrx.paramTest1(out b1, out b2);

                test(b1 != null);
                test(b1.ice_id().Equals("::Test::D1"));
                test(b1.sb.Equals("D1.sb"));
                test(b1.pb == b2);
                D1 d1 = (D1) b1;
                test(d1 != null);
                test(d1.sd1.Equals("D1.sd1"));
                test(d1.pd1 == b2);

                test(b2 != null);
                test(b2.ice_id().Equals("::Test::B")); // No factory, must be sliced
                test(b2.sb.Equals("D2.sb"));
                test(b2.pb == b1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("param ptr slicing with known first (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_paramTest1().whenCompleted(
                (B b1, B b2) =>
                {
                    test(b1 != null);
                    test(b1.ice_id().Equals("::Test::D1"));
                    test(b1.sb.Equals("D1.sb"));
                    test(b1.pb == b2);
                    D1 d1 = (D1) b1;
                    test(d1 != null);
                    test(d1.sd1.Equals("D1.sd1"));
                    test(d1.pd1 == b2);

                    test(b2 != null);
                    test(b2.ice_id().Equals("::Test::B")); // No factory, must be sliced
                    test(b2.sb.Equals("D2.sb"));
                    test(b2.pb == b1);
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("param ptr slicing with unknown first... ");
        Flush();
        {
            try
            {
                B b2;
                B b1;
                testPrx.paramTest2(out b2, out b1);

                test(b1 != null);
                test(b1.ice_id().Equals("::Test::D1"));
                test(b1.sb.Equals("D1.sb"));
                test(b1.pb == b2);
                D1 d1 = (D1) b1;
                test(d1 != null);
                test(d1.sd1.Equals("D1.sd1"));
                test(d1.pd1 == b2);

                test(b2 != null);
                test(b2.ice_id().Equals("::Test::B")); // No factory, must be sliced
                test(b2.sb.Equals("D2.sb"));
                test(b2.pb == b1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("param ptr slicing with unknown first (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_paramTest2().whenCompleted(
                (B b2, B b1) =>
                {
                    test(b1 != null);
                    test(b1.ice_id().Equals("::Test::D1"));
                    test(b1.sb.Equals("D1.sb"));
                    test(b1.pb == b2);
                    D1 d1 = (D1) b1;
                    test(d1 != null);
                    test(d1.sd1.Equals("D1.sd1"));
                    test(d1.pd1 == b2);

                    test(b2 != null);
                    test(b2.ice_id().Equals("::Test::B")); // No factory, must be sliced
                    test(b2.sb.Equals("D2.sb"));
                    test(b2.pb == b1);
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("return value identity with known first... ");
        Flush();
        {
            try
            {
                B p1;
                B p2;
                B ret = testPrx.returnTest1(out p1, out p2);
                test(ret == p1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("return value identity with known first (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_returnTest1().whenCompleted(
                (B r, B p1, B p2) =>
                {
                    test(r == p1);
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("return value identity with unknown first... ");
        Flush();
        {
            try
            {
                B p1;
                B p2;
                B ret = testPrx.returnTest2(out p1, out p2);
                test(ret == p1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("return value identity with unknown first (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_returnTest2().whenCompleted(
                (B r, B p1, B p2) =>
                {
                    test(r == p1);
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("return value identity for input params known first... ");
        Flush();
        {
            try
            {
                D1 d1 = new D1();
                d1.sb = "D1.sb";
                d1.sd1 = "D1.sd1";
                D3 d3 = new D3();
                d3.pb = d1;
                d3.sb = "D3.sb";
                d3.sd3 = "D3.sd3";
                d3.pd3 = d1;
                d1.pb = d3;
                d1.pd1 = d3;

                B b1 = testPrx.returnTest3(d1, d3);

                test(b1 != null);
                test(b1.sb.Equals("D1.sb"));
                test(b1.ice_id().Equals("::Test::D1"));
                D1 p1 = (D1) b1;
                test(p1 != null);
                test(p1.sd1.Equals("D1.sd1"));
                test(p1.pd1 == b1.pb);

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.sb.Equals("D3.sb"));
                test(b2.ice_id().Equals("::Test::B")); // Sliced by server
                test(b2.pb == b1);
                try
                {
                    D3 p3 = (D3) b2;
                    test(false);
                    D3 tmp = p3; p3 = tmp; // Stop compiler warning about unused variable.
                }
                catch(InvalidCastException)
                {
                }

                test(b1 != d1);
                test(b1 != d3);
                test(b2 != d1);
                test(b2 != d3);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("return value identity for input params known first (AMI)... ");
        Flush();
        {
            D1 d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            D3 d3 = new D3();
            d3.pb = d1;
            d3.sb = "D3.sb";
            d3.sd3 = "D3.sd3";
            d3.pd3 = d1;
            d1.pb = d3;
            d1.pd1 = d3;

            B b1 = null;
            Callback cb = new Callback();
            testPrx.begin_returnTest3(d1, d3).whenCompleted(
                (B b) =>
                {
                    b1 = b;
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();

            test(b1 != null);
            test(b1.sb.Equals("D1.sb"));
            test(b1.ice_id().Equals("::Test::D1"));
            D1 p1 = (D1) b1;
            test(p1 != null);
            test(p1.sd1.Equals("D1.sd1"));
            test(p1.pd1 == b1.pb);

            B b2 = b1.pb;
            test(b2 != null);
            test(b2.sb.Equals("D3.sb"));
            test(b2.ice_id().Equals("::Test::B")); // Sliced by server
            test(b2.pb == b1);
            try
            {
                D3 p3 = (D3) b2;
                test(false);
                D3 tmp = p3; p3 = tmp; // Stop compiler warning about unused variable.
            }
            catch(InvalidCastException)
            {
            }

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        WriteLine("ok");

        Write("return value identity for input params unknown first... ");
        Flush();
        {
            try
            {
                D1 d1 = new D1();
                d1.sb = "D1.sb";
                d1.sd1 = "D1.sd1";
                D3 d3 = new D3();
                d3.pb = d1;
                d3.sb = "D3.sb";
                d3.sd3 = "D3.sd3";
                d3.pd3 = d1;
                d1.pb = d3;
                d1.pd1 = d3;

                B b1 = testPrx.returnTest3(d3, d1);

                test(b1 != null);
                test(b1.sb.Equals("D3.sb"));
                test(b1.ice_id().Equals("::Test::B")); // Sliced by server

                try
                {
                    D3 p1 = (D3) b1;
                    test(false);
                    D3 tmp = p1; p1 = tmp; // Stop compiler warning about unused variable.
                }
                catch(InvalidCastException)
                {
                }

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.sb.Equals("D1.sb"));
                test(b2.ice_id().Equals("::Test::D1"));
                test(b2.pb == b1);
                D1 p3 = (D1) b2;
                test(p3 != null);
                test(p3.sd1.Equals("D1.sd1"));
                test(p3.pd1 == b1);

                test(b1 != d1);
                test(b1 != d3);
                test(b2 != d1);
                test(b2 != d3);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("return value identity for input params unknown first (AMI)... ");
        Flush();
        {
            D1 d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            D3 d3 = new D3();
            d3.pb = d1;
            d3.sb = "D3.sb";
            d3.sd3 = "D3.sd3";
            d3.pd3 = d1;
            d1.pb = d3;
            d1.pd1 = d3;

            B b1 = null;
            Callback cb = new Callback();
            testPrx.begin_returnTest3(d3, d1).whenCompleted(
                (B b) =>
                {
                    b1 = b;
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();

            test(b1 != null);
            test(b1.sb.Equals("D3.sb"));
            test(b1.ice_id().Equals("::Test::B")); // Sliced by server

            try
            {
                D3 p1 = (D3) b1;
                test(false);
                D3 tmp = p1; p1 = tmp; // Stop compiler warning about unused variable.
            }
            catch(InvalidCastException)
            {
            }

            B b2 = b1.pb;
            test(b2 != null);
            test(b2.sb.Equals("D1.sb"));
            test(b2.ice_id().Equals("::Test::D1"));
            test(b2.pb == b1);
            D1 p3 = (D1) b2;
            test(p3 != null);
            test(p3.sd1.Equals("D1.sd1"));
            test(p3.pd1 == b1);

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        WriteLine("ok");

        Write("remainder unmarshaling (3 instances)... ");
        Flush();
        {
            try
            {
                B p1;
                B p2;
                B ret = testPrx.paramTest3(out p1, out p2);

                test(p1 != null);
                test(p1.sb.Equals("D2.sb (p1 1)"));
                test(p1.pb == null);
                test(p1.ice_id().Equals("::Test::B"));

                test(p2 != null);
                test(p2.sb.Equals("D2.sb (p2 1)"));
                test(p2.pb == null);
                test(p2.ice_id().Equals("::Test::B"));

                test(ret != null);
                test(ret.sb.Equals("D1.sb (p2 2)"));
                test(ret.pb == null);
                test(ret.ice_id().Equals("::Test::D1"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("remainder unmarshaling (3 instances) (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_paramTest3().whenCompleted(
                (B ret, B p1, B p2) =>
                {
                    test(p1 != null);
                    test(p1.sb.Equals("D2.sb (p1 1)"));
                    test(p1.pb == null);
                    test(p1.ice_id().Equals("::Test::B"));

                    test(p2 != null);
                    test(p2.sb.Equals("D2.sb (p2 1)"));
                    test(p2.pb == null);
                    test(p2.ice_id().Equals("::Test::B"));

                    test(ret != null);
                    test(ret.sb.Equals("D1.sb (p2 2)"));
                    test(ret.pb == null);
                    test(ret.ice_id().Equals("::Test::D1"));
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("remainder unmarshaling (4 instances)... ");
        Flush();
        {
            try
            {
                B b;
                B ret = testPrx.paramTest4(out b);

                test(b != null);
                test(b.sb.Equals("D4.sb (1)"));
                test(b.pb == null);
                test(b.ice_id().Equals("::Test::B"));

                test(ret != null);
                test(ret.sb.Equals("B.sb (2)"));
                test(ret.pb == null);
                test(ret.ice_id().Equals("::Test::B"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("remainder unmarshaling (4 instances) (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_paramTest4().whenCompleted(
                (B ret, B b) =>
                {
                    test(b != null);
                    test(b.sb.Equals("D4.sb (1)"));
                    test(b.pb == null);
                    test(b.ice_id().Equals("::Test::B"));

                    test(ret != null);
                    test(ret.sb.Equals("B.sb (2)"));
                    test(ret.pb == null);
                    test(ret.ice_id().Equals("::Test::B"));
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("param ptr slicing, instance marshaled in unknown derived as base... ");
        Flush();
        {
            try
            {
                B b1 = new B();
                b1.sb = "B.sb(1)";
                b1.pb = b1;

                D3 d3 = new D3();
                d3.sb = "D3.sb";
                d3.pb = d3;
                d3.sd3 = "D3.sd3";
                d3.pd3 = b1;

                B b2 = new B();
                b2.sb = "B.sb(2)";
                b2.pb = b1;

                B ret = testPrx.returnTest3(d3, b2);

                test(ret != null);
                test(ret.ice_id().Equals("::Test::B"));
                test(ret.sb.Equals("D3.sb"));
                test(ret.pb == ret);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("param ptr slicing, instance marshaled in unknown derived as base (AMI)... ");
        Flush();
        {
            B b1 = new B();
            b1.sb = "B.sb(1)";
            b1.pb = b1;

            D3 d3 = new D3();
            d3.sb = "D3.sb";
            d3.pb = d3;
            d3.sd3 = "D3.sd3";
            d3.pd3 = b1;

            B b2 = new B();
            b2.sb = "B.sb(2)";
            b2.pb = b1;

            B rv = null;
            Callback cb = new Callback();
            testPrx.begin_returnTest3(d3, b2).whenCompleted(
                (B b) =>
                {
                    rv = b;                    
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();

            test(rv != null);
            test(rv.ice_id().Equals("::Test::B"));
            test(rv.sb.Equals("D3.sb"));
            test(rv.pb == rv);
        }
        WriteLine("ok");

        Write("param ptr slicing, instance marshaled in unknown derived as derived... ");
        Flush();
        {
            try
            {
                D1 d11 = new D1();
                d11.sb = "D1.sb(1)";
                d11.pb = d11;
                d11.sd1 = "D1.sd1(1)";

                D3 d3 = new D3();
                d3.sb = "D3.sb";
                d3.pb = d3;
                d3.sd3 = "D3.sd3";
                d3.pd3 = d11;

                D1 d12 = new D1();
                d12.sb = "D1.sb(2)";
                d12.pb = d12;
                d12.sd1 = "D1.sd1(2)";
                d12.pd1 = d11;

                B ret = testPrx.returnTest3(d3, d12);
                test(ret != null);
                test(ret.ice_id().Equals("::Test::B"));
                test(ret.sb.Equals("D3.sb"));
                test(ret.pb == ret);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("param ptr slicing, instance marshaled in unknown derived as derived (AMI)... ");
        Flush();
        {
            D1 d11 = new D1();
            d11.sb = "D1.sb(1)";
            d11.pb = d11;
            d11.sd1 = "D1.sd1(1)";

            D3 d3 = new D3();
            d3.sb = "D3.sb";
            d3.pb = d3;
            d3.sd3 = "D3.sd3";
            d3.pd3 = d11;

            D1 d12 = new D1();
            d12.sb = "D1.sb(2)";
            d12.pb = d12;
            d12.sd1 = "D1.sd1(2)";
            d12.pd1 = d11;

            B rv = null;
            Callback cb = new Callback();
            testPrx.begin_returnTest3(d3, d12).whenCompleted(
                (B b) =>
                {
                    rv = b;                    
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();

            test(rv != null);
            test(rv.ice_id().Equals("::Test::B"));
            test(rv.sb.Equals("D3.sb"));
            test(rv.pb == rv);
        }
        WriteLine("ok");

        Write("sequence slicing... ");
        Flush();
        {
            try
            {
                SS3 ss;
                {
                    B ss1b = new B();
                    ss1b.sb = "B.sb";
                    ss1b.pb = ss1b;

                    D1 ss1d1 = new D1();
                    ss1d1.sb = "D1.sb";
                    ss1d1.sd1 = "D1.sd1";
                    ss1d1.pb = ss1b;

                    D3 ss1d3 = new D3();
                    ss1d3.sb = "D3.sb";
                    ss1d3.sd3 = "D3.sd3";
                    ss1d3.pb = ss1b;

                    B ss2b = new B();
                    ss2b.sb = "B.sb";
                    ss2b.pb = ss1b;

                    D1 ss2d1 = new D1();
                    ss2d1.sb = "D1.sb";
                    ss2d1.sd1 = "D1.sd1";
                    ss2d1.pb = ss2b;

                    D3 ss2d3 = new D3();
                    ss2d3.sb = "D3.sb";
                    ss2d3.sd3 = "D3.sd3";
                    ss2d3.pb = ss2b;

                    ss1d1.pd1 = ss2b;
                    ss1d3.pd3 = ss2d1;

                    ss2d1.pd1 = ss1d3;
                    ss2d3.pd3 = ss1d1;

                    SS1 ss1 = new SS1();
                    ss1.s = new BSeq(3);
                    ss1.s.Add(ss1b);
                    ss1.s.Add(ss1d1);
                    ss1.s.Add(ss1d3);

                    SS2 ss2 = new SS2();
                    ss2.s = new BSeq(3);
                    ss2.s.Add(ss2b);
                    ss2.s.Add(ss2d1);
                    ss2.s.Add(ss2d3);

                    ss = testPrx.sequenceTest(ss1, ss2);
                }

                test(ss.c1 != null);
                B ss1b2 = ss.c1.s[0];
                B ss1d2 = ss.c1.s[1];
                test(ss.c2 != null);
                B ss1d4 = ss.c1.s[2];

                test(ss.c2 != null);
                B ss2b2 = ss.c2.s[0];
                B ss2d2 = ss.c2.s[1];
                B ss2d4 = ss.c2.s[2];

                test(ss1b2.pb == ss1b2);
                test(ss1d2.pb == ss1b2);
                test(ss1d4.pb == ss1b2);

                test(ss2b2.pb == ss1b2);
                test(ss2d2.pb == ss2b2);
                test(ss2d4.pb == ss2b2);

                test(ss1b2.ice_id().Equals("::Test::B"));
                test(ss1d2.ice_id().Equals("::Test::D1"));
                test(ss1d4.ice_id().Equals("::Test::B"));

                test(ss2b2.ice_id().Equals("::Test::B"));
                test(ss2d2.ice_id().Equals("::Test::D1"));
                test(ss2d4.ice_id().Equals("::Test::B"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("sequence slicing (AMI)... ");
        Flush();
        {
            SS3 ss = null;
            {
                B ss1b = new B();
                ss1b.sb = "B.sb";
                ss1b.pb = ss1b;

                D1 ss1d1 = new D1();
                ss1d1.sb = "D1.sb";
                ss1d1.sd1 = "D1.sd1";
                ss1d1.pb = ss1b;

                D3 ss1d3 = new D3();
                ss1d3.sb = "D3.sb";
                ss1d3.sd3 = "D3.sd3";
                ss1d3.pb = ss1b;

                B ss2b = new B();
                ss2b.sb = "B.sb";
                ss2b.pb = ss1b;

                D1 ss2d1 = new D1();
                ss2d1.sb = "D1.sb";
                ss2d1.sd1 = "D1.sd1";
                ss2d1.pb = ss2b;

                D3 ss2d3 = new D3();
                ss2d3.sb = "D3.sb";
                ss2d3.sd3 = "D3.sd3";
                ss2d3.pb = ss2b;

                ss1d1.pd1 = ss2b;
                ss1d3.pd3 = ss2d1;

                ss2d1.pd1 = ss1d3;
                ss2d3.pd3 = ss1d1;

                SS1 ss1 = new SS1();
                ss1.s = new BSeq();
                ss1.s.Add(ss1b);
                ss1.s.Add(ss1d1);
                ss1.s.Add(ss1d3);

                SS2 ss2 = new SS2();
                ss2.s = new BSeq();
                ss2.s.Add(ss2b);
                ss2.s.Add(ss2d1);
                ss2.s.Add(ss2d3);

                Callback cb = new Callback();
                testPrx.begin_sequenceTest(ss1, ss2).whenCompleted(
                    (SS3 s) =>
                    {
                        ss = s;                   
                        cb.called();
                    },
                    (Ice.Exception ex) =>
                    {
                        test(false);
                    });
                cb.check();
            }
            test(ss.c1 != null);
            B ss1b3 = ss.c1.s[0];
            B ss1d5 = ss.c1.s[1];
            test(ss.c2 != null);
            B ss1d6 = ss.c1.s[2];

            test(ss.c2 != null);
            B ss2b3 = ss.c2.s[0];
            B ss2d5 = ss.c2.s[1];
            B ss2d6 = ss.c2.s[2];

            test(ss1b3.pb == ss1b3);
            test(ss1d6.pb == ss1b3);
            test(ss1d6.pb == ss1b3);

            test(ss2b3.pb == ss1b3);
            test(ss2d6.pb == ss2b3);
            test(ss2d6.pb == ss2b3);

            test(ss1b3.ice_id().Equals("::Test::B"));
            test(ss1d5.ice_id().Equals("::Test::D1"));
            test(ss1d6.ice_id().Equals("::Test::B"));

            test(ss2b3.ice_id().Equals("::Test::B"));
            test(ss2d5.ice_id().Equals("::Test::D1"));
            test(ss2d6.ice_id().Equals("::Test::B"));
        }
        WriteLine("ok");

        Write("dictionary slicing... ");
        Flush();
        {
            try
            {
                Dictionary<int, B> bin = new Dictionary<int, B>();
                Dictionary<int, B> bout;
                Dictionary<int, B> ret;
                int i;
                for(i = 0; i < 10; ++i)
                {
                    string s = "D1." + i.ToString();
                    D1 d1 = new D1();
                    d1.sb = s;
                    d1.pb = d1;
                    d1.sd1 = s;
                    bin[i] = d1;
                }

                ret = testPrx.dictionaryTest(bin, out bout);

                test(bout.Count == 10);
                for(i = 0; i < 10; ++i)
                {
                    B b = bout[i * 10];
                    test(b != null);
                    string s = "D1." + i.ToString();
                    test(b.sb.Equals(s));
                    test(b.pb != null);
                    test(b.pb != b);
                    test(b.pb.sb.Equals(s));
                    test(b.pb.pb == b.pb);
                }

                test(ret.Count == 10);
                for(i = 0; i < 10; ++i)
                {
                    B b = ret[i * 20];
                    test(b != null);
                    string s = "D1." + (i * 20).ToString();
                    test(b.sb.Equals(s));
                    test(b.pb == (i == 0 ? (B)null : ret[(i - 1) * 20]));
                    D1 d1 = (D1) b;
                    test(d1 != null);
                    test(d1.sd1.Equals(s));
                    test(d1.pd1 == d1);
                }
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("dictionary slicing (AMI)... ");
        Flush();
        {
            Dictionary<int, B> bin = new Dictionary<int, B>();
            Dictionary<int, B> bout = null;
            Dictionary<int, B> rv = null;
            int i;
            for(i = 0; i < 10; ++i)
            {
                string s = "D1." + i.ToString();
                D1 d1 = new D1();
                d1.sb = s;
                d1.pb = d1;
                d1.sd1 = s;
                bin[i] = d1;
            }

            Callback cb = new Callback();
            testPrx.begin_dictionaryTest(bin).whenCompleted(
                (Dictionary<int, B> r, Dictionary<int, B> b) =>
                {
                    rv = (Dictionary<int, B>)r;
                    bout = (Dictionary<int, B>)b;
                    cb.called();              
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();

            test(bout.Count == 10);
            for(i = 0; i < 10; ++i)
            {
                B b = bout[i * 10];
                test(b != null);
                string s = "D1." + i.ToString();
                test(b.sb.Equals(s));
                test(b.pb != null);
                test(b.pb != b);
                test(b.pb.sb.Equals(s));
                test(b.pb.pb == b.pb);
            }

            test(rv.Count == 10);
            for(i = 0; i < 10; ++i)
            {
                B b = rv[i * 20];
                test(b != null);
                string s = "D1." + (i * 20).ToString();
                test(b.sb.Equals(s));
                test(b.pb == (i == 0 ? (B)null : rv[(i - 1) * 20]));
                D1 d1 = (D1) b;
                test(d1 != null);
                test(d1.sd1.Equals(s));
                test(d1.pd1 == d1);
            }
        }
        WriteLine("ok");

        Write("base exception thrown as base exception... ");
        Flush();
        {
            try
            {
                testPrx.throwBaseAsBase();
                test(false);
            }
            catch(BaseException e)
            {
                test(e.GetType().FullName.Equals("Test.BaseException"));
                test(e.sbe.Equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.Equals("sb"));
                test(e.pb.pb == e.pb);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("base exception thrown as base exception (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_throwBaseAsBase().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                     try
                    {
                        BaseException e = (BaseException)ex;
                        test(e.sbe.Equals("sbe"));
                        test(e.pb != null);
                        test(e.pb.sb.Equals("sb"));
                        test(e.pb.pb == e.pb);
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();
        }
        WriteLine("ok");

        Write("derived exception thrown as base exception... ");
        Flush();
        {
            try
            {
                testPrx.throwDerivedAsBase();
                test(false);
            }
            catch(DerivedException e)
            {
                test(e.GetType().FullName.Equals("Test.DerivedException"));
                test(e.sbe.Equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.Equals("sb1"));
                test(e.pb.pb == e.pb);
                test(e.sde.Equals("sde1"));
                test(e.pd1 != null);
                test(e.pd1.sb.Equals("sb2"));
                test(e.pd1.pb == e.pd1);
                test(e.pd1.sd1.Equals("sd2"));
                test(e.pd1.pd1 == e.pd1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("derived exception thrown as base exception (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_throwDerivedAsBase().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        DerivedException e = (DerivedException)ex;
                        test(e.sbe.Equals("sbe"));
                        test(e.pb != null);
                        test(e.pb.sb.Equals("sb1"));
                        test(e.pb.pb == e.pb);
                        test(e.sde.Equals("sde1"));
                        test(e.pd1 != null);
                        test(e.pd1.sb.Equals("sb2"));
                        test(e.pd1.pb == e.pd1);
                        test(e.pd1.sd1.Equals("sd2"));
                        test(e.pd1.pd1 == e.pd1);
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();
        }
        WriteLine("ok");

        Write("derived exception thrown as derived exception... ");
        Flush();
        {
            try
            {
                testPrx.throwDerivedAsDerived();
                test(false);
            }
            catch(DerivedException e)
            {
                test(e.GetType().FullName.Equals("Test.DerivedException"));
                test(e.sbe.Equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.Equals("sb1"));
                test(e.pb.pb == e.pb);
                test(e.sde.Equals("sde1"));
                test(e.pd1 != null);
                test(e.pd1.sb.Equals("sb2"));
                test(e.pd1.pb == e.pd1);
                test(e.pd1.sd1.Equals("sd2"));
                test(e.pd1.pd1 == e.pd1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("derived exception thrown as derived exception (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_throwDerivedAsDerived().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        DerivedException e = (DerivedException)ex;
                        test(e.sbe.Equals("sbe"));
                        test(e.pb != null);
                        test(e.pb.sb.Equals("sb1"));
                        test(e.pb.pb == e.pb);
                        test(e.sde.Equals("sde1"));
                        test(e.pd1 != null);
                        test(e.pd1.sb.Equals("sb2"));
                        test(e.pd1.pb == e.pd1);
                        test(e.pd1.sd1.Equals("sd2"));
                        test(e.pd1.pd1 == e.pd1);
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();
        }
        WriteLine("ok");

        Write("unknown derived exception thrown as base exception... ");
        Flush();
        {
            try
            {
                testPrx.throwUnknownDerivedAsBase();
                test(false);
            }
            catch(BaseException e)
            {
                test(e.GetType().FullName.Equals("Test.BaseException"));
                test(e.sbe.Equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.Equals("sb d2"));
                test(e.pb.pb == e.pb);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("unknown derived exception thrown as base exception (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_throwUnknownDerivedAsBase().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    try
                    {
                        BaseException e = (BaseException)ex;
                        test(e.sbe.Equals("sbe"));
                        test(e.pb != null);
                        test(e.pb.sb.Equals("sb d2"));
                        test(e.pb.pb == e.pb);
                    }
                    catch(Exception)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();
        }
        WriteLine("ok");

        Write("forward-declared class... ");
        Flush();
        {
            try
            {
                Forward f;
                testPrx.useForward(out f);
                test(f != null);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("forward-declared class (AMI)... ");
        Flush();
        {
            Callback cb = new Callback();
            testPrx.begin_useForward().whenCompleted(
                (Forward f) =>
                {
                    test(f != null);
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("preserved classes... ");
        Flush();

        //
        // Register a factory in order to substitute our own subclass of Preserved. This provides
        // an easy way to determine how many unmarshaled instances currently exist.
        //
        // TODO: We have to install this now (even though it's not necessary yet), because otherwise
        // the Ice run time will install its own internal factory for Preserved upon receiving the
        // first instance.
        //
        communicator.addObjectFactory(new PreservedFactoryI(), Preserved.ice_staticId());

        try
        {
            //
            // Server knows the most-derived class PDerived.
            //
            PDerived pd = new PDerived();
            pd.pi = 3;
            pd.ps = "preserved";
            pd.pb = pd;

            PBase r = testPrx.exchangePBase(pd);
            PDerived p2 = r as PDerived;
            test(p2.pi == 3);
            test(p2.ps.Equals("preserved"));
            test(p2.pb == p2);
        }
        catch(Ice.OperationNotExistException)
        {
        }

        try
        {
            //
            // Server only knows the base (non-preserved) type, so the object is sliced.
            //
            PCUnknown pu = new PCUnknown();
            pu.pi = 3;
            pu.pu = "preserved";

            PBase r = testPrx.exchangePBase(pu);
            test(!(r is PCUnknown));
            test(r.pi == 3);
        }
        catch(Ice.OperationNotExistException)
        {
        }

        try
        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            PCDerived pcd = new PCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[] { pcd };

            PBase r = testPrx.exchangePBase(pcd);
            if(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0))
            {
                test(!(r is PCDerived));
                test(r.pi == 3);
            }
            else
            {
                PCDerived p2 = r as PCDerived;
                test(p2.pi == 3);
                test(p2.pbs[0] == p2);
            }
        }
        catch(Ice.OperationNotExistException)
        {
        }

        try
        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            CompactPCDerived pcd = new CompactPCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[] { pcd };

            PBase r = testPrx.exchangePBase(pcd);
            if(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0))
            {
                test(!(r is CompactPCDerived));
                test(r.pi == 3);
            }
            else
            {
                CompactPCDerived p2 = r as CompactPCDerived;
                test(p2.pi == 3);
                test(p2.pbs[0] == p2);
            }
        }
        catch(Ice.OperationNotExistException)
        {
        }

        try
        {
            //
            // Send an object that will have multiple preserved slices in the server.
            // The object will be sliced to Preserved for the 1.0 encoding.
            //
            PCDerived3 pcd = new PCDerived3();
            pcd.pi = 3;
            //
            // Sending more than 254 objects exercises the encoding for object ids.
            //
            pcd.pbs = new PBase[300];
            int i;
            for(i = 0; i < 300; ++i)
            {
                PCDerived2 p2 = new PCDerived2();
                p2.pi = i;
                p2.pbs = new PBase[] { null }; // Nil reference. This slice should not have an indirection table.
                p2.pcd2 = i;
                pcd.pbs[i] = p2;
            }
            pcd.pcd2 = pcd.pi;
            pcd.pcd3 = pcd.pbs[10];

            PBase r = testPrx.exchangePBase(pcd);
            if(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0))
            {
                test(!(r is PCDerived3));
                test(r is Preserved);
                test(r.pi == 3);
            }
            else
            {
                PCDerived3 p3 = r as PCDerived3;
                test(p3.pi == 3);
                for(i = 0; i < 300; ++i)
                {
                    PCDerived2 p2 = p3.pbs[i] as PCDerived2;
                    test(p2.pi == i);
                    test(p2.pbs.Length == 1);
                    test(p2.pbs[0] == null);
                    test(p2.pcd2 == i);
                }
                test(p3.pcd2 == p3.pi);
                test(p3.pcd3 == p3.pbs[10]);
            }
        }
        catch(Ice.OperationNotExistException)
        {
        }

        try
        {
            //
            // Obtain an object with preserved slices and send it back to the server.
            // The preserved slices should be excluded for the 1.0 encoding, otherwise
            // they should be included.
            //
            Preserved p = testPrx.PBSUnknownAsPreserved();
            testPrx.checkPBSUnknown(p);
            if(!testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0))
            {
                (testPrx.ice_encodingVersion(Ice.Util.Encoding_1_0) as TestIntfPrx).checkPBSUnknown(p);
            }
        }
        catch(Ice.OperationNotExistException)
        {
        }

        WriteLine("ok");

        Write("preserved classes (AMI)... ");
        Flush();
        {
            //
            // Server knows the most-derived class PDerived.
            //
            PDerived pd = new PDerived();
            pd.pi = 3;
            pd.ps = "preserved";
            pd.pb = pd;

            Callback cb = new Callback();
            testPrx.begin_exchangePBase(pd).whenCompleted(
                (PBase r) =>
                {
                    PDerived p2 = (PDerived)r;
                    test(p2.pi == 3);
                    test(p2.ps.Equals("preserved"));
                    test(p2.pb == p2);
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }

        {
            //
            // Server only knows the base (non-preserved) type, so the object is sliced.
            //
            PCUnknown pu = new PCUnknown();
            pu.pi = 3;
            pu.pu = "preserved";

            Callback cb = new Callback();
            testPrx.begin_exchangePBase(pu).whenCompleted(
                (PBase r) =>
                {
                    test(!(r is PCUnknown));
                    test(r.pi == 3);
                    cb.called();
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }

        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            PCDerived pcd = new PCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[] { pcd };

            Callback cb = new Callback();
            if(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0))
            {
                testPrx.begin_exchangePBase(pcd).whenCompleted(
                    (PBase r) =>
                    {
                        test(!(r is PCDerived));
                        test(r.pi == 3);
                        cb.called();
                    },
                    (Ice.Exception ex) =>
                    {
                        test(false);
                    });
            }
            else
            {
                testPrx.begin_exchangePBase(pcd).whenCompleted(
                    (PBase r) =>
                    {
                        PCDerived p2 = r as PCDerived;
                        test(p2.pi == 3);
                        test(p2.pbs[0] == p2);
                        cb.called();
                    },
                    (Ice.Exception ex) =>
                    {
                        test(false);
                    });
            }
            cb.check();
        }

        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            CompactPCDerived pcd = new CompactPCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[] { pcd };

            Callback cb = new Callback();
            if(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0))
            {
                testPrx.begin_exchangePBase(pcd).whenCompleted(
                    (PBase r) =>
                    {
                        test(!(r is CompactPCDerived));
                        test(r.pi == 3);
                        cb.called();
                    },
                    (Ice.Exception ex) =>
                    {
                        test(false);
                    });
            }
            else
            {
                testPrx.begin_exchangePBase(pcd).whenCompleted(
                    (PBase r) =>
                    {
                        CompactPCDerived p2 = r as CompactPCDerived;
                        test(p2.pi == 3);
                        test(p2.pbs[0] == p2);
                        cb.called();
                    },
                    (Ice.Exception ex) =>
                    {
                        test(false);
                    });
            }
            cb.check();
        }

        {
            //
            // Send an object that will have multiple preserved slices in the server.
            // The object will be sliced to Preserved for the 1.0 encoding.
            //
            PCDerived3 pcd = new PCDerived3();
            pcd.pi = 3;
            //
            // Sending more than 254 objects exercises the encoding for object ids.
            //
            pcd.pbs = new PBase[300];
            for(int i = 0; i < 300; ++i)
            {
                PCDerived2 p2 = new PCDerived2();
                p2.pi = i;
                p2.pbs = new PBase[] { null }; // Nil reference. This slice should not have an indirection table.
                p2.pcd2 = i;
                pcd.pbs[i] = p2;
            }
            pcd.pcd2 = pcd.pi;
            pcd.pcd3 = pcd.pbs[10];

            Callback cb = new Callback();
            if(testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0))
            {
                testPrx.begin_exchangePBase(pcd).whenCompleted(
                    (PBase r) =>
                    {
                        test(!(r is PCDerived3));
                        test(r is Preserved);
                        test(r.pi == 3);
                        cb.called();
                    },
                    (Ice.Exception ex) =>
                    {
                        test(false);
                    });
            }
            else
            {
                testPrx.begin_exchangePBase(pcd).whenCompleted(
                    (PBase r) =>
                    {
                        PCDerived3 p3 = r as PCDerived3;
                        test(p3.pi == 3);
                        for(int i = 0; i < 300; ++i)
                        {
                            PCDerived2 p2 = p3.pbs[i] as PCDerived2;
                            test(p2.pi == i);
                            test(p2.pbs.Length == 1);
                            test(p2.pbs[0] == null);
                            test(p2.pcd2 == i);
                        }
                        test(p3.pcd2 == p3.pi);
                        test(p3.pcd3 == p3.pbs[10]);
                        cb.called();
                    },
                    (Ice.Exception ex) =>
                    {
                        test(false);
                    });
            }
            cb.check();
        }

        try
        {
            //
            // Obtain an object with preserved slices and send it back to the server.
            // The preserved slices should be excluded for the 1.0 encoding, otherwise
            // they should be included.
            //
            Preserved p = testPrx.PBSUnknownAsPreserved();
            testPrx.checkPBSUnknown(p);
            if(!testPrx.ice_getEncodingVersion().Equals(Ice.Util.Encoding_1_0))
            {
                (testPrx.ice_encodingVersion(Ice.Util.Encoding_1_0) as TestIntfPrx).checkPBSUnknown(p);
            }
        }
        catch(Ice.OperationNotExistException)
        {
        }

        WriteLine("ok");

        Write("garbage collection for preserved classes... ");
        Flush();
        try
        {
            //
            // Register a factory in order to substitute our own subclass of PNode. This provides
            // an easy way to determine how many unmarshaled instances currently exist.
            //
            communicator.addObjectFactory(new NodeFactoryI(), PNode.ice_staticId());

            //
            // Relay a graph through the server.
            //
            {
                PNode c = new PNode();
                c.next = new PNode();
                c.next.next = new PNode();
                c.next.next.next = c;

                test(PNodeI.counter == 0);
                PNode n = testPrx.exchangePNode(c);

                test(PNodeI.counter == 3);
                PNodeI.counter = 0;
                n.next = null;
            }

            //
            // Obtain a preserved object from the server where the most-derived
            // type is unknown. The preserved slice refers to a graph of PNode
            // objects.
            //
            {
                test(PNodeI.counter == 0);
                Preserved p = testPrx.PBSUnknownAsPreservedWithGraph();
                testPrx.checkPBSUnknownWithGraph(p);
                test(PNodeI.counter == 3);
                PNodeI.counter = 0;
            }

            //
            // Obtain a preserved object from the server where the most-derived
            // type is unknown. A data member in the preserved slice refers to the
            // outer object, so the chain of references looks like this:
            //
            // outer.slicedData.outer
            //
            {
                PreservedI.counter = 0;
                Preserved p = testPrx.PBSUnknown2AsPreservedWithGraph();
                testPrx.checkPBSUnknown2WithGraph(p);
                test(PreservedI.counter == 1);
                PreservedI.counter = 0;
            }

            //
            // Throw a preserved exception where the most-derived type is unknown.
            // The preserved exception slice contains a class data member. This
            // object is also preserved, and its most-derived type is also unknown.
            // The preserved slice of the object contains a class data member that
            // refers to itself.
            //
            // The chain of references looks like this:
            //
            // ex.slicedData.obj.slicedData.obj
            //
            try
            {
                test(PreservedI.counter == 0);

                try
                {
                    testPrx.throwPreservedException();
                }
                catch(PreservedException)
                {
                    test(PreservedI.counter == 1);
                }

                PreservedI.counter = 0;
            }
            catch(Exception)
            {
                test(false);
            }
        }
        catch(Ice.OperationNotExistException)
        {
        }

        WriteLine("ok");

#if SILVERLIGHT
        testPrx.shutdown();
#else
        return testPrx;
#endif
    }
}
