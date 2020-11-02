// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.IO;
using Test;

namespace ZeroC.Ice.Test.Exceptions
{
    public static class AllTests
    {
        public static IThrowerPrx Run(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator;
            TestHelper.Assert(communicator != null);
            bool ice1 = helper.Protocol == Protocol.Ice1;
            TextWriter output = helper.Output;
            {
                output.Write("testing object adapter registration exceptions... ");
                ObjectAdapter first;
                try
                {
                    first = communicator.CreateObjectAdapter("TestAdapter0");
                }
                catch (InvalidConfigurationException)
                {
                    // Expected
                }

                communicator.SetProperty("TestAdapter0.Endpoints", "tcp -h *");
                first = communicator.CreateObjectAdapter("TestAdapter0");
                try
                {
                    communicator.CreateObjectAdapter("TestAdapter0");
                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                    // Expected.
                }

                try
                {
                    _ = communicator.CreateObjectAdapterWithEndpoints("TestAdapter0", "ssl -h foo -p 12011");
                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                    // Expected
                }
                first.Dispose();
                output.WriteLine("ok");
            }

            {
                output.Write("testing servant registration exceptions... ");
                communicator.SetProperty("TestAdapter1.Endpoints", "tcp -h *");
                ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter1");
                var obj = new Empty();
                adapter.Add("x", obj);
                try
                {
                    adapter.Add("x", obj);
                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                }

                try
                {
                    adapter.Add("", obj);
                    TestHelper.Assert(false);
                }
                catch (FormatException)
                {
                }

                adapter.Remove("x");
                adapter.Remove("x"); // as of Ice 4.0, Remove succeeds with multiple removals
                adapter.Dispose();
                output.WriteLine("ok");
            }

            var thrower = IThrowerPrx.Parse(helper.GetTestProxy("thrower", 0), communicator);
            TestHelper.Assert(thrower != null);
            output.Write("catching exact types... ");
            output.Flush();

            try
            {
                thrower.ThrowAasA(1);
                TestHelper.Assert(false);
            }
            catch (A ex)
            {
                TestHelper.Assert(ex.AMem == 1);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                TestHelper.Assert(false);
            }

            try
            {
                thrower.ThrowAorDasAorD(1);
                TestHelper.Assert(false);
            }
            catch (A ex)
            {
                TestHelper.Assert(ex.AMem == 1);
            }
            catch
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.ThrowAorDasAorD(-1);
                TestHelper.Assert(false);
            }
            catch (D ex)
            {
                TestHelper.Assert(ex.DMem == -1);
            }
            catch
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.ThrowBasB(1, 2);
                TestHelper.Assert(false);
            }
            catch (B ex)
            {
                TestHelper.Assert(ex.AMem == 1);
                TestHelper.Assert(ex.BMem == 2);
            }
            catch
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.ThrowCasC(1, 2, 3);
                TestHelper.Assert(false);
            }
            catch (C ex)
            {
                TestHelper.Assert(ex.AMem == 1);
                TestHelper.Assert(ex.BMem == 2);
                TestHelper.Assert(ex.CMem == 3);
            }
            catch
            {
                TestHelper.Assert(false);
            }

            output.WriteLine("ok");

            output.Write("catching base types... ");
            output.Flush();

            try
            {
                thrower.ThrowBasB(1, 2);
                TestHelper.Assert(false);
            }
            catch (A ex)
            {
                TestHelper.Assert(ex.AMem == 1);
            }
            catch
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.ThrowCasC(1, 2, 3);
                TestHelper.Assert(false);
            }
            catch (B ex)
            {
                TestHelper.Assert(ex.AMem == 1);
                TestHelper.Assert(ex.BMem == 2);
            }
            catch
            {
                TestHelper.Assert(false);
            }

            output.WriteLine("ok");

            output.Write("catching derived types... ");
            output.Flush();

            try
            {
                thrower.ThrowBasA(1, 2);
                TestHelper.Assert(false);
            }
            catch (B ex)
            {
                TestHelper.Assert(ex.AMem == 1);
                TestHelper.Assert(ex.BMem == 2);
            }
            catch
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.ThrowCasA(1, 2, 3);
                TestHelper.Assert(false);
            }
            catch (C ex)
            {
                TestHelper.Assert(ex.AMem == 1);
                TestHelper.Assert(ex.BMem == 2);
                TestHelper.Assert(ex.CMem == 3);
            }
            catch
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.ThrowCasB(1, 2, 3);
                TestHelper.Assert(false);
            }
            catch (C ex)
            {
                TestHelper.Assert(ex.AMem == 1);
                TestHelper.Assert(ex.BMem == 2);
                TestHelper.Assert(ex.CMem == 3);
            }
            catch
            {
                TestHelper.Assert(false);
            }

            output.WriteLine("ok");

            if (thrower.GetConnection() is not ColocatedConnection)
            {
                output.Write("testing memory limit marshal exception...");
                output.Flush();
                try
                {
                    thrower.ThrowMemoryLimitException(Array.Empty<byte>());
                    TestHelper.Assert(false);
                }
                catch (InvalidDataException)
                {
                }
                catch
                {
                    TestHelper.Assert(false);
                }

                try
                {
                    thrower.ThrowMemoryLimitException(new byte[20 * 1024]); // 20KB
                    TestHelper.Assert(false);
                }
                catch (ConnectionLostException)
                {
                }
                catch (UnhandledException)
                {
                    // Expected with JS bidir server
                }
                catch
                {
                    TestHelper.Assert(false);
                }

                try
                {
                    var thrower2 = IThrowerPrx.Parse(helper.GetTestProxy("thrower", 1), communicator);
                    try
                    {
                        thrower2.ThrowMemoryLimitException(new byte[2 * 1024 * 1024]); // 2MB(no limits)
                    }
                    catch (InvalidDataException)
                    {
                    }

                    var thrower3 = IThrowerPrx.Parse(helper.GetTestProxy("thrower", 2), communicator);
                    try
                    {
                        thrower3.ThrowMemoryLimitException(new byte[1024]); // 1KB limit
                        TestHelper.Assert(false);
                    }
                    catch (ConnectionLostException)
                    {
                    }
                }
                catch (ConnectionRefusedException)
                {
                    // Expected with JS bidir server
                }

                output.WriteLine("ok");
            }

            output.Write("catching object not exist exception... ");
            output.Flush();

            {
                var identity = Identity.Parse("does not exist");
                try
                {
                    IThrowerPrx thrower2 = thrower.Clone(IThrowerPrx.Factory, identity: identity);
                    thrower2.IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException ex)
                {
                    TestHelper.Assert(ex.Origin!.Value.Identity == identity);
                    TestHelper.Assert(ex.Message.Contains("servant")); // verify we don't get system message
                }
                catch
                {
                    TestHelper.Assert(false);
                }
            }

            output.WriteLine("ok");

            output.Write("catching object not exist exception... ");
            output.Flush();

            try
            {
                IThrowerPrx thrower2 = thrower.Clone(IThrowerPrx.Factory, facet: "no such facet");
                try
                {
                    thrower2.IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException ex)
                {
                    TestHelper.Assert(ex.Origin!.Value.Facet == "no such facet");
                    TestHelper.Assert(ex.Message.Contains("with facet")); // verify we don't get system message
                }
            }
            catch
            {
                TestHelper.Assert(false);
            }

            output.WriteLine("ok");

            output.Write("catching operation not exist exception... ");
            output.Flush();

            try
            {
                var thrower2 = thrower.Clone(IWrongOperationPrx.Factory);
                thrower2.NoSuchOperation();
                TestHelper.Assert(false);
            }
            catch (OperationNotExistException ex)
            {
                TestHelper.Assert(ex.Origin!.Value.Operation == "noSuchOperation");
                TestHelper.Assert(ex.Message.Contains("could not find operation")); // verify we don't get system message
            }
            catch
            {
                TestHelper.Assert(false);
            }

            output.WriteLine("ok");

            output.Write("catching unhandled local exception... ");
            output.Flush();

            try
            {
                thrower.ThrowLocalException();
                TestHelper.Assert(false);
            }
            catch (UnhandledException ex)
            {
                TestHelper.Assert(ex.Message.Contains("unhandled exception")); // verify we get custom message

                // With ice1, the origin is not set; with ice2, it is.
                if (ice1)
                {
                    TestHelper.Assert(ex.Origin == null);
                }
                else
                {
                    TestHelper.Assert(ex.Origin!.Value.Identity == thrower.Identity &&
                                      ex.Origin!.Value.Operation == "throwLocalException");
                }
            }
            catch
            {
                TestHelper.Assert(false);
            }
            try
            {
                thrower.ThrowLocalExceptionIdempotent();
                TestHelper.Assert(false);
            }
            catch (UnhandledException)
            {
            }
            catch
            {
                TestHelper.Assert(false);
            }

            output.WriteLine("ok");

            output.Write("catching unhandled non-Ice exception... ");
            output.Flush();
            try
            {
                thrower.ThrowNonIceException();
                TestHelper.Assert(false);
            }
            catch (UnhandledException)
            {
            }
            catch
            {
                TestHelper.Assert(false);
            }
            output.WriteLine("ok");

            output.Write("catching unhandled remote exception... ");
            output.Flush();
            try
            {
                thrower.ThrowAConvertedToUnhandled();
                TestHelper.Assert(false);
            }
            catch (UnhandledException)
            {
            }
            catch
            {
                TestHelper.Assert(false);
            }
            output.WriteLine("ok");

            output.Write("testing asynchronous exceptions... ");
            output.Flush();

            try
            {
                thrower.ThrowAfterResponse();
            }
            catch
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.ThrowAfterException();
                TestHelper.Assert(false);
            }
            catch (A)
            {
            }
            catch
            {
                TestHelper.Assert(false);
            }
            output.WriteLine("ok");

            output.Write("catching exact types with AMI... ");
            output.Flush();
            {
                try
                {
                    thrower.ThrowAasAAsync(1).Wait();
                }
                catch (AggregateException ex)
                {
                    TestHelper.Assert(ex.InnerException != null);
                    TestHelper.Assert(((A)ex.InnerException).AMem == 1);
                }
            }

            {
                try
                {
                    thrower.ThrowAorDasAorDAsync(1).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (A ex)
                    {
                        TestHelper.Assert(ex.AMem == 1);
                    }
                    catch (D ex)
                    {
                        TestHelper.Assert(ex.DMem == -1);
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            {
                try
                {
                    thrower.ThrowAorDasAorDAsync(-1).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (A ex)
                    {
                        TestHelper.Assert(ex.AMem == 1);
                    }
                    catch (D ex)
                    {
                        TestHelper.Assert(ex.DMem == -1);
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            {
                try
                {
                    thrower.ThrowBasBAsync(1, 2).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (B ex)
                    {
                        TestHelper.Assert(ex.AMem == 1);
                        TestHelper.Assert(ex.BMem == 2);
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            {
                try
                {
                    thrower.ThrowCasCAsync(1, 2, 3).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (C ex)
                    {
                        TestHelper.Assert(ex.AMem == 1);
                        TestHelper.Assert(ex.BMem == 2);
                        TestHelper.Assert(ex.CMem == 3);
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching derived types with AMI... ");
            output.Flush();

            {
                try
                {
                    thrower.ThrowBasAAsync(1, 2).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (B ex)
                    {
                        TestHelper.Assert(ex.AMem == 1);
                        TestHelper.Assert(ex.BMem == 2);
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            {
                try
                {
                    thrower.ThrowCasAAsync(1, 2, 3).Wait();
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (C ex)
                    {
                        TestHelper.Assert(ex.AMem == 1);
                        TestHelper.Assert(ex.BMem == 2);
                        TestHelper.Assert(ex.CMem == 3);
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            {
                try
                {
                    thrower.ThrowCasBAsync(1, 2, 3).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (C ex)
                    {
                        TestHelper.Assert(ex.AMem == 1);
                        TestHelper.Assert(ex.BMem == 2);
                        TestHelper.Assert(ex.CMem == 3);
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching object not exist exception with AMI... ");
            output.Flush();

            {
                var identity = Identity.Parse("does not exist");
                IThrowerPrx thrower2 = thrower.Clone(IThrowerPrx.Factory, identity: identity);
                try
                {
                    thrower2.ThrowAasAAsync(1).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (ObjectNotExistException ex)
                    {
                        TestHelper.Assert(ex.Origin!.Value.Identity == identity);
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching object not exist exception with AMI... ");
            output.Flush();

            {
                IThrowerPrx thrower2 = thrower.Clone(IThrowerPrx.Factory, facet: "no such facet");
                try
                {
                    thrower2.ThrowAasAAsync(1).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (ObjectNotExistException ex)
                    {
                        TestHelper.Assert(ex.Origin!.Value.Facet.Equals("no such facet"));
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching operation not exist exception with AMI... ");
            output.Flush();

            {
                try
                {
                    var thrower4 = thrower.Clone(IWrongOperationPrx.Factory);
                    thrower4.NoSuchOperationAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (OperationNotExistException ex)
                    {
                        TestHelper.Assert(ex.Origin!.Value.Operation.Equals("noSuchOperation"));
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }
            output.WriteLine("ok");

            output.Write("catching unhandled local exception with AMI... ");
            output.Flush();

            {
                try
                {
                    thrower.ThrowLocalExceptionAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (UnhandledException)
                    {
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            {
                try
                {
                    thrower.ThrowLocalExceptionIdempotentAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (UnhandledException)
                    {
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching unhandled non-Ice exception with AMI... ");
            output.Flush();
            {
                try
                {
                    thrower.ThrowNonIceExceptionAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (UnhandledException)
                    {
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }
            output.WriteLine("ok");

            output.Write("catching object not exist exception with AMI... ");
            output.Flush();

            {
                var identity = Identity.Parse("does not exist");
                IThrowerPrx thrower2 = thrower.Clone(IThrowerPrx.Factory, identity: identity);
                try
                {
                    thrower2.ThrowAasAAsync(1).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    TestHelper.Assert(exc.InnerException != null);
                    try
                    {
                        throw exc.InnerException;
                    }
                    catch (ObjectNotExistException ex)
                    {
                        TestHelper.Assert(ex.Origin!.Value.Identity == identity);
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching object not exist exception with AMI... ");
            output.Flush();

            {
                IThrowerPrx thrower2 = thrower.Clone(IThrowerPrx.Factory, facet: "no such facet");
                try
                {
                    thrower2.ThrowAasAAsync(1).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (ObjectNotExistException ex)
                    {
                        TestHelper.Assert(ex.Origin!.Value.Facet == "no such facet");
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching operation not exist exception with AMI... ");
            output.Flush();

            {
                var thrower4 = thrower.Clone(IWrongOperationPrx.Factory);
                try
                {
                    thrower4.NoSuchOperationAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (OperationNotExistException ex)
                    {
                        TestHelper.Assert(ex.Origin!.Value.Operation == "noSuchOperation");
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching unhandled local exception with AMI... ");
            output.Flush();

            {
                try
                {
                    thrower.ThrowLocalExceptionAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (UnhandledException)
                    {
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            {
                try
                {
                    thrower.ThrowLocalExceptionIdempotentAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (UnhandledException)
                    {
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching unhandled non-Ice exception with AMI... ");
            output.Flush();
            {
                try
                {
                    thrower.ThrowNonIceExceptionAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (UnhandledException)
                    {
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }
            output.WriteLine("ok");

            output.Write("catching unhandled remote exception with AMI... ");
            output.Flush();
            {
                try
                {
                    thrower.ThrowAConvertedToUnhandledAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (UnhandledException)
                    {
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                }
            }
            output.WriteLine("ok");

            return thrower;
        }
    }
}
