// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;
using Test;

#if SILVERLIGHT
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
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
            _m.Lock();
            try
            {
                while(!_called)
                {
                    _m.Wait();
                }

                _called = false;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public virtual void called()
        {
            _m.Lock();
            try
            {
                Debug.Assert(!_called);
                _called = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        private bool _called;
        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }

    private class AMI_Thrower_throwAasAI : AMI_Thrower_throwAasA
    {
        public AMI_Thrower_throwAasAI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(A ex)
            {
                test(ex.aMem == 1);
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    private class AMI_Thrower_throwAasAObjectNotExistI : AMI_Thrower_throwAasA
    {
        public AMI_Thrower_throwAasAObjectNotExistI(Ice.Communicator comm)
        {
            InitBlock(comm);
        }
        private void InitBlock(Ice.Communicator comm)
        {
            callback = new Callback();
            communicator = comm;
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.ObjectNotExistException ex)
            {
                Ice.Identity id = communicator.stringToIdentity("does not exist");
                test(ex.id.Equals(id));
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Ice.Communicator communicator;
        private Callback callback;
    }

    private class AMI_Thrower_throwAasAFacetNotExistI : AMI_Thrower_throwAasA
    {
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.FacetNotExistException ex)
            {
                test(ex.facet.Equals("no such facet"));
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_Thrower_throwAorDasAorDI : AMI_Thrower_throwAorDasAorD
    {
        public AMI_Thrower_throwAorDasAorDI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
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
                test(ex.dMem == - 1);
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    private class AMI_Thrower_throwBasAI : AMI_Thrower_throwBasA
    {
        public AMI_Thrower_throwBasAI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
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
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    private class AMI_Thrower_throwCasAI : AMI_Thrower_throwCasA
    {
        public AMI_Thrower_throwCasAI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
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
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    private class AMI_Thrower_throwBasBI : AMI_Thrower_throwBasB
    {
        public AMI_Thrower_throwBasBI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
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
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    private class AMI_Thrower_throwCasBI : AMI_Thrower_throwCasB
    {
        public AMI_Thrower_throwCasBI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
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
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    private class AMI_Thrower_throwCasCI : AMI_Thrower_throwCasC
    {
        public AMI_Thrower_throwCasCI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
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
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    private class AMI_Thrower_throwUndeclaredAI : AMI_Thrower_throwUndeclaredA
    {
        public AMI_Thrower_throwUndeclaredAI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownUserException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    private class AMI_Thrower_throwUndeclaredBI : AMI_Thrower_throwUndeclaredB
    {
        public AMI_Thrower_throwUndeclaredBI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownUserException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    private class AMI_Thrower_throwUndeclaredCI : AMI_Thrower_throwUndeclaredC
    {
        public AMI_Thrower_throwUndeclaredCI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownUserException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    private class AMI_Thrower_throwLocalExceptionI : AMI_Thrower_throwLocalException
    {
        public AMI_Thrower_throwLocalExceptionI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownLocalException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    private class AMI_Thrower_throwNonIceExceptionI : AMI_Thrower_throwNonIceException
    {
        public AMI_Thrower_throwNonIceExceptionI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    private class AMI_WrongOperation_noSuchOperationI : AMI_WrongOperation_noSuchOperation
    {
        public AMI_WrongOperation_noSuchOperationI()
        {
            InitBlock();
        }
        private void InitBlock()
        {
            callback = new Callback();
        }
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.OperationNotExistException ex)
            {
                test(ex.operation.Equals("noSuchOperation"));
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public virtual void check()
        {
            callback.check();
        }

        private Callback callback;
    }

    class AsyncCallback
    {
        public AsyncCallback()
        {
            callback = new Callback();
        }

        public AsyncCallback(Ice.Communicator c)
        {
            _communicator = c;
            callback = new Callback();
        }

        public void response()
        {
            test(false);
        }

        public void exception_AasA(Ice.Exception exc)
        {
            test(exc is A);
            A ex = exc as A;
            test(ex.aMem == 1);
            callback.called();
        }

        public void exception_AorDasAorD(Ice.Exception exc)
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
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_BasB(Ice.Exception exc)
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
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_CasC(Ice.Exception exc)
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
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_BasA(Ice.Exception exc)
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
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_CasA(Ice.Exception exc)
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
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_CasB(Ice.Exception exc)
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
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_UndeclaredA(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownUserException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_UndeclaredB(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownUserException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_UndeclaredC(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownUserException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_AasAObjectNotExist(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.ObjectNotExistException ex)
            {
                Ice.Identity id = _communicator.stringToIdentity("does not exist");
                test(ex.id.Equals(id));
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_AasAFacetNotExist(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.FacetNotExistException ex)
            {
                test(ex.facet.Equals("no such facet"));
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_noSuchOperation(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.OperationNotExistException ex)
            {
                test(ex.operation.Equals("noSuchOperation"));
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_LocalException(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownLocalException)
            {
            }
            catch(Ice.OperationNotExistException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void exception_NonIceException(Ice.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.UnknownException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback;
        private Ice.Communicator _communicator;
    }

#if SILVERLIGHT
    public override Ice.InitializationData initData()
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        WriteLine("setting Ice.FactoryAssemblies");
        initData.properties.setProperty("Ice.FactoryAssemblies", "exceptions,version=1.0.0.0");
        initData.properties.setProperty("Ice.MessageSizeMax", "10");
        return initData;
    }

    override
    public void run(Ice.Communicator communicator)
#else
    public static ThrowerPrx allTests(Ice.Communicator communicator, bool collocated)
#endif
    {
#if SILVERLIGHT
        bool collocated = false;
        WriteLine("Ice.FactoryAssemblies: " + communicator.getProperties().getProperty("Ice.FactoryAssemblies"));
#endif

#if !SILVERLIGHT
        {
            Write("testing object adapter registration exceptions... ");
            Ice.ObjectAdapter first;
            try
            {
                first = communicator.createObjectAdapter("TestAdapter0");
            }
            catch(Ice.InitializationException)
            {
                // Expected
            }

            communicator.getProperties().setProperty("TestAdapter0.Endpoints", "default");
            first = communicator.createObjectAdapter("TestAdapter0");
            try
            {
                communicator.createObjectAdapter("TestAdapter0");
                test(false);
            }
            catch(Ice.AlreadyRegisteredException)
            {
                // Expected.
            }

            try
            {
                Ice.ObjectAdapter second =
                    communicator.createObjectAdapterWithEndpoints("TestAdapter0", "ssl -h foo -p 12011");
                test(false);

                //
                // Quell mono error that variable second isn't used.
                //
                second.deactivate();
            }
            catch(Ice.AlreadyRegisteredException)
            {
                // Expected
            }
            first.deactivate();
            WriteLine("ok");
        }

        {
            Write("testing servant registration exceptions... ");
            communicator.getProperties().setProperty("TestAdapter1.Endpoints", "default");
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter1");
            Ice.Object obj = new EmptyI();
            adapter.add(obj, communicator.stringToIdentity("x"));
            try
            {
                adapter.add(obj, communicator.stringToIdentity("x"));
                test(false);
            }
            catch(Ice.AlreadyRegisteredException)
            {
            }

            adapter.remove(communicator.stringToIdentity("x"));
            try
            {
                adapter.remove(communicator.stringToIdentity("x"));
                test(false);
            }
            catch(Ice.NotRegisteredException)
            {
            }
            adapter.deactivate();
            WriteLine("ok");
        }

        {
            Write("testing servant locator registration exceptions... ");
            communicator.getProperties().setProperty("TestAdapter2.Endpoints", "default");
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter2");
            Ice.ServantLocator loc = new ServantLocatorI();
            adapter.addServantLocator(loc, "x");
            try
            {
                adapter.addServantLocator(loc, "x");
                test(false);
            }
            catch(Ice.AlreadyRegisteredException)
            {
            }

            adapter.deactivate();
            WriteLine("ok");
        }
#endif
        {
            Write("testing object factory registration exception... ");
            Ice.ObjectFactory of = new ObjectFactoryI();
            communicator.addObjectFactory(of, "::x");
            try
            {
                communicator.addObjectFactory(of, "::x");
                test(false);
            }
            catch(Ice.AlreadyRegisteredException)
            {
            }
            WriteLine("ok");
        }

        Write("testing stringToProxy... ");
        Flush();
        String @ref = "thrower:default -p 12010";
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        WriteLine("ok");

        Write("testing checked cast... ");
        Flush();
        ThrowerPrx thrower = ThrowerPrxHelper.checkedCast(@base);
        test(thrower != null);
        test(thrower.Equals(@base));
        WriteLine("ok");

        Write("catching exact types... ");
        Flush();

        try
        {
            thrower.throwAasA(1);
            test(false);
        }
        catch(A ex)
        {
            test(ex.aMem == 1);
        }
        catch(Exception)
        {
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
        catch(Exception)
        {
            test(false);
        }

        try
        {
            thrower.throwAorDasAorD(- 1);
            test(false);
        }
        catch(D ex)
        {
            test(ex.dMem == - 1);
        }
        catch(Exception)
        {
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
        catch(Exception)
        {
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
        catch(Exception)
        {
            test(false);
        }

        WriteLine("ok");

        Write("catching base types... ");
        Flush();

        try
        {
            thrower.throwBasB(1, 2);
            test(false);
        }
        catch(A ex)
        {
            test(ex.aMem == 1);
        }
        catch(Exception)
        {
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
        catch(Exception)
        {
            test(false);
        }

        WriteLine("ok");

        Write("catching derived types... ");
        Flush();

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
        catch(Exception)
        {
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
        catch(Exception)
        {
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
        catch(Exception)
        {
            test(false);
        }

        WriteLine("ok");

        if(thrower.supportsUndeclaredExceptions())
        {
            Write("catching unknown user exception... ");
            Flush();

            try
            {
                thrower.throwUndeclaredA(1);
                test(false);
            }
            catch(Ice.UnknownUserException)
            {
            }
            catch(Exception)
            {
                test(false);
            }

            try
            {
                thrower.throwUndeclaredB(1, 2);
                test(false);
            }
            catch(Ice.UnknownUserException)
            {
            }
            catch(Exception)
            {
                test(false);
            }

            try
            {
                thrower.throwUndeclaredC(1, 2, 3);
                test(false);
            }
            catch(Ice.UnknownUserException)
            {
            }
            catch(Exception)
            {
                test(false);
            }

            WriteLine("ok");
        }

        Write("testing memory limit marshal exception...");
        Flush();
        {
            try
            {
                thrower.throwMemoryLimitException(null);
                test(collocated);
            }
            catch(Ice.UnknownLocalException)
            {
            }
            catch(Exception)
            {
                test(false);
            }

            try
            {
                thrower.throwMemoryLimitException(new byte[20 * 1024]); // 20KB
                test(collocated);
            }
            catch(Ice.MemoryLimitException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            
            if(!collocated)
            {
                try
                {
                    thrower.end_throwMemoryLimitException(
                        thrower.begin_throwMemoryLimitException(new byte[20 * 1024])); // 20KB
                    test(false);
                }
                catch(Ice.MemoryLimitException)
                {
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }
        WriteLine("ok");

        Write("catching object not exist exception... ");
        Flush();

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
                test(ex.id.Equals(id));
            }
            catch(Exception)
            {
                test(false);
            }
        }

        WriteLine("ok");

        Write("catching facet not exist exception... ");
        Flush();

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
                test(ex.facet.Equals("no such facet"));
            }
        }
        catch(Exception)
        {
            test(false);
        }

        WriteLine("ok");

        Write("catching operation not exist exception... ");
        Flush();

        try
        {
            WrongOperationPrx thrower2 = WrongOperationPrxHelper.uncheckedCast(thrower);
            thrower2.noSuchOperation();
            test(false);
        }
        catch(Ice.OperationNotExistException ex)
        {
            test(ex.operation.Equals("noSuchOperation"));
        }
        catch(Exception)
        {
            test(false);
        }

        WriteLine("ok");

        Write("catching unknown local exception... ");
        Flush();

        try
        {
            thrower.throwLocalException();
            test(false);
        }
        catch(Ice.UnknownLocalException)
        {
        }
        catch(Exception)
        {
            test(false);
        }
        try
        {
            thrower.throwLocalExceptionIdempotent();
            test(false);
        }
        catch(Ice.UnknownLocalException)
        {
        }
        catch(Ice.OperationNotExistException)
        {
        }
        catch(Exception)
        {
            test(false);
        }

        WriteLine("ok");

        Write("catching unknown non-Ice exception... ");
        Flush();

        try
        {
            thrower.throwNonIceException();
            test(false);
        }
        catch(Ice.UnknownException)
        {
        }
        catch(System.Exception)
        {
            test(false);
        }

        WriteLine("ok");

        Write("testing asynchronous exceptions... ");
        Flush();

        try
        {
            thrower.throwAfterResponse();
        }
        catch(Exception)
        {
            test(false);
        }

        try
        {
            thrower.throwAfterException();
            test(false);
        }
        catch(A)
        {
        }
        catch(Exception)
        {
            test(false);
        }

        WriteLine("ok");

        if(!collocated)
        {
            Write("catching exact types with AMI... ");
            Flush();

            {
                AMI_Thrower_throwAasAI cb = new AMI_Thrower_throwAasAI();
                thrower.throwAasA_async(cb, 1);
                cb.check();
            }

            {
                AMI_Thrower_throwAorDasAorDI cb = new AMI_Thrower_throwAorDasAorDI();
                thrower.throwAorDasAorD_async(cb, 1);
                cb.check();
            }

            {
                AMI_Thrower_throwAorDasAorDI cb = new AMI_Thrower_throwAorDasAorDI();
                thrower.throwAorDasAorD_async(cb, - 1);
                cb.check();
            }

            {
                AMI_Thrower_throwBasBI cb = new AMI_Thrower_throwBasBI();
                thrower.throwBasB_async(cb, 1, 2);
                cb.check();
            }

            {
                AMI_Thrower_throwCasCI cb = new AMI_Thrower_throwCasCI();
                thrower.throwCasC_async(cb, 1, 2, 3);
                cb.check();
            }

            WriteLine("ok");

            Write("catching derived types... ");
            Flush();

            {
                AMI_Thrower_throwBasAI cb = new AMI_Thrower_throwBasAI();
                thrower.throwBasA_async(cb, 1, 2);
                cb.check();
            }

            {
                AMI_Thrower_throwCasAI cb = new AMI_Thrower_throwCasAI();
                thrower.throwCasA_async(cb, 1, 2, 3);
                cb.check();
            }

            {
                AMI_Thrower_throwCasBI cb = new AMI_Thrower_throwCasBI();
                thrower.throwCasB_async(cb, 1, 2, 3);
                cb.check();
            }

            WriteLine("ok");

            if(thrower.supportsUndeclaredExceptions())
            {
                Write("catching unknown user exception with AMI... ");
                Flush();

                {
                    AMI_Thrower_throwUndeclaredAI cb = new AMI_Thrower_throwUndeclaredAI();
                    thrower.throwUndeclaredA_async(cb, 1);
                    cb.check();
                }

                {
                    AMI_Thrower_throwUndeclaredBI cb = new AMI_Thrower_throwUndeclaredBI();
                    thrower.throwUndeclaredB_async(cb, 1, 2);
                    cb.check();
                }

                {
                    AMI_Thrower_throwUndeclaredCI cb = new AMI_Thrower_throwUndeclaredCI();
                    thrower.throwUndeclaredC_async(cb, 1, 2, 3);
                    cb.check();
                }

                WriteLine("ok");
            }

            Write("catching object not exist exception with AMI... ");
            Flush();

            {
                Ice.Identity id = communicator.stringToIdentity("does not exist");
                ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower.ice_identity(id));
                AMI_Thrower_throwAasAObjectNotExistI cb = new AMI_Thrower_throwAasAObjectNotExistI(communicator);
                thrower2.throwAasA_async(cb, 1);
                cb.check();
            }

            WriteLine("ok");

            Write("catching facet not exist exception with AMI... ");
            Flush();

            try
            {
                ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower, "no such facet");
                {
                    AMI_Thrower_throwAasAFacetNotExistI cb = new AMI_Thrower_throwAasAFacetNotExistI();
                    thrower2.throwAasA_async(cb, 1);
                    cb.check();
                }
            }
            catch(Exception)
            {
                test(false);
            }

            WriteLine("ok");

            Write("catching operation not exist exception with AMI... ");
            Flush();

            {
                AMI_WrongOperation_noSuchOperationI cb = new AMI_WrongOperation_noSuchOperationI();
                WrongOperationPrx thrower2 = WrongOperationPrxHelper.uncheckedCast(thrower);
                thrower2.noSuchOperation_async(cb);
                cb.check();
            }

            WriteLine("ok");

            Write("catching unknown local exception with AMI... ");
            Flush();

            {
                AMI_Thrower_throwLocalExceptionI cb = new AMI_Thrower_throwLocalExceptionI();
                thrower.throwLocalException_async(cb);
                cb.check();
            }

            WriteLine("ok");

            Write("catching unknown non-Ice exception with AMI... ");
            Flush();

            AMI_Thrower_throwNonIceExceptionI cb2 = new AMI_Thrower_throwNonIceExceptionI();
            thrower.throwNonIceException_async(cb2);
            cb2.check();

            WriteLine("ok");

            Write("catching exact types with new AMI mapping... ");
            Flush();

            {
                AsyncCallback cb3 = new AsyncCallback();
                thrower.begin_throwAasA(1).whenCompleted(cb3.response, cb3.exception_AasA);
                cb3.check();
            }

            {
                AsyncCallback cb3 = new AsyncCallback();
                thrower.begin_throwAorDasAorD(1).whenCompleted(cb3.response, cb3.exception_AorDasAorD);
                cb3.check();
            }

            {
                AsyncCallback cb3 = new AsyncCallback();
                thrower.begin_throwAorDasAorD(-1).whenCompleted(cb3.response, cb3.exception_AorDasAorD);
                cb3.check();
            }

            {
                AsyncCallback cb3 = new AsyncCallback();
                thrower.begin_throwBasB(1, 2).whenCompleted(cb3.response, cb3.exception_BasB);
                cb3.check();
            }

            {
                AsyncCallback cb3 = new AsyncCallback();
                thrower.begin_throwCasC(1, 2, 3).whenCompleted(cb3.response, cb3.exception_CasC);
                cb3.check();
            }

            WriteLine("ok");

            Write("catching derived types with new AMI mapping... ");
            Flush();

            {
                AsyncCallback cb3 = new AsyncCallback();
                thrower.begin_throwBasA(1, 2).whenCompleted(cb3.response, cb3.exception_BasA);
                cb3.check();
            }

            {
                AsyncCallback cb3 = new AsyncCallback();
                thrower.begin_throwCasA(1, 2, 3).whenCompleted(cb3.response, cb3.exception_CasA);
                cb3.check();
            }

            {
                AsyncCallback cb3 = new AsyncCallback();
                thrower.begin_throwCasB(1, 2, 3).whenCompleted(cb3.response, cb3.exception_CasB);
                cb3.check();
            }

            WriteLine("ok");

            if(thrower.supportsUndeclaredExceptions())
            {
                Write("catching unknown user exception with new AMI mapping... ");
                Flush();

                {
                    AsyncCallback cb3 = new AsyncCallback();
                    thrower.begin_throwUndeclaredA(1).whenCompleted(cb3.response, cb3.exception_UndeclaredA);
                    cb3.check();
                }

                {
                    AsyncCallback cb3 = new AsyncCallback();
                    thrower.begin_throwUndeclaredB(1, 2).whenCompleted(cb3.response, cb3.exception_UndeclaredB);
                    cb3.check();
                }

                {
                    AsyncCallback cb3 = new AsyncCallback();
                    thrower.begin_throwUndeclaredC(1, 2, 3).whenCompleted(cb3.response, cb3.exception_UndeclaredC);
                    cb3.check();
                }

                WriteLine("ok");
            }

            Write("catching object not exist exception with new AMI mapping... ");
            Flush();

            {
                Ice.Identity id = communicator.stringToIdentity("does not exist");
                ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower.ice_identity(id));
                AsyncCallback cb3 = new AsyncCallback(communicator);
                thrower2.begin_throwAasA(1).whenCompleted(cb3.response, cb3.exception_AasAObjectNotExist);
                cb3.check();
            }

            WriteLine("ok");

            Write("catching facet not exist exception with new AMI mapping... ");
            Flush();

            {
                ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower, "no such facet");
                AsyncCallback cb3 = new AsyncCallback();
                thrower2.begin_throwAasA(1).whenCompleted(cb3.response, cb3.exception_AasAFacetNotExist);
                cb3.check();
            }

            WriteLine("ok");

            Write("catching operation not exist exception with new AMI mapping... ");
            Flush();

            {
                AsyncCallback cb3 = new AsyncCallback();
                WrongOperationPrx thrower4 = WrongOperationPrxHelper.uncheckedCast(thrower);
                thrower4.begin_noSuchOperation().whenCompleted(cb3.response, cb3.exception_noSuchOperation);
                cb3.check();
            }

            WriteLine("ok");

            Write("catching unknown local exception with new AMI mapping... ");
            Flush();

            {
                AsyncCallback cb3 = new AsyncCallback();
                thrower.begin_throwLocalException().whenCompleted(cb3.response, cb3.exception_LocalException);
                cb3.check();
            }

            {
                AsyncCallback cb3 = new AsyncCallback();
                thrower.begin_throwLocalExceptionIdempotent().whenCompleted(cb3.response, cb3.exception_LocalException);
                cb3.check();
            }

            WriteLine("ok");

            Write("catching unknown non-Ice exception with new AMI mapping... ");
            Flush();

            {
                AsyncCallback cb3 = new AsyncCallback();
                thrower.begin_throwNonIceException().whenCompleted(cb3.response, cb3.exception_NonIceException);
                cb3.check();
            }

            WriteLine("ok");
        }
#if SILVERLIGHT
        thrower.shutdown();
#else
        return thrower;
#endif
    }
}
