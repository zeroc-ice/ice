// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

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

        public synchronized boolean
        check()
        {
            while(!_called)
            {
                try
                {
                    wait(5000);
                }
                catch(InterruptedException ex)
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
        
        public synchronized void
        called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static class AMI_Thrower_throwAasAI extends AMI_Thrower_throwAasA
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            exc.printStackTrace();
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(A ex)
            {
                test(ex.aMem == 1);
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwAasAObjectNotExistI extends AMI_Thrower_throwAasA
    {
        AMI_Thrower_throwAasAObjectNotExistI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.ObjectNotExistException ex)
            {
                Ice.Identity id = _communicator.stringToIdentity("does not exist");
                test(ex.id.equals(id));
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            exc.printStackTrace();
            test(false);
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
        private Ice.Communicator _communicator;
    }

    private static class AMI_Thrower_throwAasAFacetNotExistI extends AMI_Thrower_throwAasA
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.FacetNotExistException ex)
            {
                test(ex.facet.equals("no such facet"));
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            exc.printStackTrace();
            test(false);
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwAorDasAorDI extends AMI_Thrower_throwAorDasAorD
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            exc.printStackTrace();
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(A ex)
            {
                test(ex.aMem == 1);
            }
            catch(D ex)
            {
                test(ex.dMem == -1);
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwBasAI extends AMI_Thrower_throwBasA
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            exc.printStackTrace();
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(B ex)
            {
                test(ex.aMem == 1);
                test(ex.bMem == 2);
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwCasAI extends AMI_Thrower_throwCasA
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            exc.printStackTrace();
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(C ex)
            {
                test(ex.aMem == 1);
                test(ex.bMem == 2);
                test(ex.cMem == 3);
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwBasBI extends AMI_Thrower_throwBasB
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            exc.printStackTrace();
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(B ex)
            {
                test(ex.aMem == 1);
                test(ex.bMem == 2);
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwCasBI extends AMI_Thrower_throwCasB
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            exc.printStackTrace();
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(C ex)
            {
                test(ex.aMem == 1);
                test(ex.bMem == 2);
                test(ex.cMem == 3);
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwCasCI extends AMI_Thrower_throwCasC
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            exc.printStackTrace();
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            try
            {
                throw exc;
            }
            catch(C ex)
            {
                test(ex.aMem == 1);
                test(ex.bMem == 2);
                test(ex.cMem == 3);
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwUndeclaredAI extends AMI_Thrower_throwUndeclaredA
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownUserException ex)
            {
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwUndeclaredBI extends AMI_Thrower_throwUndeclaredB
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownUserException ex)
            {
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwUndeclaredCI extends AMI_Thrower_throwUndeclaredC
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownUserException ex)
            {
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwLocalExceptionI extends AMI_Thrower_throwLocalException
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownLocalException ex)
            {
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwNonIceExceptionI extends AMI_Thrower_throwNonIceException
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownException ex)
            {
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Thrower_throwAssertExceptionI extends AMI_Thrower_throwAssertException
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.ConnectionLostException ex)
            {
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_WrongOperation_noSuchOperationI extends AMI_WrongOperation_noSuchOperation
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.OperationNotExistException ex)
            {
                test(ex.operation.equals("noSuchOperation"));
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    public static ThrowerPrx
    allTests(Ice.Communicator communicator, boolean collocated)
    {
        {
            System.out.print("testing object adapter registration exceptions... ");
            Ice.ObjectAdapter first;
            try
            {
                first = communicator.createObjectAdapter("TestAdapter0");
            }
            catch(Ice.InitializationException ex)
            {
                // Expected
            }

            communicator.getProperties().setProperty("TestAdapter0.Endpoints", "default");
            first = communicator.createObjectAdapter("TestAdapter0");
            try
            {
                Ice.ObjectAdapter second = communicator.createObjectAdapter("TestAdapter0");
                test(false);
            }
            catch(Ice.AlreadyRegisteredException ex)
            {
                // Expected
            }
            
            try
            {
                Ice.ObjectAdapter second = 
                    communicator.createObjectAdapterWithEndpoints("TestAdapter0", "ssl -h foo -p 12011 -t 10000");
                test(false);
            }
            catch(Ice.AlreadyRegisteredException ex)
            {
                // Expected
            }
            test(communicator.getProperties().getProperty("TestAdapter0.Endpoints").equals("default"));
            first.deactivate();
            System.out.println("ok");
        }
        
        {
            System.out.print("testing servant registration exceptions... ");
            communicator.getProperties().setProperty("TestAdapter1.Endpoints", "default");
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter1");
            Ice.Object obj = new EmptyI();
            adapter.add(obj, communicator.stringToIdentity("x"));
            try
            {
                adapter.add(obj, communicator.stringToIdentity("x"));
                test(false);
            }
            catch(Ice.AlreadyRegisteredException ex)
            {
            }

            adapter.remove(communicator.stringToIdentity("x"));
            try
            {
                adapter.remove(communicator.stringToIdentity("x"));
                test(false);
            }
            catch(Ice.NotRegisteredException ex)
            {
            }
            adapter.deactivate();
            System.out.println("ok");
        }

        {
            System.out.print("testing servant locator registration exceptions... ");
            communicator.getProperties().setProperty("TestAdapter2.Endpoints", "default");
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter2");
            Ice.ServantLocator loc = new ServantLocatorI();
            adapter.addServantLocator(loc, "x");
            try
            {
                adapter.addServantLocator(loc, "x");
                test(false);
            }
            catch(Ice.AlreadyRegisteredException ex)
            {
            }

            adapter.deactivate();
            System.out.println("ok");
        }

        {
            System.out.print("testing object factory registration exception... ");
            Ice.ObjectFactory of = new ObjectFactoryI();
            communicator.addObjectFactory(of, "::x");
            try
            {
                communicator.addObjectFactory(of, "::x");
                test(false);
            }
            catch(Ice.AlreadyRegisteredException ex)
            {
            }
            System.out.println("ok");
        }

        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "thrower:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        ThrowerPrx thrower = ThrowerPrxHelper.checkedCast(base);
        test(thrower != null);
        test(thrower.equals(base));
        System.out.println("ok");

        System.out.print("catching exact types... ");
        System.out.flush();

        try
        {
            thrower.throwAasA(1);
            test(false);
        }
        catch(A ex)
        {
            test(ex.aMem == 1);
        }
        catch(Exception ex)
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
        catch(Exception ex)
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
        catch(Exception ex)
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
        catch(Exception ex)
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
        catch(Exception ex)
        {
            ex.printStackTrace();
            test(false);
        }

        System.out.println("ok");

        System.out.print("catching base types... ");
        System.out.flush();

        try
        {
            thrower.throwBasB(1, 2);
            test(false);
        }
        catch(A ex)
        {
            test(ex.aMem == 1);
        }
        catch(Exception ex)
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
        catch(Exception ex)
        {
            ex.printStackTrace();
            test(false);
        }

        System.out.println("ok");

        System.out.print("catching derived types... ");
        System.out.flush();

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
        catch(Exception ex)
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
        catch(Exception ex)
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
        catch(Exception ex)
        {
            ex.printStackTrace();
            test(false);
        }

        System.out.println("ok");

        if(thrower.supportsUndeclaredExceptions())
        {
            test(!collocated);

            System.out.print("catching unknown user exception... ");
            System.out.flush();
            
            try
            {
                thrower.throwUndeclaredA(1);
                test(false);
            }
            catch(Ice.UnknownUserException ex)
            {
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            
            try
            {
                thrower.throwUndeclaredB(1, 2);
                test(false);
            }
            catch(Ice.UnknownUserException ex)
            {
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            
            try
            {
                thrower.throwUndeclaredC(1, 2, 3);
                test(false);
            }
            catch(Ice.UnknownUserException ex)
            {
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            
            System.out.println("ok");
        }
        
        if(thrower.supportsAssertException())
        {
            System.out.print("testing assert in the server... ");
            System.out.flush();
            
            try
            {
                thrower.throwAssertException();
                test(false);
            }
            catch(java.lang.AssertionError ex)
            {
                assert(collocated);
            }
            catch(Ice.ConnectionLostException ex)
            {
                assert(!collocated);
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
            
            System.out.println("ok");
        }

        System.out.print("catching object not exist exception... ");
        System.out.flush();

        {
            Ice.Identity id = communicator.stringToIdentity("does not exist");
            try
            {
                ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower.ice_identity(id));
                thrower2.ice_ping();
                test(false);
            }
            catch(Ice.ObjectNotExistException ex)
            {
                test(ex.id.equals(id));
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
        }

        System.out.println("ok");

        System.out.print("catching facet not exist exception... ");
        System.out.flush();
 
        try
        {
            ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower, "no such facet");
            try
            {
                thrower2.ice_ping();
                test(false);
            }
            catch(Ice.FacetNotExistException ex)
            {
                test(ex.facet.equals("no such facet"));
            }
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            test(false);
        }

        System.out.println("ok");

        System.out.print("catching operation not exist exception... ");
        System.out.flush();

        try
        {
            WrongOperationPrx thrower2 = WrongOperationPrxHelper.uncheckedCast(thrower);
            thrower2.noSuchOperation();
            test(false);
        }
        catch(Ice.OperationNotExistException ex)
        {
            test(ex.operation.equals("noSuchOperation"));
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            test(false);
        }

        System.out.println("ok");

        System.out.print("catching unknown local exception... ");
        System.out.flush();

        try
        {
            thrower.throwLocalException();
            test(false);
        }
        catch(Ice.TimeoutException ex)
        {
            //
            // We get the original exception with collocation
            // optimization.
            //
            test(collocated);
        }
        catch(Ice.UnknownLocalException ex)
        {
            //
            // We get an unknown local exception without collocation
            // optimization.
            //
            test(!collocated);
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            test(false);
        }

        System.out.println("ok");

        System.out.print("catching unknown non-Ice exception... ");
        System.out.flush();

        try
        {
            thrower.throwNonIceException();
            test(false);
        }
        catch(Ice.UnknownException ex)
        {
            //
            // We get the an unknown exception without collocation
            // optimization.
            //
            test(!collocated);
        }
        catch(RuntimeException ex)
        {
            //
            // We get the original exception with collocation
            // optimization.
            //
            test(collocated);
        }

        System.out.println("ok");

        if(!collocated)
        {
            System.out.print("catching exact types with AMI... ");
            System.out.flush();

            {
                AMI_Thrower_throwAasAI cb = new AMI_Thrower_throwAasAI();
                thrower.throwAasA_async(cb, 1);
                test(cb.check());
                // Let's check if we can reuse the same callback object for another call.
                thrower.throwAasA_async(cb, 1);
                test(cb.check());
            }
        
            {
                AMI_Thrower_throwAorDasAorDI cb = new AMI_Thrower_throwAorDasAorDI();
                thrower.throwAorDasAorD_async(cb, 1);
                test(cb.check());
            }
        
            {
                AMI_Thrower_throwAorDasAorDI cb = new AMI_Thrower_throwAorDasAorDI();
                thrower.throwAorDasAorD_async(cb, -1);
                test(cb.check());
            }
        
            {
                AMI_Thrower_throwBasBI cb = new AMI_Thrower_throwBasBI();
                thrower.throwBasB_async(cb, 1, 2);
                test(cb.check());
            }
        
            {
                AMI_Thrower_throwCasCI cb = new AMI_Thrower_throwCasCI();
                thrower.throwCasC_async(cb, 1, 2, 3);
                test(cb.check());
                // Let's check if we can reuse the same callback object for another call.
                thrower.throwCasC_async(cb, 1, 2, 3);
                test(cb.check());
            }
        
            System.out.println("ok");
        
            System.out.print("catching derived types with AMI... ");
            System.out.flush();
        
            {
                AMI_Thrower_throwBasAI cb = new AMI_Thrower_throwBasAI();
                thrower.throwBasA_async(cb, 1, 2);
                test(cb.check());
            }

            {
                AMI_Thrower_throwCasAI cb = new AMI_Thrower_throwCasAI();
                thrower.throwCasA_async(cb, 1, 2, 3);
                test(cb.check());
            }
        
            {
                AMI_Thrower_throwCasBI cb = new AMI_Thrower_throwCasBI();
                thrower.throwCasB_async(cb, 1, 2, 3);
                test(cb.check());
            }
        
            System.out.println("ok");

            if(thrower.supportsUndeclaredExceptions())
            {
                System.out.print("catching unknown user exception with AMI... ");
                System.out.flush();
            
                {
                    AMI_Thrower_throwUndeclaredAI cb = new AMI_Thrower_throwUndeclaredAI();
                    thrower.throwUndeclaredA_async(cb, 1);
                    test(cb.check());
                }

                {
                    AMI_Thrower_throwUndeclaredBI cb = new AMI_Thrower_throwUndeclaredBI();
                    thrower.throwUndeclaredB_async(cb, 1, 2);
                    test(cb.check());
                }

                {
                    AMI_Thrower_throwUndeclaredCI cb = new AMI_Thrower_throwUndeclaredCI();
                    thrower.throwUndeclaredC_async(cb, 1, 2, 3);
                    test(cb.check());
                }
        
                System.out.println("ok");
            }

            if(thrower.supportsAssertException())
            {
                System.out.print("testing assert in the server with AMI... ");
                System.out.flush();
            
                AMI_Thrower_throwAssertExceptionI cb = new AMI_Thrower_throwAssertExceptionI();
                thrower.throwAssertException_async(cb);
                test(cb.check());
        
                System.out.println("ok");
            }

            System.out.print("catching object not exist exception with AMI... ");
            System.out.flush();

            {
                Ice.Identity id = communicator.stringToIdentity("does not exist");
                ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower.ice_identity(id));
                AMI_Thrower_throwAasAObjectNotExistI cb = new AMI_Thrower_throwAasAObjectNotExistI(communicator);
                thrower2.throwAasA_async(cb, 1);
                test(cb.check());
            }

            System.out.println("ok");

            System.out.print("catching facet not exist exception with AMI... ");
            System.out.flush();

            try
            {
                ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower, "no such facet");
                {
                    AMI_Thrower_throwAasAFacetNotExistI cb = new AMI_Thrower_throwAasAFacetNotExistI();
                    thrower2.throwAasA_async(cb, 1);
                    test(cb.check());
                }
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                test(false);
            }
                
            System.out.println("ok");

            System.out.print("catching operation not exist exception with AMI... ");
            System.out.flush();

            {
                AMI_WrongOperation_noSuchOperationI cb = new AMI_WrongOperation_noSuchOperationI();
                WrongOperationPrx thrower2 = WrongOperationPrxHelper.uncheckedCast(thrower);
                thrower2.noSuchOperation_async(cb);
                test(cb.check());
            }

            System.out.println("ok");
    
            System.out.print("catching unknown local exception with AMI... ");
            System.out.flush();

            {
                AMI_Thrower_throwLocalExceptionI cb = new AMI_Thrower_throwLocalExceptionI();
                thrower.throwLocalException_async(cb);
                test(cb.check());
            }
        
            System.out.println("ok");

            System.out.print("catching unknown non-Ice exception with AMI... ");
            System.out.flush();
        
            AMI_Thrower_throwNonIceExceptionI cb = new AMI_Thrower_throwNonIceExceptionI();
            thrower.throwNonIceException_async(cb);
            test(cb.check());
        
            System.out.println("ok");
        }

        return thrower;
    }
}
