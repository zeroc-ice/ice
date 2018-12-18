// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;

namespace Ice
{
    namespace exceptions
    {
        public class AllTests : global::Test.AllTests
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

            public static Test.ThrowerPrx allTests(global::Test.TestHelper helper)
            {
                Ice.Communicator communicator = helper.communicator();
                var output = helper.getWriter();
                {
                    output.Write("testing object adapter registration exceptions... ");
                    Ice.ObjectAdapter first;
                    try
                    {
                        first = communicator.createObjectAdapter("TestAdapter0");
                    }
                    catch(Ice.InitializationException)
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
                    output.WriteLine("ok");
                }

                {
                    output.Write("testing servant registration exceptions... ");
                    communicator.getProperties().setProperty("TestAdapter1.Endpoints", "tcp -h *");
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter1");
                    Ice.Object obj = new EmptyI();
                    adapter.add(obj, Ice.Util.stringToIdentity("x"));
                    try
                    {
                        adapter.add(obj, Ice.Util.stringToIdentity("x"));
                        test(false);
                    }
                    catch(Ice.AlreadyRegisteredException)
                    {
                    }

                    try
                    {
                        adapter.add(obj, Ice.Util.stringToIdentity(""));
                        test(false);
                    }
                    catch(Ice.IllegalIdentityException e)
                    {
                        test(e.id.name.Equals(""));
                    }

                    try
                    {
                        adapter.add(null, Ice.Util.stringToIdentity("x"));
                        test(false);
                    }
                    catch(Ice.IllegalServantException)
                    {
                    }

                    adapter.remove(Ice.Util.stringToIdentity("x"));
                    try
                    {
                        adapter.remove(Ice.Util.stringToIdentity("x"));
                        test(false);
                    }
                    catch(Ice.NotRegisteredException)
                    {
                    }
                    adapter.deactivate();
                    output.WriteLine("ok");
                }

                {
                    output.Write("testing servant locator registration exceptions... ");
                    communicator.getProperties().setProperty("TestAdapter2.Endpoints", "tcp -h *");
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
                    output.WriteLine("ok");
                }

                {
                    output.Write("testing object factory registration exception... ");
                    communicator.getValueFactoryManager().add(_ => { return null; }, "::x");
                    try
                    {
                        communicator.getValueFactoryManager().add(_ => { return null; }, "::x");
                        test(false);
                    }
                    catch(Ice.AlreadyRegisteredException)
                    {
                    }
                    output.WriteLine("ok");
                }

                output.Write("testing stringToProxy... ");
                output.Flush();
                String @ref = "thrower:" + helper.getTestEndpoint(0);
                Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
                test(@base != null);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                var thrower = Test.ThrowerPrxHelper.checkedCast(@base);

                test(thrower != null);
                test(thrower.Equals(@base));
                output.WriteLine("ok");

                output.Write("catching exact types... ");
                output.Flush();

                try
                {
                    thrower.throwAasA(1);
                    test(false);
                }
                catch(Test.A ex)
                {
                    test(ex.aMem == 1);
                }
                catch(Exception ex)
                {
                    Console.WriteLine(ex);
                    test(false);
                }

                try
                {
                    thrower.throwAorDasAorD(1);
                    test(false);
                }
                catch(Test.A ex)
                {
                    test(ex.aMem == 1);
                }
                catch(Exception)
                {
                    test(false);
                }

                try
                {
                    thrower.throwAorDasAorD(-1);
                    test(false);
                }
                catch(Test.D ex)
                {
                    test(ex.dMem == -1);
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
                catch(Test.B ex)
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
                catch(Test.C ex)
                {
                    test(ex.aMem == 1);
                    test(ex.bMem == 2);
                    test(ex.cMem == 3);
                }
                catch(Exception)
                {
                    test(false);
                }

                output.WriteLine("ok");

                output.Write("catching base types... ");
                output.Flush();

                try
                {
                    thrower.throwBasB(1, 2);
                    test(false);
                }
                catch(Test.A ex)
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
                catch(Test.B ex)
                {
                    test(ex.aMem == 1);
                    test(ex.bMem == 2);
                }
                catch(Exception)
                {
                    test(false);
                }

                output.WriteLine("ok");

                output.Write("catching derived types... ");
                output.Flush();

                try
                {
                    thrower.throwBasA(1, 2);
                    test(false);
                }
                catch(Test.B ex)
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
                catch(Test.C ex)
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
                catch(Test.C ex)
                {
                    test(ex.aMem == 1);
                    test(ex.bMem == 2);
                    test(ex.cMem == 3);
                }
                catch(Exception)
                {
                    test(false);
                }

                output.WriteLine("ok");

                if(thrower.supportsUndeclaredExceptions())
                {
                    output.Write("catching unknown user exception... ");
                    output.Flush();

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

                    output.WriteLine("ok");
                }

                if(thrower.ice_getConnection() != null)
                {
                    output.Write("testing memory limit marshal exception...");
                    output.Flush();
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
                    catch(Ice.UnknownLocalException)
                    {
                        // Expected with JS bidir server
                    }
                    catch(Exception)
                    {
                        test(false);
                    }

                    try
                    {
                        var thrower2 = Test.ThrowerPrxHelper.uncheckedCast(
                            communicator.stringToProxy("thrower:" + helper.getTestEndpoint(1)));
                        try
                        {
                            thrower2.throwMemoryLimitException(new byte[2 * 1024 * 1024]); // 2MB(no limits)
                        }
                        catch(Ice.MemoryLimitException)
                        {
                        }
                        var thrower3 = Test.ThrowerPrxHelper.uncheckedCast(
                            communicator.stringToProxy("thrower:" + helper.getTestEndpoint(2)));
                        try
                        {
                            thrower3.throwMemoryLimitException(new byte[1024]); // 1KB limit
                            test(false);
                        }
                        catch(Ice.ConnectionLostException)
                        {
                        }
                    }
                    catch(Ice.ConnectionRefusedException)
                    {
                        // Expected with JS bidir server
                    }

                    output.WriteLine("ok");
                }

                output.Write("catching object not exist exception... ");
                output.Flush();

                {
                    Ice.Identity id = Ice.Util.stringToIdentity("does not exist");
                    try
                    {
                        var thrower2 = Test.ThrowerPrxHelper.uncheckedCast(thrower.ice_identity(id));
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

                output.WriteLine("ok");

                output.Write("catching facet not exist exception... ");
                output.Flush();

                try
                {
                    var thrower2 = Test.ThrowerPrxHelper.uncheckedCast(thrower, "no such facet");
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

                output.WriteLine("ok");

                output.Write("catching operation not exist exception... ");
                output.Flush();

                try
                {
                    var thrower2 = Test.WrongOperationPrxHelper.uncheckedCast(thrower);
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

                output.WriteLine("ok");

                output.Write("catching unknown local exception... ");
                output.Flush();

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

                output.WriteLine("ok");

                output.Write("catching unknown non-Ice exception... ");
                output.Flush();

                try
                {
                    thrower.throwNonIceException();
                    test(false);
                }
                catch(Ice.UnknownException)
                {
                }
                catch(Exception)
                {
                    test(false);
                }

                output.WriteLine("ok");

                output.Write("testing asynchronous exceptions... ");
                output.Flush();

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
                catch(Test.A)
                {
                }
                catch(Exception)
                {
                    test(false);
                }

                output.WriteLine("ok");

                output.Write("catching exact types with AMI mapping... ");
                output.Flush();

                {
                    Callback cb = new Callback();
                    thrower.begin_throwAasA(1).whenCompleted(
                       () =>
                        {
                            test(false);
                        },
                       (Ice.Exception exc) =>
                        {
                            test(exc is Test.A);
                            var ex = exc as Test.A;
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
                            catch(Test.A ex)
                            {
                                test(ex.aMem == 1);
                            }
                            catch(Test.D ex)
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
                            catch(Test.A ex)
                            {
                                test(ex.aMem == 1);
                            }
                            catch(Test.D ex)
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
                            catch(Test.B ex)
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
                            catch(Test.C ex)
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

                output.WriteLine("ok");

                output.Write("catching derived types with new AMI mapping... ");
                output.Flush();

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
                            catch(Test.B ex)
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
                            catch(Test.C ex)
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
                            catch(Test.C ex)
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

                output.WriteLine("ok");

                if(thrower.supportsUndeclaredExceptions())
                {
                    output.Write("catching unknown user exception with new AMI mapping... ");
                    output.Flush();

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

                    output.WriteLine("ok");
                }

                output.Write("catching object not exist exception with new AMI mapping... ");
                output.Flush();

                {
                    Ice.Identity id = Ice.Util.stringToIdentity("does not exist");
                    var thrower2 = Test.ThrowerPrxHelper.uncheckedCast(thrower.ice_identity(id));
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

                output.WriteLine("ok");

                output.Write("catching facet not exist exception with new AMI mapping... ");
                output.Flush();

                {
                    var thrower2 = Test.ThrowerPrxHelper.uncheckedCast(thrower, "no such facet");
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

                output.WriteLine("ok");

                output.Write("catching operation not exist exception with new AMI mapping... ");
                output.Flush();

                {
                    Callback cb = new Callback();
                    var thrower4 = Test.WrongOperationPrxHelper.uncheckedCast(thrower);
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

                output.WriteLine("ok");

                output.Write("catching unknown local exception with new AMI mapping... ");
                output.Flush();

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

                output.WriteLine("ok");

                output.Write("catching unknown non-Ice exception with new AMI mapping... ");
                output.Flush();

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

                output.WriteLine("ok");

                if(thrower.supportsUndeclaredExceptions())
                {
                    output.Write("catching unknown user exception with new AMI mapping... ");
                    output.Flush();

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

                    output.WriteLine("ok");
                }

                output.Write("catching object not exist exception with new AMI mapping... ");
                output.Flush();

                {
                    Ice.Identity id = Ice.Util.stringToIdentity("does not exist");
                    var thrower2 = Test.ThrowerPrxHelper.uncheckedCast(thrower.ice_identity(id));
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

                output.WriteLine("ok");

                output.Write("catching facet not exist exception with new AMI mapping... ");
                output.Flush();

                {
                    var thrower2 = Test.ThrowerPrxHelper.uncheckedCast(thrower, "no such facet");
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

                output.WriteLine("ok");

                output.Write("catching operation not exist exception with new AMI mapping... ");
                output.Flush();

                {
                    Callback cb = new Callback();
                    var thrower4 = Test.WrongOperationPrxHelper.uncheckedCast(thrower);
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

                output.WriteLine("ok");

                output.Write("catching unknown local exception with new AMI mapping... ");
                output.Flush();

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

                output.WriteLine("ok");

                output.Write("catching unknown non-Ice exception with new AMI mapping... ");
                output.Flush();

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

                output.WriteLine("ok");
                return thrower;
            }
        }
    }
}
