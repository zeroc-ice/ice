//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

namespace ZeroC.Ice.Test.Exceptions
{
    public class AllTests
    {
        public static IThrowerPrx allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            System.IO.TextWriter output = helper.GetWriter();
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
                    ObjectAdapter second =
                        communicator.CreateObjectAdapterWithEndpoints("TestAdapter0", "ssl -h foo -p 12011");
                    TestHelper.Assert(false);

                    //
                    // Quell mono error that variable second isn't used.
                    //
                    second.Dispose();
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
                adapter.Remove("x"); // as of Ice 4.0, can remove multiple times
                adapter.Dispose();
                output.WriteLine("ok");
            }

            var thrower = IThrowerPrx.Parse($"thrower:{helper.GetTestEndpoint(0)}", communicator);
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
            catch (Exception)
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
            catch (Exception)
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
            catch (Exception)
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
            catch (Exception)
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
            catch (Exception)
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
            catch (Exception)
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
            catch (Exception)
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
            catch (Exception)
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
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            output.WriteLine("ok");

            output.Write("catching remote exception... ");
            output.Flush();

            try
            {
                thrower.ThrowUndeclaredA(1);
                TestHelper.Assert(false);
            }
            catch (A)
            {
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.ThrowUndeclaredB(1, 2);
                TestHelper.Assert(false);
            }
            catch (B)
            {
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.ThrowUndeclaredC(1, 2, 3);
                TestHelper.Assert(false);
            }
            catch (C)
            {
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            output.WriteLine("ok");

            if (thrower.GetConnection() != null)
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
                catch (Exception)
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
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }

                try
                {
                    var thrower2 = IThrowerPrx.Parse("thrower:" + helper.GetTestEndpoint(1), communicator);
                    try
                    {
                        thrower2.ThrowMemoryLimitException(new byte[2 * 1024 * 1024]); // 2MB(no limits)
                    }
                    catch (InvalidDataException)
                    {
                    }
                    var thrower3 = IThrowerPrx.Parse("thrower:" + helper.GetTestEndpoint(2), communicator);
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
                    IThrowerPrx thrower2 = thrower.Clone(identity, IThrowerPrx.Factory);
                    thrower2.IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException ex)
                {
                    TestHelper.Assert(ex.Id.Equals(identity));
                    TestHelper.Assert(ex.Message.Contains("servant")); // verify we don't get system message
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }

            output.WriteLine("ok");

            output.Write("catching object not exist exception... ");
            output.Flush();

            try
            {
                IThrowerPrx thrower2 = thrower.Clone(facet: "no such facet", IThrowerPrx.Factory);
                try
                {
                    thrower2.IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException ex)
                {
                    TestHelper.Assert(ex.Facet.Equals("no such facet"));
                    TestHelper.Assert(ex.Message.Contains("with facet")); // verify we don't get system message
                }
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            output.WriteLine("ok");

            output.Write("catching operation not exist exception... ");
            output.Flush();

            try
            {
                var thrower2 = IWrongOperationPrx.UncheckedCast(thrower);
                thrower2.NoSuchOperation();
                TestHelper.Assert(false);
            }
            catch (OperationNotExistException ex)
            {
                TestHelper.Assert(ex.Operation.Equals("noSuchOperation"));
                TestHelper.Assert(ex.Message.Contains("could not find operation")); // verify we don't get system message
            }
            catch (Exception)
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
            }
            catch (Exception)
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
            catch (Exception)
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
            catch (Exception)
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
            catch (Exception)
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
            catch (Exception)
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
            catch (Exception)
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
                    catch (Exception)
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
                    catch (Exception)
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
                    catch (Exception)
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
                    catch (Exception)
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
                    catch (Exception)
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
                    catch (Exception)
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
                    catch (Exception)
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching remote exception with AMI... ");
            output.Flush();

            {
                try
                {
                    thrower.ThrowUndeclaredAAsync(1).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (A)
                    {
                    }
                    catch (Exception)
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            {
                try
                {
                    thrower.ThrowUndeclaredBAsync(1, 2).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (B)
                    {
                    }
                    catch (Exception)
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            {
                try
                {
                    thrower.ThrowUndeclaredCAsync(1, 2, 3).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (C)
                    {
                    }
                    catch (Exception)
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
                IThrowerPrx thrower2 = thrower.Clone(identity, IThrowerPrx.Factory);
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
                        TestHelper.Assert(ex.Id.Equals(identity));
                    }
                    catch (Exception)
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching object not exist exception with AMI... ");
            output.Flush();

            {
                IThrowerPrx thrower2 = thrower.Clone(facet: "no such facet", IThrowerPrx.Factory);
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
                        TestHelper.Assert(ex.Facet.Equals("no such facet"));
                    }
                    catch (Exception)
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
                    var thrower4 = IWrongOperationPrx.UncheckedCast(thrower);
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
                        TestHelper.Assert(ex.Operation.Equals("noSuchOperation"));
                    }
                    catch (Exception)
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
                    catch (Exception)
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
                    catch (Exception)
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
                    catch (Exception)
                    {
                        TestHelper.Assert(false);
                    }
                }
            }
            output.WriteLine("ok");

            output.Write("catching remote exception with AMI... ");
            output.Flush();

            {
                try
                {
                    thrower.ThrowUndeclaredAAsync(1).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (A)
                    {
                    }
                    catch (Exception)
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            {
                try
                {
                    thrower.ThrowUndeclaredBAsync(1, 2).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (B)
                    {
                    }
                    catch (Exception)
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            {
                try
                {
                    thrower.ThrowUndeclaredCAsync(1, 2, 3).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException exc)
                {
                    try
                    {
                        TestHelper.Assert(exc.InnerException != null);
                        throw exc.InnerException;
                    }
                    catch (C)
                    {
                    }
                    catch (Exception)
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
                IThrowerPrx thrower2 = thrower.Clone(identity, IThrowerPrx.Factory);
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
                        TestHelper.Assert(ex.Id.Equals(identity));
                    }
                    catch (Exception)
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching object not exist exception with AMI... ");
            output.Flush();

            {
                IThrowerPrx thrower2 = thrower.Clone(facet: "no such facet", IThrowerPrx.Factory);
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
                        TestHelper.Assert(ex.Facet.Equals("no such facet"));
                    }
                    catch (Exception)
                    {
                        TestHelper.Assert(false);
                    }
                }
            }

            output.WriteLine("ok");

            output.Write("catching operation not exist exception with AMI... ");
            output.Flush();

            {
                var thrower4 = IWrongOperationPrx.UncheckedCast(thrower);
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
                        TestHelper.Assert(ex.Operation.Equals("noSuchOperation"));
                    }
                    catch (Exception)
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
                    catch (Exception)
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
                    catch (Exception)
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
                    catch (Exception)
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
                    catch (Exception)
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
