//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.scope;

import java.io.PrintWriter;

public class AllTests
{
    private static void test(boolean b)
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

        public synchronized void check()
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

        public synchronized void called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    public static void allTests(test.TestHelper helper)
    {
        PrintWriter out = helper.getWriter();
        Ice.Communicator communicator = helper.communicator();

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i1:" + helper.getTestEndpoint());
            test.Ice.scope.Test.IPrx i = test.Ice.scope.Test.IPrxHelper.checkedCast(obj);

            test.Ice.scope.Test.S s1 = new test.Ice.scope.Test.S(0);
            test.Ice.scope.Test.SHolder s2 = new test.Ice.scope.Test.SHolder();
            test.Ice.scope.Test.S s3 = i.opS(s1, s2);
            test(s1.equals(s2.value));
            test(s1.equals(s3));

            test.Ice.scope.Test.S[] sseq1 = new test.Ice.scope.Test.S[]{ s1 };
            test.Ice.scope.Test.SSeqHolder sseq2 = new test.Ice.scope.Test.SSeqHolder();
            test.Ice.scope.Test.S[] sseq3 = i.opSSeq(sseq1, sseq2);
            test(sseq2.value[0].equals(s1));
            test(sseq3[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.S> smap1 = new java.util.HashMap<String, test.Ice.scope.Test.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.SMapHolder smap2 = new test.Ice.scope.Test.SMapHolder();
            java.util.Map<String, test.Ice.scope.Test.S> smap3 = i.opSMap(smap1, smap2);
            test(smap2.value.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            test.Ice.scope.Test.C c1 = new test.Ice.scope.Test.C(s1);
            test.Ice.scope.Test.CHolder c2 = new test.Ice.scope.Test.CHolder();
            test.Ice.scope.Test.C c3 = i.opC(c1, c2);
            test(c1.s.equals(c2.value.s));
            test(c1.s.equals(c3.s));

            test.Ice.scope.Test.C[] cseq1 = new test.Ice.scope.Test.C[]{ c1 };
            test.Ice.scope.Test.CSeqHolder cseq2 = new test.Ice.scope.Test.CSeqHolder();
            test.Ice.scope.Test.C[] cseq3 = i.opCSeq(cseq1, cseq2);
            test(cseq2.value[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.C> cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.CMapHolder cmap2 = new test.Ice.scope.Test.CMapHolder();
            java.util.Map<String, test.Ice.scope.Test.C> cmap3 = i.opCMap(cmap1, cmap2);
            test(cmap2.value.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i1:" + helper.getTestEndpoint());
            test.Ice.scope.Test.IPrx i = test.Ice.scope.Test.IPrxHelper.checkedCast(obj);

            test.Ice.scope.Test.S s1 = new test.Ice.scope.Test.S(0);
            test.Ice.scope.Test.SHolder s2 = new test.Ice.scope.Test.SHolder();
            test.Ice.scope.Test.S s3 = i.end_opS(s2, i.begin_opS(s1));
            test(s1.equals(s2.value));
            test(s1.equals(s3));

            test.Ice.scope.Test.S[] sseq1 = new test.Ice.scope.Test.S[]{ s1 };
            test.Ice.scope.Test.SSeqHolder sseq2 = new test.Ice.scope.Test.SSeqHolder();
            test.Ice.scope.Test.S[] sseq3 = i.end_opSSeq(sseq2, i.begin_opSSeq(sseq1));
            test(sseq2.value[0].equals(s1));
            test(sseq3[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.S> smap1 = new java.util.HashMap<String, test.Ice.scope.Test.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.SMapHolder smap2 = new test.Ice.scope.Test.SMapHolder();
            java.util.Map<String, test.Ice.scope.Test.S> smap3 = i.end_opSMap(smap2, i.begin_opSMap(smap1));
            test(smap2.value.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            test.Ice.scope.Test.C c1 = new test.Ice.scope.Test.C(s1);
            test.Ice.scope.Test.CHolder c2 = new test.Ice.scope.Test.CHolder();
            test.Ice.scope.Test.C c3 = i.end_opC(c2, i.begin_opC(c1));
            test(c1.s.equals(c2.value.s));
            test(c1.s.equals(c3.s));

            test.Ice.scope.Test.C[] cseq1 = new test.Ice.scope.Test.C[]{ c1 };
            test.Ice.scope.Test.CSeqHolder cseq2 = new test.Ice.scope.Test.CSeqHolder();
            test.Ice.scope.Test.C[] cseq3 = i.end_opCSeq(cseq2, i.begin_opCSeq(cseq1));
            test(cseq2.value[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.C> cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.CMapHolder cmap2 = new test.Ice.scope.Test.CMapHolder();
            java.util.Map<String, test.Ice.scope.Test.C> cmap3 = i.end_opCMap(cmap2, i.begin_opCMap(cmap1));
            test(cmap2.value.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i1:" + helper.getTestEndpoint());
            test.Ice.scope.Test.IPrx i = test.Ice.scope.Test.IPrxHelper.checkedCast(obj);

            final test.Ice.scope.Test.S s1 = new test.Ice.scope.Test.S(0);
            {
                class OpSCallback extends test.Ice.scope.Test.Callback_I_opS
                {
                    public void response(test.Ice.scope.Test.S s2, test.Ice.scope.Test.S s3)
                    {
                        test(s1.equals(s2));
                        test(s1.equals(s3));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpSCallback cb = new OpSCallback();
                i.begin_opS(s1, cb);
                cb.check();
            }

            test.Ice.scope.Test.S[] sseq1 = new test.Ice.scope.Test.S[]{ s1 };
            {
                class OpSSeqCallback extends test.Ice.scope.Test.Callback_I_opSSeq
                {
                    public void response(test.Ice.scope.Test.S[] s2, test.Ice.scope.Test.S[] s3)
                    {
                        test(s1.equals(s2[0]));
                        test(s1.equals(s3[0]));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpSSeqCallback cb = new OpSSeqCallback();
                i.begin_opSSeq(sseq1, cb);
                cb.check();
            }

            java.util.Map<String, test.Ice.scope.Test.S> smap1 = new java.util.HashMap<String, test.Ice.scope.Test.S>();
            smap1.put("a", s1);
            {
                class OpSMapCallback extends test.Ice.scope.Test.Callback_I_opSMap
                {
                    public void response(java.util.Map<String, test.Ice.scope.Test.S> s2,
                                         java.util.Map<String, test.Ice.scope.Test.S> s3)
                    {
                        test(s1.equals(s2.get("a")));
                        test(s1.equals(s3.get("a")));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpSMapCallback cb = new OpSMapCallback();
                i.begin_opSMap(smap1, cb);
                cb.check();
            }

            test.Ice.scope.Test.C c1 = new test.Ice.scope.Test.C(s1);
            {
                class OpCCallback extends test.Ice.scope.Test.Callback_I_opC
                {
                    public void response(test.Ice.scope.Test.C c2,
                                         test.Ice.scope.Test.C c3)
                    {
                        test(s1.equals(c2.s));
                        test(s1.equals(c3.s));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpCCallback cb = new OpCCallback();
                i.begin_opC(c1, cb);
                cb.check();
            }

            test.Ice.scope.Test.C[] cseq1 = new test.Ice.scope.Test.C[]{ c1 };
            {
                class OpCSeqCallback extends test.Ice.scope.Test.Callback_I_opCSeq
                {
                    public void response(test.Ice.scope.Test.C[] c2,
                                         test.Ice.scope.Test.C[] c3)
                    {
                        test(s1.equals(c2[0].s));
                        test(s1.equals(c3[0].s));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpCSeqCallback cb = new OpCSeqCallback();
                i.begin_opCSeq(cseq1, cb);
                cb.check();
            }

            java.util.Map<String, test.Ice.scope.Test.C> cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.C>();
            cmap1.put("a", c1);
            {
                class OpCMapCallback extends test.Ice.scope.Test.Callback_I_opCMap
                {
                    public void response(java.util.Map<String, test.Ice.scope.Test.C> c2,
                                         java.util.Map<String, test.Ice.scope.Test.C> c3)
                    {
                        test(s1.equals(c2.get("a").s));
                        test(s1.equals(c3.get("a").s));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpCMapCallback cb = new OpCMapCallback();
                i.begin_opCMap(cmap1, cb);
                cb.check();
            }
        }

        //  test.Ice.scope

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i2:" + helper.getTestEndpoint());
            test.Ice.scope.Test.Inner.IPrx i = test.Ice.scope.Test.Inner.IPrxHelper.checkedCast(obj);

            test.Ice.scope.Test.Inner.Inner2.S s1 = new test.Ice.scope.Test.Inner.Inner2.S(0);
            test.Ice.scope.Test.Inner.Inner2.SHolder s2 = new test.Ice.scope.Test.Inner.Inner2.SHolder();
            test.Ice.scope.Test.Inner.Inner2.S s3 = i.opS(s1, s2);
            test(s1.equals(s2.value));
            test(s1.equals(s3));

            test.Ice.scope.Test.Inner.Inner2.S[] sseq1 = new test.Ice.scope.Test.Inner.Inner2.S[]{ s1 };
            test.Ice.scope.Test.Inner.Inner2.SSeqHolder sseq2 = new test.Ice.scope.Test.Inner.Inner2.SSeqHolder();
            test.Ice.scope.Test.Inner.Inner2.S[] sseq3 = i.opSSeq(sseq1, sseq2);
            test(sseq2.value[0].equals(s1));
            test(sseq3[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.Inner.Inner2.SMapHolder smap2 = new test.Ice.scope.Test.Inner.Inner2.SMapHolder();
            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap3 = i.opSMap(smap1, smap2);
            test(smap2.value.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            test.Ice.scope.Test.Inner.Inner2.C c1 = new test.Ice.scope.Test.Inner.Inner2.C(s1);
            test.Ice.scope.Test.Inner.Inner2.CHolder c2 = new test.Ice.scope.Test.Inner.Inner2.CHolder();
            test.Ice.scope.Test.Inner.Inner2.C c3 = i.opC(c1, c2);
            test(c1.s.equals(c2.value.s));
            test(c1.s.equals(c3.s));

            test.Ice.scope.Test.Inner.Inner2.C[] cseq1 = new test.Ice.scope.Test.Inner.Inner2.C[]{ c1 };
            test.Ice.scope.Test.Inner.Inner2.CSeqHolder cseq2 = new test.Ice.scope.Test.Inner.Inner2.CSeqHolder();
            test.Ice.scope.Test.Inner.Inner2.C[] cseq3 = i.opCSeq(cseq1, cseq2);
            test(cseq2.value[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.Inner.Inner2.CMapHolder cmap2 = new test.Ice.scope.Test.Inner.Inner2.CMapHolder();
            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap3 = i.opCMap(cmap1, cmap2);
            test(cmap2.value.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i2:" + helper.getTestEndpoint());
            test.Ice.scope.Test.Inner.IPrx i = test.Ice.scope.Test.Inner.IPrxHelper.checkedCast(obj);

            test.Ice.scope.Test.Inner.Inner2.S s1 = new test.Ice.scope.Test.Inner.Inner2.S(0);
            test.Ice.scope.Test.Inner.Inner2.SHolder s2 = new test.Ice.scope.Test.Inner.Inner2.SHolder();
            test.Ice.scope.Test.Inner.Inner2.S s3 = i.end_opS(s2, i.begin_opS(s1));
            test(s1.equals(s2.value));
            test(s1.equals(s3));

            test.Ice.scope.Test.Inner.Inner2.S[] sseq1 = new test.Ice.scope.Test.Inner.Inner2.S[]{ s1 };
            test.Ice.scope.Test.Inner.Inner2.SSeqHolder sseq2 = new test.Ice.scope.Test.Inner.Inner2.SSeqHolder();
            test.Ice.scope.Test.Inner.Inner2.S[] sseq3 = i.end_opSSeq(sseq2, i.begin_opSSeq(sseq1));
            test(sseq2.value[0].equals(s1));
            test(sseq3[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.Inner.Inner2.SMapHolder smap2 = new test.Ice.scope.Test.Inner.Inner2.SMapHolder();
            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap3 =
                i.end_opSMap(smap2, i.begin_opSMap(smap1));
            test(smap2.value.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            test.Ice.scope.Test.Inner.Inner2.C c1 = new test.Ice.scope.Test.Inner.Inner2.C(s1);
            test.Ice.scope.Test.Inner.Inner2.CHolder c2 = new test.Ice.scope.Test.Inner.Inner2.CHolder();
            test.Ice.scope.Test.Inner.Inner2.C c3 = i.end_opC(c2, i.begin_opC(c1));
            test(c1.s.equals(c2.value.s));
            test(c1.s.equals(c3.s));

            test.Ice.scope.Test.Inner.Inner2.C[] cseq1 = new test.Ice.scope.Test.Inner.Inner2.C[]{ c1 };
            test.Ice.scope.Test.Inner.Inner2.CSeqHolder cseq2 = new test.Ice.scope.Test.Inner.Inner2.CSeqHolder();
            test.Ice.scope.Test.Inner.Inner2.C[] cseq3 = i.end_opCSeq(cseq2, i.begin_opCSeq(cseq1));
            test(cseq2.value[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.Inner.Inner2.CMapHolder cmap2 = new test.Ice.scope.Test.Inner.Inner2.CMapHolder();
            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap3 =
                i.end_opCMap(cmap2, i.begin_opCMap(cmap1));
            test(cmap2.value.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i2:" + helper.getTestEndpoint());
            test.Ice.scope.Test.Inner.IPrx i = test.Ice.scope.Test.Inner.IPrxHelper.checkedCast(obj);

            final test.Ice.scope.Test.Inner.Inner2.S s1 = new test.Ice.scope.Test.Inner.Inner2.S(0);
            {
                class OpSCallback extends test.Ice.scope.Test.Inner.Callback_I_opS
                {
                    public void response(test.Ice.scope.Test.Inner.Inner2.S s2,
                                         test.Ice.scope.Test.Inner.Inner2.S s3)
                    {
                        test(s1.equals(s2));
                        test(s1.equals(s3));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpSCallback cb = new OpSCallback();
                i.begin_opS(s1, cb);
                cb.check();
            }

            test.Ice.scope.Test.Inner.Inner2.S[] sseq1 = new test.Ice.scope.Test.Inner.Inner2.S[]{ s1 };
            {
                class OpSSeqCallback extends test.Ice.scope.Test.Inner.Callback_I_opSSeq
                {
                    public void response(test.Ice.scope.Test.Inner.Inner2.S[] s2,
                                         test.Ice.scope.Test.Inner.Inner2.S[] s3)
                    {
                        test(s1.equals(s2[0]));
                        test(s1.equals(s3[0]));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpSSeqCallback cb = new OpSSeqCallback();
                i.begin_opSSeq(sseq1, cb);
                cb.check();
            }

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.S>();
            smap1.put("a", s1);
            {
                class OpSMapCallback extends test.Ice.scope.Test.Inner.Callback_I_opSMap
                {
                    public void response(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> s2,
                                         java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> s3)
                    {
                        test(s1.equals(s2.get("a")));
                        test(s1.equals(s3.get("a")));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpSMapCallback cb = new OpSMapCallback();
                i.begin_opSMap(smap1, cb);
                cb.check();
            }

            test.Ice.scope.Test.Inner.Inner2.C c1 = new test.Ice.scope.Test.Inner.Inner2.C(s1);
            {
                class OpCCallback extends test.Ice.scope.Test.Inner.Callback_I_opC
                {
                    public void response(test.Ice.scope.Test.Inner.Inner2.C c2,
                                         test.Ice.scope.Test.Inner.Inner2.C c3)
                    {
                        test(s1.equals(c2.s));
                        test(s1.equals(c3.s));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpCCallback cb = new OpCCallback();
                i.begin_opC(c1, cb);
                cb.check();
            }

            test.Ice.scope.Test.Inner.Inner2.C[] cseq1 =
                new test.Ice.scope.Test.Inner.Inner2.C[]{ c1 };
            {
                class OpCSeqCallback extends test.Ice.scope.Test.Inner.Callback_I_opCSeq
                {
                    public void response(test.Ice.scope.Test.Inner.Inner2.C[] c2,
                                         test.Ice.scope.Test.Inner.Inner2.C[] c3)
                    {
                        test(s1.equals(c2[0].s));
                        test(s1.equals(c3[0].s));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpCSeqCallback cb = new OpCSeqCallback();
                i.begin_opCSeq(cseq1, cb);
                cb.check();
            }

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.C>();
            cmap1.put("a", c1);
            {
                class OpCMapCallback extends test.Ice.scope.Test.Inner.Callback_I_opCMap
                {
                    public void response(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> c2,
                                         java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> c3)
                    {
                        test(s1.equals(c2.get("a").s));
                        test(s1.equals(c3.get("a").s));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpCMapCallback cb = new OpCMapCallback();
                i.begin_opCMap(cmap1, cb);
                cb.check();
            }
        }

        //  test.Ice.scope

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i3:" + helper.getTestEndpoint());
            test.Ice.scope.Test.Inner.Inner2.IPrx i = test.Ice.scope.Test.Inner.Inner2.IPrxHelper.checkedCast(obj);

            test.Ice.scope.Test.Inner.Inner2.S s1 = new test.Ice.scope.Test.Inner.Inner2.S(0);
            test.Ice.scope.Test.Inner.Inner2.SHolder s2 = new test.Ice.scope.Test.Inner.Inner2.SHolder();
            test.Ice.scope.Test.Inner.Inner2.S s3 = i.opS(s1, s2);
            test(s1.equals(s2.value));
            test(s1.equals(s3));

            test.Ice.scope.Test.Inner.Inner2.S[] sseq1 = new test.Ice.scope.Test.Inner.Inner2.S[]{ s1 };
            test.Ice.scope.Test.Inner.Inner2.SSeqHolder sseq2 = new test.Ice.scope.Test.Inner.Inner2.SSeqHolder();
            test.Ice.scope.Test.Inner.Inner2.S[] sseq3 = i.opSSeq(sseq1, sseq2);
            test(sseq2.value[0].equals(s1));
            test(sseq3[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.Inner.Inner2.SMapHolder smap2 = new test.Ice.scope.Test.Inner.Inner2.SMapHolder();
            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap3 = i.opSMap(smap1, smap2);
            test(smap2.value.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            test.Ice.scope.Test.Inner.Inner2.C c1 = new test.Ice.scope.Test.Inner.Inner2.C(s1);
            test.Ice.scope.Test.Inner.Inner2.CHolder c2 = new test.Ice.scope.Test.Inner.Inner2.CHolder();
            test.Ice.scope.Test.Inner.Inner2.C c3 = i.opC(c1, c2);
            test(c1.s.equals(c2.value.s));
            test(c1.s.equals(c3.s));

            test.Ice.scope.Test.Inner.Inner2.C[] cseq1 = new test.Ice.scope.Test.Inner.Inner2.C[]{ c1 };
            test.Ice.scope.Test.Inner.Inner2.CSeqHolder cseq2 = new test.Ice.scope.Test.Inner.Inner2.CSeqHolder();
            test.Ice.scope.Test.Inner.Inner2.C[] cseq3 = i.opCSeq(cseq1, cseq2);
            test(cseq2.value[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.Inner.Inner2.CMapHolder cmap2 = new test.Ice.scope.Test.Inner.Inner2.CMapHolder();
            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap3 = i.opCMap(cmap1, cmap2);
            test(cmap2.value.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i3:" + helper.getTestEndpoint());
            test.Ice.scope.Test.Inner.Inner2.IPrx i = test.Ice.scope.Test.Inner.Inner2.IPrxHelper.checkedCast(obj);

            test.Ice.scope.Test.Inner.Inner2.S s1 = new test.Ice.scope.Test.Inner.Inner2.S(0);
            test.Ice.scope.Test.Inner.Inner2.SHolder s2 = new test.Ice.scope.Test.Inner.Inner2.SHolder();
            test.Ice.scope.Test.Inner.Inner2.S s3 = i.end_opS(s2, i.begin_opS(s1));
            test(s1.equals(s2.value));
            test(s1.equals(s3));

            test.Ice.scope.Test.Inner.Inner2.S[] sseq1 = new test.Ice.scope.Test.Inner.Inner2.S[]{ s1 };
            test.Ice.scope.Test.Inner.Inner2.SSeqHolder sseq2 = new test.Ice.scope.Test.Inner.Inner2.SSeqHolder();
            test.Ice.scope.Test.Inner.Inner2.S[] sseq3 = i.end_opSSeq(sseq2, i.begin_opSSeq(sseq1));
            test(sseq2.value[0].equals(s1));
            test(sseq3[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.Inner.Inner2.SMapHolder smap2 = new test.Ice.scope.Test.Inner.Inner2.SMapHolder();
            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap3 =
                i.end_opSMap(smap2, i.begin_opSMap(smap1));
            test(smap2.value.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            test.Ice.scope.Test.Inner.Inner2.C c1 = new test.Ice.scope.Test.Inner.Inner2.C(s1);
            test.Ice.scope.Test.Inner.Inner2.CHolder c2 = new test.Ice.scope.Test.Inner.Inner2.CHolder();
            test.Ice.scope.Test.Inner.Inner2.C c3 = i.end_opC(c2, i.begin_opC(c1));
            test(c1.s.equals(c2.value.s));
            test(c1.s.equals(c3.s));

            test.Ice.scope.Test.Inner.Inner2.C[] cseq1 = new test.Ice.scope.Test.Inner.Inner2.C[]{ c1 };
            test.Ice.scope.Test.Inner.Inner2.CSeqHolder cseq2 = new test.Ice.scope.Test.Inner.Inner2.CSeqHolder();
            test.Ice.scope.Test.Inner.Inner2.C[] cseq3 = i.end_opCSeq(cseq2, i.begin_opCSeq(cseq1));
            test(cseq2.value[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.Inner.Inner2.CMapHolder cmap2 = new test.Ice.scope.Test.Inner.Inner2.CMapHolder();
            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap3 =
                i.end_opCMap(cmap2, i.begin_opCMap(cmap1));
            test(cmap2.value.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i3:" + helper.getTestEndpoint());
            test.Ice.scope.Test.Inner.Inner2.IPrx i = test.Ice.scope.Test.Inner.Inner2.IPrxHelper.checkedCast(obj);

            final test.Ice.scope.Test.Inner.Inner2.S s1 = new test.Ice.scope.Test.Inner.Inner2.S(0);
            {
                class OpSCallback extends test.Ice.scope.Test.Inner.Inner2.Callback_I_opS
                {
                    public void response(test.Ice.scope.Test.Inner.Inner2.S s2,
                                         test.Ice.scope.Test.Inner.Inner2.S s3)
                    {
                        test(s1.equals(s2));
                        test(s1.equals(s3));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpSCallback cb = new OpSCallback();
                i.begin_opS(s1, cb);
                cb.check();
            }

            test.Ice.scope.Test.Inner.Inner2.S[] sseq1 = new test.Ice.scope.Test.Inner.Inner2.S[]{ s1 };
            {
                class OpSSeqCallback extends test.Ice.scope.Test.Inner.Inner2.Callback_I_opSSeq
                {
                    public void response(test.Ice.scope.Test.Inner.Inner2.S[] s2,
                                         test.Ice.scope.Test.Inner.Inner2.S[] s3)
                    {
                        test(s1.equals(s2[0]));
                        test(s1.equals(s3[0]));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpSSeqCallback cb = new OpSSeqCallback();
                i.begin_opSSeq(sseq1, cb);
                cb.check();
            }

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.S>();
            smap1.put("a", s1);
            {
                class OpSMapCallback extends test.Ice.scope.Test.Inner.Inner2.Callback_I_opSMap
                {
                    public void response(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> s2,
                                         java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> s3)
                    {
                        test(s1.equals(s2.get("a")));
                        test(s1.equals(s3.get("a")));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpSMapCallback cb = new OpSMapCallback();
                i.begin_opSMap(smap1, cb);
                cb.check();
            }

            test.Ice.scope.Test.Inner.Inner2.C c1 = new test.Ice.scope.Test.Inner.Inner2.C(s1);
            {
                class OpCCallback extends test.Ice.scope.Test.Inner.Inner2.Callback_I_opC
                {
                    public void response(test.Ice.scope.Test.Inner.Inner2.C c2,
                                         test.Ice.scope.Test.Inner.Inner2.C c3)
                    {
                        test(s1.equals(c2.s));
                        test(s1.equals(c3.s));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpCCallback cb = new OpCCallback();
                i.begin_opC(c1, cb);
                cb.check();
            }

            test.Ice.scope.Test.Inner.Inner2.C[] cseq1 =
                new test.Ice.scope.Test.Inner.Inner2.C[]{ c1 };
            {
                class OpCSeqCallback extends test.Ice.scope.Test.Inner.Inner2.Callback_I_opCSeq
                {
                    public void response(test.Ice.scope.Test.Inner.Inner2.C[] c2,
                                         test.Ice.scope.Test.Inner.Inner2.C[] c3)
                    {
                        test(s1.equals(c2[0].s));
                        test(s1.equals(c3[0].s));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpCSeqCallback cb = new OpCSeqCallback();
                i.begin_opCSeq(cseq1, cb);
                cb.check();
            }

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.C>();
            cmap1.put("a", c1);
            {
                class OpCMapCallback extends test.Ice.scope.Test.Inner.Inner2.Callback_I_opCMap
                {
                    public void response(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> c2,
                                         java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> c3)
                    {
                        test(s1.equals(c2.get("a").s));
                        test(s1.equals(c3.get("a").s));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpCMapCallback cb = new OpCMapCallback();
                i.begin_opCMap(cmap1, cb);
                cb.check();
            }
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i4:" + helper.getTestEndpoint());
            test.Ice.scope.Inner.Test.Inner2.IPrx i = test.Ice.scope.Inner.Test.Inner2.IPrxHelper.checkedCast(obj);

            test.Ice.scope.Test.S s1 = new test.Ice.scope.Test.S(0);
            test.Ice.scope.Test.SHolder s2 = new test.Ice.scope.Test.SHolder();
            test.Ice.scope.Test.S s3 = i.opS(s1, s2);
            test(s1.equals(s2.value));
            test(s1.equals(s3));

            test.Ice.scope.Test.S[] sseq1 = new test.Ice.scope.Test.S[]{ s1 };
            test.Ice.scope.Test.SSeqHolder sseq2 = new test.Ice.scope.Test.SSeqHolder();
            test.Ice.scope.Test.S[] sseq3 = i.opSSeq(sseq1, sseq2);
            test(sseq2.value[0].equals(s1));
            test(sseq3[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.S> smap1 = new java.util.HashMap<String, test.Ice.scope.Test.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.SMapHolder smap2 = new test.Ice.scope.Test.SMapHolder();
            java.util.Map<String, test.Ice.scope.Test.S> smap3 = i.opSMap(smap1, smap2);
            test(smap2.value.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            test.Ice.scope.Test.C c1 = new test.Ice.scope.Test.C(s1);
            test.Ice.scope.Test.CHolder c2 = new test.Ice.scope.Test.CHolder();
            test.Ice.scope.Test.C c3 = i.opC(c1, c2);
            test(c1.s.equals(c2.value.s));
            test(c1.s.equals(c3.s));

            test.Ice.scope.Test.C[] cseq1 = new test.Ice.scope.Test.C[]{ c1 };
            test.Ice.scope.Test.CSeqHolder cseq2 = new test.Ice.scope.Test.CSeqHolder();
            test.Ice.scope.Test.C[] cseq3 = i.opCSeq(cseq1, cseq2);
            test(cseq2.value[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.C> cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.CMapHolder cmap2 = new test.Ice.scope.Test.CMapHolder();
            java.util.Map<String, test.Ice.scope.Test.C> cmap3 = i.opCMap(cmap1, cmap2);
            test(cmap2.value.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i4:" + helper.getTestEndpoint());
            test.Ice.scope.Inner.Test.Inner2.IPrx i = test.Ice.scope.Inner.Test.Inner2.IPrxHelper.checkedCast(obj);

            test.Ice.scope.Test.S s1 = new test.Ice.scope.Test.S(0);
            test.Ice.scope.Test.SHolder s2 = new test.Ice.scope.Test.SHolder();
            test.Ice.scope.Test.S s3 = i.end_opS(s2, i.begin_opS(s1));
            test(s1.equals(s2.value));
            test(s1.equals(s3));

            test.Ice.scope.Test.S[] sseq1 = new test.Ice.scope.Test.S[]{ s1 };
            test.Ice.scope.Test.SSeqHolder sseq2 = new test.Ice.scope.Test.SSeqHolder();
            test.Ice.scope.Test.S[] sseq3 = i.end_opSSeq(sseq2, i.begin_opSSeq(sseq1));
            test(sseq2.value[0].equals(s1));
            test(sseq3[0].equals(s1));

            java.util.Map<String, test.Ice.scope.Test.S> smap1 = new java.util.HashMap<String, test.Ice.scope.Test.S>();
            smap1.put("a", s1);
            test.Ice.scope.Test.SMapHolder smap2 = new test.Ice.scope.Test.SMapHolder();
            java.util.Map<String, test.Ice.scope.Test.S> smap3 = i.end_opSMap(smap2, i.begin_opSMap(smap1));
            test(smap2.value.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            test.Ice.scope.Test.C c1 = new test.Ice.scope.Test.C(s1);
            test.Ice.scope.Test.CHolder c2 = new test.Ice.scope.Test.CHolder();
            test.Ice.scope.Test.C c3 = i.end_opC(c2, i.begin_opC(c1));
            test(c1.s.equals(c2.value.s));
            test(c1.s.equals(c3.s));

            test.Ice.scope.Test.C[] cseq1 = new test.Ice.scope.Test.C[]{ c1 };
            test.Ice.scope.Test.CSeqHolder cseq2 = new test.Ice.scope.Test.CSeqHolder();
            test.Ice.scope.Test.C[] cseq3 = i.end_opCSeq(cseq2, i.begin_opCSeq(cseq1));
            test(cseq2.value[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            java.util.Map<String, test.Ice.scope.Test.C> cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.C>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.CMapHolder cmap2 = new test.Ice.scope.Test.CMapHolder();
            java.util.Map<String, test.Ice.scope.Test.C> cmap3 = i.end_opCMap(cmap2, i.begin_opCMap(cmap1));
            test(cmap2.value.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i3:" + helper.getTestEndpoint());
            test.Ice.scope.Test.Inner.Inner2.IPrx i = test.Ice.scope.Test.Inner.Inner2.IPrxHelper.checkedCast(obj);

            final test.Ice.scope.Test.Inner.Inner2.S s1 = new test.Ice.scope.Test.Inner.Inner2.S(0);
            {
                class OpSCallback extends test.Ice.scope.Test.Inner.Inner2.Callback_I_opS
                {
                    public void response(test.Ice.scope.Test.Inner.Inner2.S s2,
                                         test.Ice.scope.Test.Inner.Inner2.S s3)
                    {
                        test(s1.equals(s2));
                        test(s1.equals(s3));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpSCallback cb = new OpSCallback();
                i.begin_opS(s1, cb);
                cb.check();
            }

            test.Ice.scope.Test.Inner.Inner2.S[] sseq1 = new test.Ice.scope.Test.Inner.Inner2.S[]{ s1 };
            {
                class OpSSeqCallback extends test.Ice.scope.Test.Inner.Inner2.Callback_I_opSSeq
                {
                    public void response(test.Ice.scope.Test.Inner.Inner2.S[] s2,
                                         test.Ice.scope.Test.Inner.Inner2.S[] s3)
                    {
                        test(s1.equals(s2[0]));
                        test(s1.equals(s3[0]));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpSSeqCallback cb = new OpSSeqCallback();
                i.begin_opSSeq(sseq1, cb);
                cb.check();
            }

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> smap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.S>();
            smap1.put("a", s1);
            {
                class OpSMapCallback extends test.Ice.scope.Test.Inner.Inner2.Callback_I_opSMap
                {
                    public void response(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> s2,
                                         java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> s3)
                    {
                        test(s1.equals(s2.get("a")));
                        test(s1.equals(s3.get("a")));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpSMapCallback cb = new OpSMapCallback();
                i.begin_opSMap(smap1, cb);
                cb.check();
            }

            test.Ice.scope.Test.Inner.Inner2.C c1 = new test.Ice.scope.Test.Inner.Inner2.C(s1);
            {
                class OpCCallback extends test.Ice.scope.Test.Inner.Inner2.Callback_I_opC
                {
                    public void response(test.Ice.scope.Test.Inner.Inner2.C c2,
                                         test.Ice.scope.Test.Inner.Inner2.C c3)
                    {
                        test(s1.equals(c2.s));
                        test(s1.equals(c3.s));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpCCallback cb = new OpCCallback();
                i.begin_opC(c1, cb);
                cb.check();
            }

            test.Ice.scope.Test.Inner.Inner2.C[] cseq1 =
                new test.Ice.scope.Test.Inner.Inner2.C[]{ c1 };
            {
                class OpCSeqCallback extends test.Ice.scope.Test.Inner.Inner2.Callback_I_opCSeq
                {
                    public void response(test.Ice.scope.Test.Inner.Inner2.C[] c2,
                                         test.Ice.scope.Test.Inner.Inner2.C[] c3)
                    {
                        test(s1.equals(c2[0].s));
                        test(s1.equals(c3[0].s));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpCSeqCallback cb = new OpCSeqCallback();
                i.begin_opCSeq(cseq1, cb);
                cb.check();
            }

            java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> cmap1 =
                new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.C>();
            cmap1.put("a", c1);
            {
                class OpCMapCallback extends test.Ice.scope.Test.Inner.Inner2.Callback_I_opCMap
                {
                    public void response(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> c2,
                                         java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> c3)
                    {
                        test(s1.equals(c2.get("a").s));
                        test(s1.equals(c3.get("a").s));
                        _callback.called();
                    }

                    public void exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void check()
                    {
                        _callback.check();
                    }

                    private Callback _callback = new Callback();
                }
                OpCMapCallback cb = new OpCMapCallback();
                i.begin_opCMap(cmap1, cb);
                cb.check();
            }
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i1:" + helper.getTestEndpoint());
            test.Ice.scope.Test.IPrx i = test.Ice.scope.Test.IPrxHelper.checkedCast(obj);
            i.shutdown();
        }
    }
}
