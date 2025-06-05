// Copyright (c) ZeroC, Inc.

package test.Ice.ami;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.CommunicatorDestroyedException;
import com.zeroc.Ice.CompressBatch;
import com.zeroc.Ice.Connection;
import com.zeroc.Ice.ConnectionAbortedException;
import com.zeroc.Ice.ConnectionLostException;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.InvocationFuture;
import com.zeroc.Ice.NoEndpointException;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Util;

import test.TestHelper;
import test.Ice.ami.Test.PingReply;
import test.Ice.ami.Test.PingReplyPrx;
import test.Ice.ami.Test.TestIntfControllerPrx;
import test.Ice.ami.Test.TestIntfException;
import test.Ice.ami.Test.TestIntfPrx;

import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CancellationException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            new Throwable().printStackTrace();
            //
            // Exceptions raised by callbacks are swallowed by CompletableFuture.
            //
            throw new RuntimeException();
        }
    }

    public static class PingReplyI implements PingReply {
        @Override
        public void reply(Current current) {
            _received = true;
        }

        public boolean checkReceived() {
            return _received;
        }

        private boolean _received;
    }

    private static class Callback {
        Callback() {
            _called = false;
        }

        public synchronized void check() {
            while (!_called) {
                try {
                    wait();
                } catch (InterruptedException ex) {}
            }

            _called = false;
        }

        public synchronized void called() {
            assert (!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    static class SentCallback extends Callback {
        public void sent(boolean ss) {
            called();
        }
    }

    static class SentAsyncCallback extends Callback {
        SentAsyncCallback(long thread) {
            _thread = thread;
        }

        public void sent(boolean ss) {
            //
            // For whenSentAsync(), the callback is always invoked from an Executor thread.
            //
            test(_thread == Thread.currentThread().getId());
            called();
        }

        long _thread;
    }

    enum ThrowType {
        LocalException,
        OtherException
    }

    private static void throwEx(ThrowType t) {
        switch (t) {
            case LocalException:
            {
                throw new ObjectNotExistException();
            }
            case OtherException:
            {
                throw new RuntimeException();
            }
            default:
            {
                assert false;
                break;
            }
        }
    }

    public static void allTests(TestHelper helper, boolean collocated) {
        Communicator communicator = helper.communicator();
        final boolean bluetooth =
            communicator.getProperties().getIceProperty("Ice.Default.Protocol").indexOf("bt")
                == 0;
        PrintWriter out = helper.getWriter();

        String sref = "test:" + helper.getTestEndpoint(0);
        ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfPrx p = TestIntfPrx.uncheckedCast(obj);

        sref = "testController:" + helper.getTestEndpoint(1);
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfControllerPrx testController = TestIntfControllerPrx.uncheckedCast(obj);

        out.print("testing begin/end invocation... ");
        out.flush();
        {
            Map<String, String> ctx = new HashMap<>();

            test(p.ice_isAAsync("::Test::TestIntf").join());
            test(p.ice_isAAsync("::Test::TestIntf", ctx).join());

            p.ice_pingAsync().join();
            p.ice_pingAsync(ctx).join();

            test("::Test::TestIntf".equals(p.ice_idAsync().join()));
            test("::Test::TestIntf".equals(p.ice_idAsync(ctx).join()));

            test(p.ice_idsAsync().join().length == 2);
            test(p.ice_idsAsync(ctx).join().length == 2);

            if (!collocated) {
                test(p.ice_getConnectionAsync().join() != null);
            }

            p.opAsync().join();
            p.opAsync(ctx).join();

            test(p.opWithResultAsync().join() == 15);
            test(p.opWithResultAsync(ctx).join() == 15);

            p.opWithUEAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex != null && ex instanceof TestIntfException);
                    });
            p.opWithUEAsync(ctx)
                .whenComplete(
                    (result, ex) -> {
                        test(ex != null && ex instanceof TestIntfException);
                    });

            if (p.supportsFunctionalTests()) {
                test(p.opBoolAsync(true).join());

                test(p.opByteAsync((byte) 0xff).join() == (byte) 0xff);

                test(p.opShortAsync(Short.MIN_VALUE).join() == Short.MIN_VALUE);

                test(p.opIntAsync(Integer.MIN_VALUE).join() == Integer.MIN_VALUE);

                test(p.opLongAsync(Long.MIN_VALUE).join() == Long.MIN_VALUE);

                test(p.opFloatAsync(3.14f).join() == 3.14f);

                test(p.opDoubleAsync(1.1E10).join() == 1.1E10);
            }
        }
        out.println("ok");

        out.print("testing local exceptions... ");
        out.flush();
        {
            TestIntfPrx indirect = p.ice_adapterId("dummy");

            indirect.opAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex != null && ex instanceof NoEndpointException);
                    });

            indirect.ice_getConnectionAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex != null && ex instanceof NoEndpointException);
                    });

            //
            // Check that CommunicatorDestroyedException is raised directly.
            //
            if (p.ice_getConnection() != null) {
                InitializationData initData = new InitializationData();
                initData.properties = communicator.getProperties()._clone();
                Communicator ic = helper.initialize(initData);
                ObjectPrx o = ic.stringToProxy(p.toString());
                TestIntfPrx p2 = TestIntfPrx.uncheckedCast(o);
                ic.destroy();

                try {
                    p2.opAsync();
                    test(false);
                } catch (CommunicatorDestroyedException ex) {
                    // Expected.
                }
            }
        }
        out.println("ok");

        out.print("testing sent callback... ");
        out.flush();
        {
            final SentCallback cb = new SentCallback();

            {
                CompletableFuture<Boolean> r = p.ice_isAAsync("");
                Util.getInvocationFuture(r)
                    .whenSent(
                        (sentSynchronously, ex) -> {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        });
                cb.check();
            }

            {
                CompletableFuture<Void> r = p.ice_pingAsync();
                Util.getInvocationFuture(r)
                    .whenSent(
                        (sentSynchronously, ex) -> {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        });
                cb.check();
            }

            {
                CompletableFuture<String> r = p.ice_idAsync();
                Util.getInvocationFuture(r)
                    .whenSent(
                        (sentSynchronously, ex) -> {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        });
                cb.check();
            }

            {
                CompletableFuture<String[]> r = p.ice_idsAsync();
                Util.getInvocationFuture(r)
                    .whenSent(
                        (sentSynchronously, ex) -> {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        });
                cb.check();
            }
        }
        out.println("ok");

        //
        // Create an executor to use for dispatching completed futures.
        //
        ExecutorService executor =
            Executors.newSingleThreadExecutor();

        //
        // Determine the id of the executor's thread.
        //
        long executorThread = 0;
        try {
            executorThread =
                executor.submit(
                    () -> {
                        return Thread.currentThread().getId();
                    })
                    .get();
        } catch (Exception ex) {
            test(false);
        }

        out.print("testing sent async callback... ");
        out.flush();
        {
            final SentAsyncCallback cb = new SentAsyncCallback(executorThread);

            {
                CompletableFuture<Boolean> r = p.ice_isAAsync("");
                Util.getInvocationFuture(r)
                    .whenSentAsync(
                        (sentSynchronously, ex) -> {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        },
                        executor);
                cb.check();
            }

            {
                CompletableFuture<Void> r = p.ice_pingAsync();
                Util.getInvocationFuture(r)
                    .whenSentAsync(
                        (sentSynchronously, ex) -> {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        },
                        executor);
                cb.check();
            }

            {
                CompletableFuture<String> r = p.ice_idAsync();
                Util.getInvocationFuture(r)
                    .whenSentAsync(
                        (sentSynchronously, ex) -> {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        },
                        executor);
                cb.check();
            }

            {
                CompletableFuture<String[]> r = p.ice_idsAsync();
                Util.getInvocationFuture(r)
                    .whenSentAsync(
                        (sentSynchronously, ex) -> {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        },
                        executor);
                cb.check();
            }
        }
        out.println("ok");

        out.print("testing unexpected exceptions... ");
        out.flush();
        {
            TestIntfPrx q = TestIntfPrx.uncheckedCast(p.ice_adapterId("dummy"));
            ThrowType[] throwExType = {ThrowType.LocalException, ThrowType.OtherException};

            for (int i = 0; i < 2; i++) {
                final int idx = i;
                try {
                    p.opAsync()
                        .whenComplete(
                            (result, ex) -> {
                                throwEx(throwExType[idx]);
                            })
                        .get();
                } catch (Exception ex) {
                    test(ex instanceof ExecutionException);
                }

                try {
                    p.opAsync()
                        .whenCompleteAsync(
                            (result, ex) -> {
                                throwEx(throwExType[idx]);
                            })
                        .get();
                } catch (Exception ex) {
                    test(ex instanceof ExecutionException);
                }

                try {
                    q.opAsync()
                        .whenComplete(
                            (result, ex) -> {
                                throwEx(throwExType[idx]);
                            })
                        .get();
                } catch (Exception ex) {
                    test(ex instanceof ExecutionException);
                }

                try {
                    q.opAsync()
                        .whenCompleteAsync(
                            (result, ex) -> {
                                throwEx(throwExType[idx]);
                            })
                        .get();
                } catch (Exception ex) {
                    test(ex instanceof ExecutionException);
                }

                try {
                    Util.getInvocationFuture(p.opAsync())
                        .whenSent(
                            (sentSynchronously, ex) -> {
                                throwEx(throwExType[idx]);
                            })
                        .get();
                } catch (Exception ex) {
                    test(ex instanceof ExecutionException);
                }

                try {
                    Util.getInvocationFuture(p.opAsync())
                        .whenSentAsync(
                            (sentSynchronously, ex) -> {
                                throwEx(throwExType[idx]);
                            })
                        .get();
                } catch (Exception ex) {
                    test(ex instanceof ExecutionException);
                }
            }
        }
        out.println("ok");

        out.print("testing batch requests with proxy... ");
        out.flush();
        {
            {
                CompletableFuture<Void> r = p.ice_batchOneway().ice_flushBatchRequestsAsync();
                test(Util.getInvocationFuture(r).sentSynchronously());
                test(Util.getInvocationFuture(r).isSent());
                test(Util.getInvocationFuture(r).isDone());
            }

            {
                final SentCallback cb = new SentCallback();
                final SentAsyncCallback cbAsync = new SentAsyncCallback(executorThread);
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = p.ice_batchOneway();
                CompletableFuture<Void> bf = b1.opBatchAsync();
                test(bf.isDone());
                test(!Util.getInvocationFuture(bf).isSent());
                b1.opBatch();
                CompletableFuture<Void> r = b1.ice_flushBatchRequestsAsync();
                Util.getInvocationFuture(r)
                    .whenSent(
                        (sentSynchronously, ex) -> {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        });
                Util.getInvocationFuture(r)
                    .whenSentAsync(
                        (sentSynchronously, ex) -> {
                            test(ex == null);
                            cbAsync.sent(sentSynchronously);
                        },
                        executor);
                cb.check();
                cbAsync.check();
                test(Util.getInvocationFuture(r).isSent());
                Util.getInvocationFuture(r).waitForCompleted();
                test(r.isDone());
                test(p.waitForBatch(2));
            }

            if (p.ice_getConnection() != null && !bluetooth) {
                final SentCallback cb = new SentCallback();
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = p.ice_batchOneway();
                b1.opBatch();
                b1.ice_getConnection().close();
                CompletableFuture<Void> r = b1.ice_flushBatchRequestsAsync();
                Util.getInvocationFuture(r)
                    .whenSent(
                        (sentSynchronously, ex) -> {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        });
                cb.check();
                test(Util.getInvocationFuture(r).isSent());
                Util.getInvocationFuture(r).waitForCompleted();
                test(r.isDone());
                test(p.waitForBatch(1));
            }
        }
        out.println("ok");

        if (p.ice_getConnection() != null && !bluetooth) {
            out.print("testing batch requests with connection... ");
            out.flush();
            {
                {
                    final SentCallback cb = new SentCallback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 =
                        TestIntfPrx.uncheckedCast(
                            p.ice_getConnection().createProxy(p.ice_getIdentity()))
                            .ice_batchOneway();
                    b1.opBatch();
                    b1.opBatch();
                    CompletableFuture<Void> r =
                        b1.ice_getConnection()
                            .flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r)
                        .whenSent(
                            (sentSynchronously, ex) -> {
                                test(ex == null);
                                cb.sent(sentSynchronously);
                            });
                    cb.check();
                    test(Util.getInvocationFuture(r).isSent());
                    Util.getInvocationFuture(r).waitForCompleted();
                    test(r.isDone());
                    test(p.waitForBatch(2));
                }

                {
                    //
                    // Exception.
                    //
                    final Callback cb = new Callback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 =
                        TestIntfPrx.uncheckedCast(
                            p.ice_getConnection().createProxy(p.ice_getIdentity()))
                            .ice_batchOneway();
                    b1.opBatch();
                    b1.ice_getConnection().close();
                    CompletableFuture<Void> r =
                        b1.ice_getConnection()
                            .flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r)
                        .whenSent(
                            (sentSynchronously, ex) -> {
                                test(ex != null);
                                cb.called();
                            });
                    cb.check();
                    test(!Util.getInvocationFuture(r).isSent());
                    Util.getInvocationFuture(r).waitForCompleted();
                    test(r.isDone() && r.isCompletedExceptionally());
                    test(p.opBatchCount() == 0);
                }
            }
            out.println("ok");

            out.print("testing batch requests with communicator... ");
            out.flush();
            {
                {
                    //
                    // 1 connection.
                    //
                    final SentCallback cb = new SentCallback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 =
                        TestIntfPrx.uncheckedCast(
                            p.ice_getConnection().createProxy(p.ice_getIdentity()))
                            .ice_batchOneway();
                    b1.opBatch();
                    b1.opBatch();
                    CompletableFuture<Void> r =
                        communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r)
                        .whenSent(
                            (sentSynchronously, ex) -> {
                                test(ex == null);
                                cb.sent(sentSynchronously);
                            });
                    cb.check();
                    test(Util.getInvocationFuture(r).isSent());
                    Util.getInvocationFuture(r).waitForCompleted();
                    test(r.isDone());
                    test(p.waitForBatch(2));
                }

                {
                    //
                    // Exception - 1 connection.
                    //
                    final SentCallback cb = new SentCallback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 =
                        TestIntfPrx.uncheckedCast(
                            p.ice_getConnection().createProxy(p.ice_getIdentity()))
                            .ice_batchOneway();
                    b1.opBatch();
                    b1.ice_getConnection().close();
                    CompletableFuture<Void> r =
                        communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r)
                        .whenSent(
                            (sentSynchronously, ex) -> {
                                test(ex == null);
                                cb.sent(sentSynchronously);
                            });
                    cb.check();
                    test(Util.getInvocationFuture(r).isSent()); // Exceptions are ignored!
                    Util.getInvocationFuture(r).waitForCompleted();
                    test(r.isDone());
                    test(p.opBatchCount() == 0);
                }

                {
                    //
                    // 2 connections.
                    //
                    final SentCallback cb = new SentCallback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 =
                        TestIntfPrx.uncheckedCast(
                            p.ice_getConnection().createProxy(p.ice_getIdentity()))
                            .ice_batchOneway();
                    TestIntfPrx b2 =
                        TestIntfPrx.uncheckedCast(
                            p.ice_connectionId("2")
                                .ice_getConnection()
                                .createProxy(p.ice_getIdentity()))
                            .ice_batchOneway();
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b1.opBatch();
                    b2.opBatch();
                    b2.opBatch();
                    CompletableFuture<Void> r =
                        communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r)
                        .whenSent(
                            (sentSynchronously, ex) -> {
                                test(ex == null);
                                cb.sent(sentSynchronously);
                            });
                    cb.check();
                    test(Util.getInvocationFuture(r).isSent()); // Exceptions are ignored!
                    Util.getInvocationFuture(r).waitForCompleted();
                    test(r.isDone());
                    test(p.waitForBatch(4));
                }

                {
                    //
                    // Exception - 2 connections - 1 failure.
                    //
                    // All connections should be flushed even if there are failures on some
                    // connections. Exceptions should not be reported.
                    //
                    final SentCallback cb = new SentCallback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 =
                        TestIntfPrx.uncheckedCast(
                            p.ice_getConnection().createProxy(p.ice_getIdentity()))
                            .ice_batchOneway();
                    TestIntfPrx b2 =
                        TestIntfPrx.uncheckedCast(
                            p.ice_connectionId("2")
                                .ice_getConnection()
                                .createProxy(p.ice_getIdentity()))
                            .ice_batchOneway();
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b2.opBatch();
                    b1.ice_getConnection().close();
                    CompletableFuture<Void> r =
                        communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r)
                        .whenSent(
                            (sentSynchronously, ex) -> {
                                test(ex == null);
                                cb.sent(sentSynchronously);
                            });
                    cb.check();
                    test(Util.getInvocationFuture(r).isSent()); // Exceptions are ignored!
                    Util.getInvocationFuture(r).waitForCompleted();
                    test(r.isDone());
                    test(p.waitForBatch(1));
                }

                {
                    //
                    // Exception - 2 connections - 2 failures.
                    //
                    // The sent callback should be invoked even if all connections fail.
                    //
                    final SentCallback cb = new SentCallback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 =
                        TestIntfPrx.uncheckedCast(
                            p.ice_getConnection().createProxy(p.ice_getIdentity()))
                            .ice_batchOneway();
                    TestIntfPrx b2 =
                        TestIntfPrx.uncheckedCast(
                            p.ice_connectionId("2")
                                .ice_getConnection()
                                .createProxy(p.ice_getIdentity()))
                            .ice_batchOneway();
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b2.opBatch();
                    b1.ice_getConnection().close();
                    b2.ice_getConnection().close();
                    CompletableFuture<Void> r =
                        communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r)
                        .whenSent(
                            (sentSynchronously, ex) -> {
                                test(ex == null);
                                cb.sent(sentSynchronously);
                            });
                    cb.check();
                    test(Util.getInvocationFuture(r).isSent()); // Exceptions are ignored!
                    Util.getInvocationFuture(r).waitForCompleted();
                    test(r.isDone());
                    test(p.opBatchCount() == 0);
                }
            }
            out.println("ok");
        }

        out.print("testing future operations... ");
        out.flush();
        {
            {
                TestIntfPrx indirect = p.ice_adapterId("dummy");
                CompletableFuture<Void> r = indirect.opAsync();
                Util.getInvocationFuture(r).waitForCompleted();
                try {
                    r.join();
                    test(false);
                } catch (CompletionException ex) {
                    test(ex.getCause() instanceof NoEndpointException);
                }

                testController.holdAdapter();
                InvocationFuture<Void> r1;
                InvocationFuture<Void> r2;
                var results = new ArrayList<InvocationFuture<Void>>();
                try {
                    r1 = Util.getInvocationFuture(p.opAsync());
                    byte[] seq = new byte[10024];
                    while (true) {
                        r2 = Util.getInvocationFuture(p.opWithPayloadAsync(seq));
                        results.add(r2);
                        if (!r2.sentSynchronously()) {
                            break;
                        }
                    }

                    if (p.ice_getConnection() != null) {
                        test(
                            r1.sentSynchronously() && r1.isSent() && !r1.isDone()
                                || !r1.sentSynchronously() && !r1.isDone());

                        test(!r2.sentSynchronously() && !r2.isDone());
                    }
                } finally {
                    testController.resumeAdapter();
                }

                r1.waitForSent();
                test(r1.isSent());

                r2.waitForSent();
                test(r2.isSent());

                r1.waitForCompleted();
                test(r1.isDone());

                r2.waitForCompleted();
                test(r2.isDone());

                test("op".equals(r1.getOperation()));
                test("opWithPayload".equals(r2.getOperation()));

                CompletableFuture.allOf(results.toArray(new CompletableFuture[0])).join();
            }

            {
                {
                    //
                    // Twoway
                    //
                    InvocationFuture<Void> r = Util.getInvocationFuture(p.ice_pingAsync());
                    test("ice_ping".equals(r.getOperation()));
                    test(r.getConnection() == null); // Expected
                    test(r.getCommunicator() == communicator);
                    test(r.getProxy() == p);
                    r.join();
                }

                {
                    //
                    // Oneway
                    //
                    TestIntfPrx p2 = p.ice_oneway();
                    InvocationFuture<Void> r = Util.getInvocationFuture(p2.ice_pingAsync());
                    test("ice_ping".equals(r.getOperation()));
                    test(r.getConnection() == null); // Expected
                    test(r.getCommunicator() == communicator);
                    test(r.getProxy() == p2);
                }

                {
                    //
                    // Batch request via proxy
                    //
                    TestIntfPrx p2 = p.ice_batchOneway();
                    p2.ice_ping();
                    InvocationFuture<Void> r =
                        Util.getInvocationFuture(p2.ice_flushBatchRequestsAsync());
                    test(r.getConnection() == null); // Expected
                    test(r.getCommunicator() == communicator);
                    test(r.getProxy() == p2);
                    r.join();
                }

                if (p.ice_getConnection() != null) {
                    //
                    // Batch request via connection
                    //
                    Connection con = p.ice_getConnection();
                    TestIntfPrx p2 = p.ice_batchOneway();
                    p2.ice_ping();
                    InvocationFuture<Void> r =
                        Util.getInvocationFuture(
                            con.flushBatchRequestsAsync(CompressBatch.BasedOnProxy));
                    test(r.getConnection() == con);
                    test(r.getCommunicator() == communicator);
                    test(r.getProxy() == null); // Expected
                    r.join();

                    //
                    // Batch request via communicator
                    //
                    p2 = p.ice_batchOneway();
                    p2.ice_ping();
                    r =
                        Util.getInvocationFuture(
                            communicator.flushBatchRequestsAsync(
                                CompressBatch.BasedOnProxy));
                    test(r.getConnection() == null); // Expected
                    test(r.getCommunicator() == communicator);
                    test(r.getProxy() == null); // Expected
                    r.join();
                }
            }

            if (p.ice_getConnection() != null) {
                InvocationFuture<Void> r1 = null;
                InvocationFuture<String> r2 = null;

                testController.holdAdapter();
                var results = new ArrayList<InvocationFuture<Void>>();
                try {
                    InvocationFuture<Void> r = null;
                    byte[] seq = new byte[10024];
                    for (int i = 0; i < 200; i++) // 2MB
                        {
                            r = Util.getInvocationFuture(p.opWithPayloadAsync(seq));
                            results.add(r);
                        }

                    test(!r.isSent());

                    r1 = Util.getInvocationFuture(p.ice_pingAsync());
                    r2 = Util.getInvocationFuture(p.ice_idAsync());
                    r1.cancel(false);
                    r2.cancel(false);
                    try {
                        r1.join();
                        test(false);
                    } catch (CancellationException ex) {}
                    try {
                        r2.join();
                        test(false);
                    } catch (CancellationException ex) {}
                } finally {
                    testController.resumeAdapter();
                }

                CompletableFuture.allOf(results.toArray(new CompletableFuture[0])).join();

                p.ice_ping();
                // test(!r1.isSent() && r1.isDone());
                test(!r1.isSent());
                test(r1.isDone());
                test(!r2.isSent() && r2.isDone());

                testController.holdAdapter();
                try {
                    r1 = Util.getInvocationFuture(p.opAsync());
                    r2 = Util.getInvocationFuture(p.ice_idAsync());
                    r1.waitForSent();
                    r2.waitForSent();
                    r1.cancel(false);
                    r2.cancel(false);
                    try {
                        r1.join();
                        test(false);
                    } catch (CancellationException ex) {}
                    try {
                        r2.join();
                        test(false);
                    } catch (CancellationException ex) {}
                } finally {
                    testController.resumeAdapter();
                }
            }
        }
        out.println("ok");

        if (p.ice_getConnection() != null && !bluetooth) {
            out.print("testing connection close... ");
            out.flush();
            {
                // Local case: begin a request, close the connection gracefully, and make sure it
                // waits for the request to complete.
                Connection con = p.ice_getConnection();
                Callback cb = new Callback();
                con.setCloseCallback(c -> cb.called());
                CompletableFuture<Void> f = p.sleepAsync(100);
                con.close(); // Blocks until the request completes.
                try {
                    f.join(); // Should complete successfully.
                } catch (Throwable ex) {
                    test(false);
                }
                cb.check();
            }
            {
                //
                // Remote case.
                //
                byte[] seq = new byte[1024 * 10];

                // Send multiple opWithPayload, followed by a close and followed by multiple
                // opWithPayload. The goal is to make sure that none of the opWithPayload fail even
                // if the server closes the connection gracefully in between.
                int maxQueue = 2;
                boolean done = false;
                while (!done && maxQueue < 50) {
                    done = true;
                    p.ice_ping();
                    var futures = new ArrayList<CompletableFuture>();
                    for (int i = 0; i < maxQueue; i++) {
                        futures.add(Util.getInvocationFuture(p.opWithPayloadAsync(seq)));
                    }

                    var closeFuture = Util.getInvocationFuture(p.closeConnectionAsync());
                    futures.add(closeFuture);

                    if (!closeFuture.isSent()) {
                        for (int i = 0; i < maxQueue; i++) {
                            InvocationFuture<Void> r =
                                Util.getInvocationFuture(p.opWithPayloadAsync(seq));
                            futures.add(r);
                            if (r.isSent()) {
                                done = false;
                                maxQueue *= 2;
                                break;
                            }
                        }
                    } else {
                        maxQueue *= 2;
                        done = false;
                    }
                    CompletableFuture.allOf(futures.toArray(CompletableFuture[]::new)).join();

                    // Wait until the connection is closed.
                    p.ice_getCachedConnection().close();
                }
            }
            out.println("ok");

            out.print("testing connection abort... ");
            out.flush();
            {
                //
                // Local case: start an operation and then close the connection forcefully on the
                // client side. There will be no retry and we expect the invocation to fail with
                // ConnectionAbortedException.
                //
                p.ice_ping();
                Connection con = p.ice_getConnection();
                CompletableFuture<Void> f = p.startDispatchAsync();
                Util.getInvocationFuture(f)
                    .waitForSent(); // Ensure the request was sent before we close the
                // connection
                con.abort();
                try {
                    f.join();
                    test(false);
                } catch (CompletionException ex) {
                    test(ex.getCause() instanceof ConnectionAbortedException);
                    test(
                        ((ConnectionAbortedException) ex.getCause())
                            .closedByApplication);
                } catch (Throwable ex) {
                    test(false);
                }
                p.finishDispatch();

                //
                // Remote case: the server closes the connection forcefully. This causes the request
                // to fail
                // with a ConnectionLostException. Since the close() operation is not idempotent,
                // the client will not retry.
                //
                try {
                    p.abortConnection();
                    test(false);
                } catch (ConnectionLostException ex) {
                    // Expected.
                }
            }
            out.println("ok");
        }

        out.print("testing ice_executor... ");
        out.flush();
        {
            p.ice_pingAsync()
                .whenCompleteAsync(
                    (result, ex) -> {
                        test(
                            Thread.currentThread()
                                .getName()
                                .indexOf("Ice.ThreadPool.Client")
                                == -1);
                    })
                .join();

            p.ice_pingAsync()
                .whenCompleteAsync(
                    (result, ex) -> {
                        test(
                            Thread.currentThread()
                                .getName()
                                .indexOf("Ice.ThreadPool.Client")
                                != -1);
                    },
                    p.ice_executor())
                .join();
        }
        out.println("ok");

        if (!collocated) {
            out.print("testing bi-dir... ");
            ObjectAdapter adapter = communicator.createObjectAdapter("");
            PingReplyI replyI = new PingReplyI();
            PingReplyPrx reply = PingReplyPrx.uncheckedCast(adapter.addWithUUID(replyI));

            var context = Map.of("ONE", "");
            p.pingBiDir(reply, context);

            p.ice_getConnection().setAdapter(adapter);
            p.pingBiDir(reply);
            test(replyI.checkReceived());
            adapter.destroy();
            out.println("ok");
        }

        out.print("testing result struct... ");
        out.flush();
        {
            var q =
                test.Ice.ami.Test.Outer.Inner.TestIntfPrx.createProxy(
                    communicator, "test2:" + helper.getTestEndpoint(0));

            q.opAsync(1)
                .whenComplete(
                    (result, ex) -> {
                        test(result.returnValue == 1);
                        test(result.j == 1);
                        test(ex == null);
                    });
        }
        out.println("ok");

        if (p.ice_getConnection() != null && p.supportsBackPressureTests()) {
            out.print("testing back pressure... ");
            out.flush();
            try {
                // Keep the 3 server thread pool threads busy.
                CompletableFuture<Void> sleep1Future = p.sleepAsync(1000);
                CompletableFuture<Void> sleep2Future = p.sleepAsync(1000);
                CompletableFuture<Void> sleep3Future = p.sleepAsync(1000);
                TestIntfPrx onewayProxy = p.ice_oneway();

                // Sending should block because the TCP send/receive buffer size on the server is
                // set to 50KB.
                CompletableFuture<Void> future =
                    onewayProxy.opWithPayloadAsync(new byte[768 * 1024]);
                boolean timeout = false;
                try {
                    future.get(200, TimeUnit.MILLISECONDS);
                } catch (TimeoutException ex) {
                    timeout = true;
                }
                test(timeout);
                test(!sleep1Future.isDone());

                sleep1Future.get();

                sleep2Future.get();
                sleep3Future.get();
            } catch (Exception ex) {
                out.println(ex.toString());
                test(false);
            }
            out.println("ok");
        }

        executor.shutdown();

        p.shutdown();
    }

    private AllTests() {}
}
