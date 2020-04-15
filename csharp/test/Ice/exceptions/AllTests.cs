//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.exceptions.Test;
using System;
using Test;

namespace Ice.exceptions
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
                    second.Deactivate();
                }
                catch (ArgumentException)
                {
                    // Expected
                }
                first.Deactivate();
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
                adapter.Deactivate();
                output.WriteLine("ok");
            }

            var thrower = IThrowerPrx.Parse($"thrower:{helper.GetTestEndpoint(0)}", communicator);
            TestHelper.Assert(thrower != null);
            output.Write("catching exact types... ");
            output.Flush();

            try
            {
                thrower.throwAasA(1);
                TestHelper.Assert(false);
            }
            catch (A ex)
            {
                TestHelper.Assert(ex.aMem == 1);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                TestHelper.Assert(false);
            }

            try
            {
                thrower.throwAorDasAorD(1);
                TestHelper.Assert(false);
            }
            catch (A ex)
            {
                TestHelper.Assert(ex.aMem == 1);
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.throwAorDasAorD(-1);
                TestHelper.Assert(false);
            }
            catch (D ex)
            {
                TestHelper.Assert(ex.dMem == -1);
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.throwBasB(1, 2);
                TestHelper.Assert(false);
            }
            catch (B ex)
            {
                TestHelper.Assert(ex.aMem == 1);
                TestHelper.Assert(ex.bMem == 2);
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.throwCasC(1, 2, 3);
                TestHelper.Assert(false);
            }
            catch (C ex)
            {
                TestHelper.Assert(ex.aMem == 1);
                TestHelper.Assert(ex.bMem == 2);
                TestHelper.Assert(ex.cMem == 3);
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
                thrower.throwBasB(1, 2);
                TestHelper.Assert(false);
            }
            catch (A ex)
            {
                TestHelper.Assert(ex.aMem == 1);
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.throwCasC(1, 2, 3);
                TestHelper.Assert(false);
            }
            catch (B ex)
            {
                TestHelper.Assert(ex.aMem == 1);
                TestHelper.Assert(ex.bMem == 2);
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
                thrower.throwBasA(1, 2);
                TestHelper.Assert(false);
            }
            catch (B ex)
            {
                TestHelper.Assert(ex.aMem == 1);
                TestHelper.Assert(ex.bMem == 2);
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.throwCasA(1, 2, 3);
                TestHelper.Assert(false);
            }
            catch (C ex)
            {
                TestHelper.Assert(ex.aMem == 1);
                TestHelper.Assert(ex.bMem == 2);
                TestHelper.Assert(ex.cMem == 3);
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.throwCasB(1, 2, 3);
                TestHelper.Assert(false);
            }
            catch (C ex)
            {
                TestHelper.Assert(ex.aMem == 1);
                TestHelper.Assert(ex.bMem == 2);
                TestHelper.Assert(ex.cMem == 3);
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
                thrower.throwUndeclaredA(1);
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
                thrower.throwUndeclaredB(1, 2);
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
                thrower.throwUndeclaredC(1, 2, 3);
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
                    thrower.throwMemoryLimitException(Array.Empty<byte>());
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
                    thrower.throwMemoryLimitException(new byte[20 * 1024]); // 20KB
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
                        thrower2.throwMemoryLimitException(new byte[2 * 1024 * 1024]); // 2MB(no limits)
                    }
                    catch (InvalidDataException)
                    {
                    }
                    var thrower3 = IThrowerPrx.Parse("thrower:" + helper.GetTestEndpoint(2), communicator);
                    try
                    {
                        thrower3.throwMemoryLimitException(new byte[1024]); // 1KB limit
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
                thrower2.noSuchOperation();
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
                thrower.throwLocalException();
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
                thrower.throwLocalExceptionIdempotent();
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
                thrower.throwNonIceException();
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
                thrower.throwAConvertedToUnhandled();
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
                thrower.throwAfterResponse();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                thrower.throwAfterException();
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
                    thrower.throwAasAAsync(1).Wait();
                }
                catch (AggregateException ex)
                {
                    TestHelper.Assert(ex.InnerException != null);
                    TestHelper.Assert(((A)ex.InnerException).aMem == 1);
                }
            }

            {
                try
                {
                    thrower.throwAorDasAorDAsync(1).Wait();
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
                        TestHelper.Assert(ex.aMem == 1);
                    }
                    catch (D ex)
                    {
                        TestHelper.Assert(ex.dMem == -1);
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
                    thrower.throwAorDasAorDAsync(-1).Wait();
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
                        TestHelper.Assert(ex.aMem == 1);
                    }
                    catch (D ex)
                    {
                        TestHelper.Assert(ex.dMem == -1);
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
                    thrower.throwBasBAsync(1, 2).Wait();
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
                        TestHelper.Assert(ex.aMem == 1);
                        TestHelper.Assert(ex.bMem == 2);
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
                    thrower.throwCasCAsync(1, 2, 3).Wait();
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
                        TestHelper.Assert(ex.aMem == 1);
                        TestHelper.Assert(ex.bMem == 2);
                        TestHelper.Assert(ex.cMem == 3);
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
                    thrower.throwBasAAsync(1, 2).Wait();
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
                        TestHelper.Assert(ex.aMem == 1);
                        TestHelper.Assert(ex.bMem == 2);
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
                    thrower.throwCasAAsync(1, 2, 3).Wait();
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
                        TestHelper.Assert(ex.aMem == 1);
                        TestHelper.Assert(ex.bMem == 2);
                        TestHelper.Assert(ex.cMem == 3);
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
                    thrower.throwCasBAsync(1, 2, 3).Wait();
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
                        TestHelper.Assert(ex.aMem == 1);
                        TestHelper.Assert(ex.bMem == 2);
                        TestHelper.Assert(ex.cMem == 3);
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
                    thrower.throwUndeclaredAAsync(1).Wait();
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
                    thrower.throwUndeclaredBAsync(1, 2).Wait();
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
                    thrower.throwUndeclaredCAsync(1, 2, 3).Wait();
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
                    thrower2.throwAasAAsync(1).Wait();
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
                    thrower2.throwAasAAsync(1).Wait();
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
                    thrower4.noSuchOperationAsync().Wait();
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
                    thrower.throwLocalExceptionAsync().Wait();
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
                    thrower.throwLocalExceptionIdempotentAsync().Wait();
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
                    thrower.throwNonIceExceptionAsync().Wait();
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
                    thrower.throwUndeclaredAAsync(1).Wait();
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
                    thrower.throwUndeclaredBAsync(1, 2).Wait();
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
                    thrower.throwUndeclaredCAsync(1, 2, 3).Wait();
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
                    thrower2.throwAasAAsync(1).Wait();
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
                    thrower2.throwAasAAsync(1).Wait();
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
                    thrower4.noSuchOperationAsync().Wait();
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
                    thrower.throwLocalExceptionAsync().Wait();
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
                    thrower.throwLocalExceptionIdempotentAsync().Wait();
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
                    thrower.throwNonIceExceptionAsync().Wait();
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
                    thrower.throwAConvertedToUnhandledAsync().Wait();
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
