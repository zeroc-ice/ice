// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.exceptions;

import java.io.PrintWriter;
import java.util.concurrent.CompletionException;

import test.Ice.exceptions.Test.A;
import test.Ice.exceptions.Test.B;
import test.Ice.exceptions.Test.C;
import test.Ice.exceptions.Test.D;
import test.Ice.exceptions.Test.ThrowerPrx;
import test.Ice.exceptions.Test.WrongOperationPrx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static ThrowerPrx allTests(test.Util.Application app)
    {
        com.zeroc.Ice.Communicator communicator=app.communicator();
        PrintWriter out = app.getWriter();
        {
            out.print("testing object adapter registration exceptions... ");
            com.zeroc.Ice.ObjectAdapter first;
            try
            {
                first = communicator.createObjectAdapter("TestAdapter0");
            }
            catch(com.zeroc.Ice.InitializationException ex)
            {
                // Expected
            }

            communicator.getProperties().setProperty("TestAdapter0.Endpoints", "tcp -h *");
            first = communicator.createObjectAdapter("TestAdapter0");
            try
            {
                communicator.createObjectAdapter("TestAdapter0");
                test(false);
            }
            catch(com.zeroc.Ice.AlreadyRegisteredException ex)
            {
                // Expected
            }

            try
            {
                communicator.createObjectAdapterWithEndpoints("TestAdapter0", "ssl -h foo -p 12011");
                test(false);
            }
            catch(com.zeroc.Ice.AlreadyRegisteredException ex)
            {
                // Expected
            }
            first.deactivate();
            out.println("ok");
        }

        {
            out.print("testing servant registration exceptions... ");
            communicator.getProperties().setProperty("TestAdapter1.Endpoints", "tcp -h *");
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter1");
            com.zeroc.Ice.Object obj = new EmptyI();
            adapter.add(obj, com.zeroc.Ice.Util.stringToIdentity("x"));
            try
            {
                adapter.add(obj, com.zeroc.Ice.Util.stringToIdentity("x"));
                test(false);
            }
            catch(com.zeroc.Ice.AlreadyRegisteredException ex)
            {
            }

            try
            {
                adapter.add(obj, com.zeroc.Ice.Util.stringToIdentity(""));
                test(false);
            }
            catch(com.zeroc.Ice.IllegalIdentityException ex)
            {
                test(ex.id.name.equals(""));
            }
            try
            {
                adapter.add(null, com.zeroc.Ice.Util.stringToIdentity("x"));
                test(false);
            }
            catch(com.zeroc.Ice.IllegalServantException ex)
            {
            }

            adapter.remove(com.zeroc.Ice.Util.stringToIdentity("x"));
            try
            {
                adapter.remove(com.zeroc.Ice.Util.stringToIdentity("x"));
                test(false);
            }
            catch(com.zeroc.Ice.NotRegisteredException ex)
            {
            }
            adapter.deactivate();
            out.println("ok");
        }

        {
            out.print("testing servant locator registration exceptions... ");
            communicator.getProperties().setProperty("TestAdapter2.Endpoints", "tcp -h *");
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter2");
            com.zeroc.Ice.ServantLocator loc = new ServantLocatorI();
            adapter.addServantLocator(loc, "x");
            try
            {
                adapter.addServantLocator(loc, "x");
                test(false);
            }
            catch(com.zeroc.Ice.AlreadyRegisteredException ex)
            {
            }

            adapter.deactivate();
            out.println("ok");
        }

        {
            out.print("testing value factory registration exception... ");
            com.zeroc.Ice.ValueFactory of = new ValueFactoryI();
            communicator.getValueFactoryManager().add(of, "::x");
            try
            {
                communicator.getValueFactoryManager().add(of, "::x");
                test(false);
            }
            catch(com.zeroc.Ice.AlreadyRegisteredException ex)
            {
            }
            out.println("ok");
        }

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "thrower:" + app.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
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

        try
        {
            thrower.throwAasA(1);
            test(false);
        }
        catch(A ex)
        {
            test(ex.aMem == 1);
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        try
        {
            thrower.throwAorDasAorD(1);
            test(false);
        }
        catch(A ex)
        {
            test(ex.aMem == 1);
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        try
        {
            thrower.throwAorDasAorD(-1);
            test(false);
        }
        catch(D ex)
        {
            test(ex.dMem == -1);
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        try
        {
            thrower.throwBasB(1, 2);
            test(false);
        }
        catch(B ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        try
        {
            thrower.throwCasC(1, 2, 3);
            test(false);
        }
        catch(C ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        out.println("ok");

        out.print("catching base types... ");
        out.flush();

        try
        {
            thrower.throwBasB(1, 2);
            test(false);
        }
        catch(A ex)
        {
            test(ex.aMem == 1);
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        try
        {
            thrower.throwCasC(1, 2, 3);
            test(false);
        }
        catch(B ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        out.println("ok");

        out.print("catching derived types... ");
        out.flush();

        try
        {
            thrower.throwBasA(1, 2);
            test(false);
        }
        catch(B ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        try
        {
            thrower.throwCasA(1, 2, 3);
            test(false);
        }
        catch(C ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        try
        {
            thrower.throwCasB(1, 2, 3);
            test(false);
        }
        catch(C ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        out.println("ok");

        if(thrower.supportsUndeclaredExceptions())
        {
            out.print("catching unknown user exception... ");
            out.flush();

            try
            {
                thrower.throwUndeclaredA(1);
                test(false);
            }
            catch(com.zeroc.Ice.UnknownUserException ex)
            {
            }
            catch(Throwable ex)
            {
                ex.printStackTrace();
                test(false);
            }

            try
            {
                thrower.throwUndeclaredB(1, 2);
                test(false);
            }
            catch(com.zeroc.Ice.UnknownUserException ex)
            {
            }
            catch(Throwable ex)
            {
                ex.printStackTrace();
                test(false);
            }

            try
            {
                thrower.throwUndeclaredC(1, 2, 3);
                test(false);
            }
            catch(com.zeroc.Ice.UnknownUserException ex)
            {
            }
            catch(Throwable ex)
            {
                ex.printStackTrace();
                test(false);
            }

            out.println("ok");
        }

        if(thrower.supportsAssertException())
        {
            out.print("testing assert in the server... ");
            out.flush();

            try
            {
                thrower.throwAssertException();
                test(false);
            }
            catch(com.zeroc.Ice.ConnectionLostException ex)
            {
            }
            catch(com.zeroc.Ice.UnknownException ex)
            {
            }
            catch(Throwable ex)
            {
                ex.printStackTrace();
                test(false);
            }

            out.println("ok");
        }

        if(thrower.ice_getConnection() != null)
        {
            out.print("testing memory limit marshal exception...");
            out.flush();
            try
            {
                thrower.throwMemoryLimitException(null);
                test(false);
            }
            catch(com.zeroc.Ice.MemoryLimitException ex)
            {
            }
            catch(Throwable ex)
            {
                ex.printStackTrace();
                test(false);
            }

            try
            {
                thrower.throwMemoryLimitException(new byte[20 * 1024]); // 20KB
                test(false);
            }
            catch(com.zeroc.Ice.ConnectionLostException ex)
            {
            }
            catch(com.zeroc.Ice.UnknownLocalException ex)
            {
                // Expected with JS bidir server
            }
            catch(com.zeroc.Ice.SocketException ex)
            {
                // This can be raised if the connection is closed during the client's call to write().
            }
            catch(Throwable ex)
            {
                ex.printStackTrace();
                test(false);
            }

            try
            {
                ThrowerPrx thrower2 = ThrowerPrx.uncheckedCast(communicator.stringToProxy("thrower:" +
                                                                                          app.getTestEndpoint(1)));
                try
                {
                    thrower2.throwMemoryLimitException(new byte[2 * 1024 * 1024]); // 2MB (no limits)
                }
                catch(com.zeroc.Ice.MemoryLimitException ex)
                {
                }
                ThrowerPrx thrower3 = ThrowerPrx.uncheckedCast(communicator.stringToProxy("thrower:" +
                                                                                          app.getTestEndpoint(2)));
                try
                {
                    thrower3.throwMemoryLimitException(new byte[1024]); // 1KB limit
                    test(false);
                }
                catch(com.zeroc.Ice.ConnectionLostException ex)
                {
                }
            }
            catch(com.zeroc.Ice.ConnectionRefusedException ex)
            {
                // Expected with JS bidir server
            }

            out.println("ok");
        }

        out.print("catching object not exist exception... ");
        out.flush();

        {
            com.zeroc.Ice.Identity id = com.zeroc.Ice.Util.stringToIdentity("does not exist");
            try
            {
                ThrowerPrx thrower2 = ThrowerPrx.uncheckedCast(thrower.ice_identity(id));
                thrower2.ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.ObjectNotExistException ex)
            {
                test(ex.id.equals(id));
            }
            catch(Throwable ex)
            {
                ex.printStackTrace();
                test(false);
            }
        }

        out.println("ok");

        out.print("catching facet not exist exception... ");
        out.flush();

        try
        {
            ThrowerPrx thrower2 = ThrowerPrx.uncheckedCast(thrower, "no such facet");
            try
            {
                thrower2.ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.FacetNotExistException ex)
            {
                test(ex.facet.equals("no such facet"));
            }
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        out.println("ok");

        out.print("catching operation not exist exception... ");
        out.flush();

        try
        {
            WrongOperationPrx thrower2 = WrongOperationPrx.uncheckedCast(thrower);
            thrower2.noSuchOperation();
            test(false);
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
            test(ex.operation.equals("noSuchOperation"));
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        out.println("ok");

        out.print("catching unknown local exception... ");
        out.flush();

        try
        {
            thrower.throwLocalException();
            test(false);
        }
        catch(com.zeroc.Ice.UnknownLocalException ex)
        {
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        try
        {
            thrower.throwLocalExceptionIdempotent();
            test(false);
        }
        catch(com.zeroc.Ice.UnknownLocalException ex)
        {
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            test(false);
        }

        out.println("ok");

        out.print("catching unknown non-Ice exception... ");
        out.flush();

        try
        {
            thrower.throwNonIceException();
            test(false);
        }
        catch(com.zeroc.Ice.UnknownException ex)
        {
        }
        catch(Throwable ex)
        {
            out.println(ex);
            test(false);
        }

        out.println("ok");

        out.print("testing asynchronous exceptions... ");
        out.flush();

        try
        {
            thrower.throwAfterResponse();
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            test(false);
        }
        catch(Throwable ex)
        {
            out.println(ex);
            test(false);
        }

        try
        {
            thrower.throwAfterException();
            test(false);
        }
        catch(A ex)
        {
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            test(false);
        }
        catch(Throwable ex)
        {
            out.println(ex);
            test(false);
        }

        out.println("ok");

        out.print("catching exact types with AMI mapping... ");
        out.flush();

        try
        {
            thrower.throwAasAAsync(1).join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof A);
            test(((A)ex.getCause()).aMem == 1);
        }

        try
        {
            thrower.throwAorDasAorDAsync(1).join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof A);
            test(((A)ex.getCause()).aMem == 1);
        }

        try
        {
            thrower.throwAorDasAorDAsync(-1).join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof D);
            test(((D)ex.getCause()).dMem == -1);
        }

        try
        {
            thrower.throwBasBAsync(1, 2).join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof B);
            test(((B)ex.getCause()).aMem == 1);
            test(((B)ex.getCause()).bMem == 2);
        }

        try
        {
            thrower.throwCasCAsync(1, 2, 3).join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof C);
            test(((C)ex.getCause()).aMem == 1);
            test(((C)ex.getCause()).bMem == 2);
            test(((C)ex.getCause()).cMem == 3);
        }

        out.println("ok");

        out.print("catching derived types with AMI mapping... ");
        out.flush();

        try
        {
            thrower.throwBasAAsync(1, 2).join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof B);
            test(((B)ex.getCause()).aMem == 1);
            test(((B)ex.getCause()).bMem == 2);
        }

        try
        {
            thrower.throwCasAAsync(1, 2, 3).join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof C);
            test(((C)ex.getCause()).aMem == 1);
            test(((C)ex.getCause()).bMem == 2);
            test(((C)ex.getCause()).cMem == 3);
        }

        try
        {
            thrower.throwCasBAsync(1, 2, 3).join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof C);
            test(((C)ex.getCause()).aMem == 1);
            test(((C)ex.getCause()).bMem == 2);
            test(((C)ex.getCause()).cMem == 3);
        }

        out.println("ok");

        if(thrower.supportsUndeclaredExceptions())
        {
            out.print("catching unknown user exception with AMI mapping... ");
            out.flush();

            try
            {
                thrower.throwUndeclaredAAsync(1).join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.UnknownUserException);
            }

            try
            {
                thrower.throwUndeclaredBAsync(1, 2).join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.UnknownUserException);
            }

            try
            {
                thrower.throwUndeclaredCAsync(1, 2, 3).join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.UnknownUserException);
            }

            out.println("ok");
        }

        if(thrower.supportsAssertException())
        {
            out.print("catching assert in the server with AMI mapping... ");
            out.flush();

            try
            {
                thrower.throwAssertExceptionAsync().join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.ConnectionLostException ||
                     ex.getCause() instanceof com.zeroc.Ice.UnknownException);
            }

            out.println("ok");
        }

        out.print("catching object not exist exception with AMI mapping... ");
        out.flush();

        {
            com.zeroc.Ice.Identity id = com.zeroc.Ice.Util.stringToIdentity("does not exist");
            ThrowerPrx thrower2 = ThrowerPrx.uncheckedCast(thrower.ice_identity(id));
            try
            {
                thrower2.throwAasAAsync(1).join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.ObjectNotExistException);
                test(((com.zeroc.Ice.ObjectNotExistException)ex.getCause()).id.name.equals("does not exist"));
            }
        }

        out.println("ok");

        out.print("catching facet not exist exception with AMI mapping... ");
        out.flush();

        {
            ThrowerPrx thrower2 = ThrowerPrx.uncheckedCast(thrower, "no such facet");
            try
            {
                thrower2.throwAasAAsync(1).join();
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.FacetNotExistException);
                test(((com.zeroc.Ice.FacetNotExistException)ex.getCause()).facet.equals("no such facet"));
            }
        }

        out.println("ok");

        out.print("catching operation not exist exception with AMI mapping... ");
        out.flush();

        {
            WrongOperationPrx thrower2 = WrongOperationPrx.uncheckedCast(thrower);
            try
            {
                thrower2.noSuchOperationAsync().join();
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.OperationNotExistException);
                test(((com.zeroc.Ice.OperationNotExistException)ex.getCause()).operation.equals("noSuchOperation"));
            }
        }

        out.println("ok");

        out.print("catching unknown local exception with AMI mapping... ");
        out.flush();

        try
        {
            thrower.throwLocalExceptionAsync().join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof com.zeroc.Ice.UnknownLocalException ||
                 ex.getCause() instanceof com.zeroc.Ice.OperationNotExistException);
        }

        try
        {
            thrower.throwLocalExceptionIdempotentAsync().join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof com.zeroc.Ice.UnknownLocalException ||
                 ex.getCause() instanceof com.zeroc.Ice.OperationNotExistException);
        }

        out.println("ok");

        out.print("catching unknown non-Ice exception with AMI mapping... ");
        out.flush();

        try
        {
            thrower.throwNonIceExceptionAsync().join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof com.zeroc.Ice.UnknownException);
        }

        out.println("ok");

        return thrower;
    }
}
