// Copyright (c) ZeroC, Inc.

package test.Ice.exceptions;

import com.zeroc.Ice.AlreadyRegisteredException;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ConnectionLostException;
import com.zeroc.Ice.ConnectionRefusedException;
import com.zeroc.Ice.DispatchException;
import com.zeroc.Ice.FacetNotExistException;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationException;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.MarshalException;
import com.zeroc.Ice.NotRegisteredException;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.OperationNotExistException;
import com.zeroc.Ice.ReplyStatus;
import com.zeroc.Ice.ServantLocator;
import com.zeroc.Ice.SocketException;
import com.zeroc.Ice.UnknownException;
import com.zeroc.Ice.UnknownLocalException;
import com.zeroc.Ice.UnknownUserException;
import com.zeroc.Ice.Util;

import test.Ice.exceptions.Test.A;
import test.Ice.exceptions.Test.B;
import test.Ice.exceptions.Test.C;
import test.Ice.exceptions.Test.D;
import test.Ice.exceptions.Test.ThrowerPrx;
import test.Ice.exceptions.Test.WrongOperationPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.concurrent.CompletionException;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static ThrowerPrx allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        final boolean bluetooth =
            communicator.getProperties().getIceProperty("Ice.Default.Protocol").indexOf("bt")
                == 0;
        PrintWriter out = helper.getWriter();

        {
            out.print("testing object adapter registration exceptions... ");
            ObjectAdapter first;
            try {
                first = communicator.createObjectAdapter("TestAdapter0");
            } catch (InitializationException ex) {
                // Expected
            }

            communicator.getProperties().setProperty("TestAdapter0.Endpoints", "tcp -h *");
            first = communicator.createObjectAdapter("TestAdapter0");
            try {
                communicator.createObjectAdapter("TestAdapter0");
                test(false);
            } catch (AlreadyRegisteredException ex) {
                // Expected
            }

            try {
                communicator.createObjectAdapterWithEndpoints(
                    "TestAdapter0", "ssl -h foo -p 12011");
                test(false);
            } catch (AlreadyRegisteredException ex) {
                // Expected
            }
            first.deactivate();
            out.println("ok");
        }

        {
            out.print("testing servant registration exceptions... ");
            communicator.getProperties().setProperty("TestAdapter1.Endpoints", "tcp -h *");
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter1");
            Object obj = new EmptyI();
            adapter.add(obj, Util.stringToIdentity("x"));
            try {
                adapter.add(obj, Util.stringToIdentity("x"));
                test(false);
            } catch (AlreadyRegisteredException ex) {}

            try {
                adapter.add(obj, Util.stringToIdentity(""));
                test(false);
            } catch (IllegalArgumentException ex) {}
            try {
                adapter.add(null, Util.stringToIdentity("x"));
                test(false);
            } catch (IllegalArgumentException ex) {}

            adapter.remove(Util.stringToIdentity("x"));
            try {
                adapter.remove(Util.stringToIdentity("x"));
                test(false);
            } catch (NotRegisteredException ex) {}
            adapter.deactivate();
            out.println("ok");
        }

        {
            out.print("testing servant locator registration exceptions... ");
            communicator.getProperties().setProperty("TestAdapter2.Endpoints", "tcp -h *");
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter2");
            ServantLocator loc = new ServantLocatorI();
            adapter.addServantLocator(loc, "x");
            try {
                adapter.addServantLocator(loc, "x");
                test(false);
            } catch (AlreadyRegisteredException ex) {}

            adapter.deactivate();
            out.println("ok");
        }

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "thrower:" + helper.getTestEndpoint(0);
        ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        ThrowerPrx thrower = ThrowerPrx.checkedCast(base);
        test(thrower != null);
        test(thrower.equals(base));
        out.println("ok");

        out.print("catching exact types... ");
        out.flush();

        try {
            thrower.throwAasA(1);
            test(false);
        } catch (A ex) {
            test(ex.aMem == 1);
        } catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            thrower.throwAorDasAorD(1);
            test(false);
        } catch (A ex) {
            test(ex.aMem == 1);
        } catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            thrower.throwAorDasAorD(-1);
            test(false);
        } catch (D ex) {
            test(ex.dMem == -1);
        } catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            thrower.throwBasB(1, 2);
            test(false);
        } catch (B ex) {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        } catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            thrower.throwCasC(1, 2, 3);
            test(false);
        } catch (C ex) {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        } catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        out.println("ok");

        out.print("catching base types... ");
        out.flush();

        try {
            thrower.throwBasB(1, 2);
            test(false);
        } catch (A ex) {
            test(ex.aMem == 1);
        } catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            thrower.throwCasC(1, 2, 3);
            test(false);
        } catch (B ex) {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        } catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        out.println("ok");

        out.print("catching derived types... ");
        out.flush();

        try {
            thrower.throwBasA(1, 2);
            test(false);
        } catch (B ex) {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        } catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            thrower.throwCasA(1, 2, 3);
            test(false);
        } catch (C ex) {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        } catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            thrower.throwCasB(1, 2, 3);
            test(false);
        } catch (C ex) {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        } catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        out.println("ok");

        if (thrower.supportsUndeclaredExceptions()) {
            out.print("catching unknown user exception... ");
            out.flush();

            try {
                thrower.throwUndeclaredA(1);
                test(false);
            } catch (UnknownUserException ex) {} catch (Throwable ex) {
                ex.printStackTrace();
                test(false);
            }

            try {
                thrower.throwUndeclaredB(1, 2);
                test(false);
            } catch (UnknownUserException ex) {} catch (Throwable ex) {
                ex.printStackTrace();
                test(false);
            }

            try {
                thrower.throwUndeclaredC(1, 2, 3);
                test(false);
            } catch (UnknownUserException ex) {} catch (Throwable ex) {
                ex.printStackTrace();
                test(false);
            }

            out.println("ok");
        }

        if (thrower.supportsAssertException()) {
            out.print("testing assert in the server... ");
            out.flush();

            try {
                thrower.throwAssertException();
                test(false);
            } catch (ConnectionLostException ex) {} catch (UnknownException ex) {} catch (Throwable ex) {
                ex.printStackTrace();
                test(false);
            }

            out.println("ok");
        }

        if (thrower.ice_getConnection() != null && !bluetooth) {
            out.print("testing memory limit marshal exception... ");
            out.flush();
            try {
                thrower.throwMemoryLimitException(null);
                test(false);
            } catch (MarshalException ex) {
                test(ex.getMessage().contains("exceeds the maximum allowed"));
            } catch (Throwable ex) {
                ex.printStackTrace();
                test(false);
            }

            try {
                thrower.throwMemoryLimitException(new byte[20 * 1024]); // 20KB
                test(false);
            } catch (ConnectionLostException ex) {} catch (UnknownLocalException ex) {
                // Expected with JS bidir server
            } catch (SocketException ex) {
                // This can be raised if the connection is closed during the client's call to
                // write().
            } catch (Throwable ex) {
                ex.printStackTrace();
                test(false);
            }

            try {
                var thrower2 =
                    ThrowerPrx.createProxy(
                        communicator, "thrower:" + helper.getTestEndpoint(1));
                try {
                    thrower2.throwMemoryLimitException(
                        new byte[2 * 1024 * 1024]); // 2MB (no limits)
                } catch (MarshalException ex) {
                    test(ex.getMessage().contains("exceeds the maximum allowed"));
                }
                var thrower3 =
                    ThrowerPrx.createProxy(
                        communicator, "thrower:" + helper.getTestEndpoint(2));
                try {
                    thrower3.throwMemoryLimitException(new byte[1024]); // 1KB limit
                    test(false);
                } catch (ConnectionLostException ex) {}
            } catch (ConnectionRefusedException ex) {
                // Expected with JS bidir server
            }

            out.println("ok");
        }

        out.print("catching object not exist exception... ");
        out.flush();

        {
            Identity id = Util.stringToIdentity("does not exist");
            try {
                ThrowerPrx thrower2 = ThrowerPrx.uncheckedCast(thrower.ice_identity(id));
                thrower2.ice_ping();
                test(false);
            } catch (ObjectNotExistException ex) {
                test(ex.id.equals(id));
            } catch (Throwable ex) {
                ex.printStackTrace();
                test(false);
            }
        }

        out.println("ok");

        out.print("catching facet not exist exception... ");
        out.flush();

        try {
            ThrowerPrx thrower2 = ThrowerPrx.uncheckedCast(thrower, "no such facet");
            try {
                thrower2.ice_ping();
                test(false);
            } catch (FacetNotExistException ex) {
                test("no such facet".equals(ex.facet));
            }
        } catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        out.println("ok");

        out.print("catching operation not exist exception... ");
        out.flush();

        try {
            WrongOperationPrx thrower2 = WrongOperationPrx.uncheckedCast(thrower);
            thrower2.noSuchOperation();
            test(false);
        } catch (OperationNotExistException ex) {
            test("noSuchOperation".equals(ex.operation));
        } catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        out.println("ok");

        out.print("catching unknown local exception... ");
        out.flush();

        try {
            thrower.throwLocalException();
            test(false);
        } catch (UnknownLocalException ex) {} catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            thrower.throwLocalExceptionIdempotent();
            test(false);
        } catch (UnknownLocalException ex) {} catch (OperationNotExistException ex) {} catch (Throwable ex) {
            ex.printStackTrace();
            test(false);
        }

        out.println("ok");

        out.print("catching unknown non-Ice exception... ");
        out.flush();

        try {
            thrower.throwNonIceException();
            test(false);
        } catch (UnknownException ex) {} catch (Throwable ex) {
            out.println(ex);
            test(false);
        }

        out.println("ok");

        out.print("catching dispatch exception... ");
        out.flush();

        try {
            thrower.throwDispatchException((byte) ReplyStatus.OperationNotExist.value());
            test(false);
        } catch (OperationNotExistException ex) {
            // remapped as expected
            final String expected = "Dispatch failed with OperationNotExist "
                + "{ id = 'thrower', facet = '', operation = 'throwDispatchException' }";
            test(expected.equals(ex.getMessage()));
        }

        try {
            thrower.throwDispatchException((byte) ReplyStatus.Unauthorized.value());
            test(false);
        } catch (DispatchException ex) {
            if (ex.replyStatus == ReplyStatus.Unauthorized.value()) {
                test(
                    "The dispatch failed with reply status Unauthorized."
                        .equals(ex.getMessage())
                        || "The dispatch failed with reply status unauthorized."
                        .equals(ex.getMessage())); // for Swift
            } else {
                test(false);
            }
        }

        try {
            thrower.throwDispatchException((byte) 212);
            test(false);
        } catch (DispatchException ex) {
            if (ex.replyStatus == 212) {
                test("The dispatch failed with reply status 212.".equals(ex.getMessage()));
            } else {
                test(false);
            }
        }

        out.println("ok");

        out.print("testing asynchronous exceptions... ");
        out.flush();

        try {
            thrower.throwAfterResponse();
        } catch (LocalException ex) {
            test(false);
        } catch (Throwable ex) {
            out.println(ex);
            test(false);
        }

        try {
            thrower.throwAfterException();
            test(false);
        } catch (A ex) {} catch (LocalException ex) {
            test(false);
        } catch (Throwable ex) {
            out.println(ex);
            test(false);
        }

        out.println("ok");

        out.print("catching exact types with AMI mapping... ");
        out.flush();

        try {
            thrower.throwAasAAsync(1).join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof A);
            test(((A) ex.getCause()).aMem == 1);
        }

        try {
            thrower.throwAorDasAorDAsync(1).join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof A);
            test(((A) ex.getCause()).aMem == 1);
        }

        try {
            thrower.throwAorDasAorDAsync(-1).join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof D);
            test(((D) ex.getCause()).dMem == -1);
        }

        try {
            thrower.throwBasBAsync(1, 2).join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof B);
            test(((B) ex.getCause()).aMem == 1);
            test(((B) ex.getCause()).bMem == 2);
        }

        try {
            thrower.throwCasCAsync(1, 2, 3).join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof C);
            test(((C) ex.getCause()).aMem == 1);
            test(((C) ex.getCause()).bMem == 2);
            test(((C) ex.getCause()).cMem == 3);
        }

        out.println("ok");

        out.print("catching derived types with AMI mapping... ");
        out.flush();

        try {
            thrower.throwBasAAsync(1, 2).join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof B);
            test(((B) ex.getCause()).aMem == 1);
            test(((B) ex.getCause()).bMem == 2);
        }

        try {
            thrower.throwCasAAsync(1, 2, 3).join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof C);
            test(((C) ex.getCause()).aMem == 1);
            test(((C) ex.getCause()).bMem == 2);
            test(((C) ex.getCause()).cMem == 3);
        }

        try {
            thrower.throwCasBAsync(1, 2, 3).join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof C);
            test(((C) ex.getCause()).aMem == 1);
            test(((C) ex.getCause()).bMem == 2);
            test(((C) ex.getCause()).cMem == 3);
        }

        out.println("ok");

        if (thrower.supportsUndeclaredExceptions()) {
            out.print("catching unknown user exception with AMI mapping... ");
            out.flush();

            try {
                thrower.throwUndeclaredAAsync(1).join();
                test(false);
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof UnknownUserException);
            }

            try {
                thrower.throwUndeclaredBAsync(1, 2).join();
                test(false);
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof UnknownUserException);
            }

            try {
                thrower.throwUndeclaredCAsync(1, 2, 3).join();
                test(false);
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof UnknownUserException);
            }

            out.println("ok");
        }

        if (thrower.supportsAssertException()) {
            out.print("catching assert in the server with AMI mapping... ");
            out.flush();

            try {
                thrower.throwAssertExceptionAsync().join();
                test(false);
            } catch (CompletionException ex) {
                test(
                    ex.getCause() instanceof ConnectionLostException
                        || ex.getCause() instanceof UnknownException);
            }

            out.println("ok");
        }

        out.print("catching object not exist exception with AMI mapping... ");
        out.flush();

        {
            Identity id = Util.stringToIdentity("does not exist");
            ThrowerPrx thrower2 = ThrowerPrx.uncheckedCast(thrower.ice_identity(id));
            try {
                thrower2.throwAasAAsync(1).join();
                test(false);
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof ObjectNotExistException);
                test(
                    "does not exist".equals(((ObjectNotExistException) ex.getCause())
                        .id.name));
            }
        }

        out.println("ok");

        out.print("catching facet not exist exception with AMI mapping... ");
        out.flush();

        {
            ThrowerPrx thrower2 = ThrowerPrx.uncheckedCast(thrower, "no such facet");
            try {
                thrower2.throwAasAAsync(1).join();
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof FacetNotExistException);
                test(
                    "no such facet".equals(((FacetNotExistException) ex.getCause())
                        .facet));
            }
        }

        out.println("ok");

        out.print("catching operation not exist exception with AMI mapping... ");
        out.flush();

        {
            WrongOperationPrx thrower2 = WrongOperationPrx.uncheckedCast(thrower);
            try {
                thrower2.noSuchOperationAsync().join();
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof OperationNotExistException);
                test(
                    "noSuchOperation".equals(((OperationNotExistException) ex.getCause())
                        .operation));
            }
        }

        out.println("ok");

        out.print("catching unknown local exception with AMI mapping... ");
        out.flush();

        try {
            thrower.throwLocalExceptionAsync().join();
            test(false);
        } catch (CompletionException ex) {
            test(
                ex.getCause() instanceof UnknownLocalException
                    || ex.getCause() instanceof OperationNotExistException);
        }

        try {
            thrower.throwLocalExceptionIdempotentAsync().join();
            test(false);
        } catch (CompletionException ex) {
            test(
                ex.getCause() instanceof UnknownLocalException
                    || ex.getCause() instanceof OperationNotExistException);
        }

        out.println("ok");

        out.print("catching unknown non-Ice exception with AMI mapping... ");
        out.flush();

        try {
            thrower.throwNonIceExceptionAsync().join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof UnknownException);
        }

        out.println("ok");

        out.print("catching dispatch exception with AMI mapping... ");
        out.flush();

        try {
            thrower.throwDispatchExceptionAsync((byte) ReplyStatus.OperationNotExist.value())
                .join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof OperationNotExistException);
        }

        try {
            thrower.throwDispatchExceptionAsync((byte) ReplyStatus.Unauthorized.value()).join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof DispatchException);
        }

        try {
            thrower.throwDispatchExceptionAsync((byte) 212).join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof DispatchException);
        }

        out.println("ok");

        return thrower;
    }

    private AllTests() {}
}
