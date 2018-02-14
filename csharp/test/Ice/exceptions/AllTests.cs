// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

#if SILVERLIGHT
    public override Ice.InitializationData initData()
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        WriteLine("setting Ice.FactoryAssemblies");
        initData.properties.setProperty("Ice.FactoryAssemblies", "exceptions,version=1.0.0.0");
        initData.properties.setProperty("Ice.MessageSizeMax", "10");
	initData.properties.setProperty("Ice.Warn.Connections", "0");
        return initData;
    }

    override
    public void run(Ice.Communicator communicator)
#else
    public static ThrowerPrx allTests(Ice.Communicator communicator)
#endif
    {
#if SILVERLIGHT
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

            try
            {
                adapter.add(obj, communicator.stringToIdentity(""));
                test(false);
            }
            catch(Ice.IllegalIdentityException e)
            {
                test(e.id.name.Equals(""));
            }

            try
            {
                adapter.add(null, communicator.stringToIdentity("x"));
                test(false);
            }
            catch(Ice.IllegalServantException)
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

        if(thrower.ice_getConnection() != null)
        {
            Write("testing memory limit marshal exception...");
            Flush();
            try
            {
                thrower.throwMemoryLimitException(null);
                test(false);
            }
            catch(Ice.MemoryLimitException)
            {
            }
            catch(Exception)
            {
                test(false);
            }

            try
            {
                thrower.throwMemoryLimitException(new byte[20 * 1024]); // 20KB
                test(false);
            }
            catch(Ice.ConnectionLostException)
            {
            }
            catch(Exception)
            {
                test(false);
            }

            ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(
                communicator.stringToProxy("thrower:default -p 12011"));
            try
            {
                thrower2.throwMemoryLimitException(new byte[2 * 1024 * 1024]); // 2MB (no limits)
            }
            catch(Ice.MemoryLimitException)
            {
            }
            ThrowerPrx thrower3 = ThrowerPrxHelper.uncheckedCast(
                communicator.stringToProxy("thrower:default -p 12012"));
            try
            {
                thrower3.throwMemoryLimitException(new byte[1024]); // 1KB limit
                test(false);
            }
            catch(Ice.ConnectionLostException)
            {
            }

            WriteLine("ok");
        }

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

        Write("catching exact types with AMI mapping... ");
        Flush();

        {
            Callback cb = new Callback();
            thrower.begin_throwAasA(1).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
                {
                    test(exc is A);
                    A ex = exc as A;
                    test(ex.aMem == 1);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            thrower.begin_throwAorDasAorD(1).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();                
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            thrower.begin_throwAorDasAorD(-1).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            thrower.begin_throwBasB(1, 2).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            thrower.begin_throwCasC(1, 2, 3).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        WriteLine("ok");

        Write("catching derived types with new AMI mapping... ");
        Flush();

        {
            Callback cb = new Callback();
            thrower.begin_throwBasA(1, 2).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            thrower.begin_throwCasA(1, 2, 3).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            thrower.begin_throwCasB(1, 2, 3).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        WriteLine("ok");

        if(thrower.supportsUndeclaredExceptions())
        {
            Write("catching unknown user exception with new AMI mapping... ");
            Flush();

            {
                Callback cb = new Callback();
                thrower.begin_throwUndeclaredA(1).whenCompleted(
                    () =>
                    {
                        test(false);
                    },
                    (Ice.Exception exc) =>
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
                        cb.called();
                    });
                cb.check();
            }

            {
                Callback cb = new Callback();
                thrower.begin_throwUndeclaredB(1, 2).whenCompleted(
                    () =>
                    {
                        test(false);
                    },
                    (Ice.Exception exc) =>
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
                        cb.called();
                    });
                cb.check();
            }

            {
                Callback cb = new Callback();
                thrower.begin_throwUndeclaredC(1, 2, 3).whenCompleted(
                    () =>
                    {
                        test(false);
                    },
                    (Ice.Exception exc) =>
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
                        cb.called();
                    });
                cb.check();
            }

            WriteLine("ok");
        }

        Write("catching object not exist exception with new AMI mapping... ");
        Flush();

        {
            Ice.Identity id = communicator.stringToIdentity("does not exist");
            ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower.ice_identity(id));
            Callback cb = new Callback();
            thrower2.begin_throwAasA(1).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
                {
                    try
                    {
                        throw exc;
                    }
                    catch(Ice.ObjectNotExistException ex)
                    {
                        test(ex.id.Equals(id));
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

        Write("catching facet not exist exception with new AMI mapping... ");
        Flush();

        {
            ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower, "no such facet");
            Callback cb = new Callback();
            thrower2.begin_throwAasA(1).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        WriteLine("ok");

        Write("catching operation not exist exception with new AMI mapping... ");
        Flush();

        {
            Callback cb = new Callback();
            WrongOperationPrx thrower4 = WrongOperationPrxHelper.uncheckedCast(thrower);
            thrower4.begin_noSuchOperation().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        WriteLine("ok");

        Write("catching unknown local exception with new AMI mapping... ");
        Flush();

        {
            Callback cb = new Callback();
            thrower.begin_throwLocalException().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            thrower.begin_throwLocalExceptionIdempotent().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        WriteLine("ok");

        Write("catching unknown non-Ice exception with new AMI mapping... ");
        Flush();

        {
            Callback cb = new Callback();
            thrower.begin_throwNonIceException().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        WriteLine("ok");

        // ----------------------------------------
        if(thrower.supportsUndeclaredExceptions())
        {
            Write("catching unknown user exception with new AMI mapping... ");
            Flush();

            {
                Callback cb = new Callback();
                thrower.begin_throwUndeclaredA(1).whenCompleted(
                    () =>
                    {
                        test(false);
                    },
                    (Ice.Exception exc) =>
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
                        cb.called();
                    });
                cb.check();
            }

            {
                Callback cb = new Callback();
                thrower.begin_throwUndeclaredB(1, 2).whenCompleted(
                    () =>
                    {
                        test(false);
                    },
                    (Ice.Exception exc) =>
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
                        cb.called();
                    });
                cb.check();
            }

            {
                Callback cb = new Callback();
                thrower.begin_throwUndeclaredC(1, 2, 3).whenCompleted(
                    () =>
                    {
                        test(false);
                    },
                    (Ice.Exception exc) =>
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
                        cb.called();
                    });
                cb.check();
            }

            WriteLine("ok");
        }

        Write("catching object not exist exception with new AMI mapping... ");
        Flush();

        {
            Ice.Identity id = communicator.stringToIdentity("does not exist");
            ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower.ice_identity(id));
            Callback cb = new Callback();
            thrower2.begin_throwAasA(1).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
                {
                    try
                    {
                        throw exc;
                    }
                    catch(Ice.ObjectNotExistException ex)
                    {
                        test(ex.id.Equals(id));
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

        Write("catching facet not exist exception with new AMI mapping... ");
        Flush();

        {
            ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower, "no such facet");
            Callback cb = new Callback();
            thrower2.begin_throwAasA(1).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        WriteLine("ok");

        Write("catching operation not exist exception with new AMI mapping... ");
        Flush();

        {
            Callback cb = new Callback();
            WrongOperationPrx thrower4 = WrongOperationPrxHelper.uncheckedCast(thrower);
            thrower4.begin_noSuchOperation().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        WriteLine("ok");

        Write("catching unknown local exception with new AMI mapping... ");
        Flush();

        {
            Callback cb = new Callback();
            thrower.begin_throwLocalException().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            thrower.begin_throwLocalExceptionIdempotent().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        WriteLine("ok");

        Write("catching unknown non-Ice exception with new AMI mapping... ");
        Flush();

        {
            Callback cb = new Callback();
            thrower.begin_throwNonIceException().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception exc) =>
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
                    cb.called();
                });
            cb.check();
        }

        WriteLine("ok");
#if SILVERLIGHT
        thrower.shutdown();
#else
        return thrower;
#endif
    }
}
