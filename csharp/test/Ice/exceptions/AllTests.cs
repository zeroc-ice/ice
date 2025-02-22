// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.exceptions;
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
            lock (this)
            {
                while (!_called)
                {
                    Monitor.Wait(this);
                }

                _called = false;
            }
        }

        public virtual void called()
        {
            lock (this)
            {
                Debug.Assert(!_called);
                _called = true;
                Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

    public static async Task<Test.ThrowerPrx> allTests(global::Test.TestHelper helper)
    {
        Communicator communicator = helper.communicator();
        var output = helper.getWriter();
        {
            output.Write("testing object adapter registration exceptions... ");
            ObjectAdapter first;
            try
            {
                first = communicator.createObjectAdapter("TestAdapter0");
            }
            catch (InitializationException)
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
            catch (AlreadyRegisteredException)
            {
                // Expected.
            }

            try
            {
                ObjectAdapter second =
                    communicator.createObjectAdapterWithEndpoints("TestAdapter0", "ssl -h foo -p 12011");
                test(false);

                //
                // Quell mono error that variable second isn't used.
                //
                second.deactivate();
            }
            catch (AlreadyRegisteredException)
            {
                // Expected
            }
            first.deactivate();
            output.WriteLine("ok");
        }

        {
            output.Write("testing servant registration exceptions... ");
            communicator.getProperties().setProperty("TestAdapter1.Endpoints", "tcp -h *");
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter1");
            Object obj = new EmptyI();
            adapter.add(obj, Util.stringToIdentity("x"));
            try
            {
                adapter.add(obj, Util.stringToIdentity("x"));
                test(false);
            }
            catch (AlreadyRegisteredException)
            {
            }

            try
            {
                adapter.add(obj, Util.stringToIdentity(""));
                test(false);
            }
            catch (ArgumentException)
            {
            }

            try
            {
                adapter.add(null, Util.stringToIdentity("x"));
                test(false);
            }
            catch (ArgumentNullException)
            {
            }

            adapter.remove(Util.stringToIdentity("x"));
            try
            {
                adapter.remove(Util.stringToIdentity("x"));
                test(false);
            }
            catch (NotRegisteredException)
            {
            }
            adapter.deactivate();
            output.WriteLine("ok");
        }

        {
            output.Write("testing servant locator registration exceptions... ");
            communicator.getProperties().setProperty("TestAdapter2.Endpoints", "tcp -h *");
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter2");
            ServantLocator loc = new ServantLocatorI();
            adapter.addServantLocator(loc, "x");
            try
            {
                adapter.addServantLocator(loc, "x");
                test(false);
            }
            catch (AlreadyRegisteredException)
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
            catch (AlreadyRegisteredException)
            {
            }
            output.WriteLine("ok");
        }

        output.Write("testing stringToProxy... ");
        output.Flush();
        string @ref = "thrower:" + helper.getTestEndpoint(0);
        ObjectPrx @base = communicator.stringToProxy(@ref);
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
        catch (Test.A ex)
        {
            test(ex.aMem == 1);
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
            test(false);
        }

        try
        {
            thrower.throwAorDasAorD(1);
            test(false);
        }
        catch (Test.A ex)
        {
            test(ex.aMem == 1);
        }
        catch (Exception)
        {
            test(false);
        }

        try
        {
            thrower.throwAorDasAorD(-1);
            test(false);
        }
        catch (Test.D ex)
        {
            test(ex.dMem == -1);
        }
        catch (Exception)
        {
            test(false);
        }

        try
        {
            thrower.throwBasB(1, 2);
            test(false);
        }
        catch (Test.B ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        catch (Exception)
        {
            test(false);
        }

        try
        {
            thrower.throwCasC(1, 2, 3);
            test(false);
        }
        catch (Test.C ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        catch (Exception)
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
        catch (Test.A ex)
        {
            test(ex.aMem == 1);
        }
        catch (Exception)
        {
            test(false);
        }

        try
        {
            thrower.throwCasC(1, 2, 3);
            test(false);
        }
        catch (Test.B ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        catch (Exception)
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
        catch (Test.B ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        catch (Exception)
        {
            test(false);
        }

        try
        {
            thrower.throwCasA(1, 2, 3);
            test(false);
        }
        catch (Test.C ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        catch (Exception)
        {
            test(false);
        }

        try
        {
            thrower.throwCasB(1, 2, 3);
            test(false);
        }
        catch (Test.C ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        catch (Exception)
        {
            test(false);
        }

        output.WriteLine("ok");

        if (thrower.supportsUndeclaredExceptions())
        {
            output.Write("catching unknown user exception... ");
            output.Flush();

            try
            {
                thrower.throwUndeclaredA(1);
                test(false);
            }
            catch (UnknownUserException)
            {
            }
            catch (Exception)
            {
                test(false);
            }

            try
            {
                thrower.throwUndeclaredB(1, 2);
                test(false);
            }
            catch (UnknownUserException)
            {
            }
            catch (Exception)
            {
                test(false);
            }

            try
            {
                thrower.throwUndeclaredC(1, 2, 3);
                test(false);
            }
            catch (UnknownUserException)
            {
            }
            catch (Exception)
            {
                test(false);
            }

            output.WriteLine("ok");
        }

        if (thrower.ice_getConnection() != null)
        {
            output.Write("testing memory limit marshal exception...");
            output.Flush();
            try
            {
                thrower.throwMemoryLimitException(null);
                test(false);
            }
            catch (MarshalException ex)
            {
                test(ex.Message.Contains("exceeds the maximum allowed"));
            }
            catch (Exception)
            {
                test(false);
            }

            try
            {
                thrower.throwMemoryLimitException(new byte[20 * 1024]); // 20KB
                test(false);
            }
            catch (ConnectionLostException)
            {
            }
            catch (UnknownLocalException)
            {
                // Expected with JS bidir server
            }
            catch (Exception)
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
                catch (MarshalException ex)
                {
                    test(ex.Message.Contains("exceeds the maximum allowed"));
                }

                var thrower3 = Test.ThrowerPrxHelper.uncheckedCast(
                    communicator.stringToProxy("thrower:" + helper.getTestEndpoint(2)));
                try
                {
                    thrower3.throwMemoryLimitException(new byte[1024]); // 1KB limit
                    test(false);
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

        Identity id = Util.stringToIdentity("does not exist");
        try
        {
            var thrower2 = Test.ThrowerPrxHelper.uncheckedCast(thrower.ice_identity(id));
            thrower2.ice_ping();
            test(false);
        }
        catch (ObjectNotExistException ex)
        {
            test(ex.id.Equals(id));
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
            catch (FacetNotExistException ex)
            {
                test(ex.facet == "no such facet");
            }
        }
        catch (Exception)
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
        catch (OperationNotExistException ex)
        {
            test(ex.operation == "noSuchOperation");
        }

        output.WriteLine("ok");

        output.Write("catching unknown local exception... ");
        output.Flush();

        try
        {
            thrower.throwLocalException();
            test(false);
        }
        catch (UnknownLocalException)
        {
        }

        try
        {
            thrower.throwLocalExceptionIdempotent();
            test(false);
        }
        catch (UnknownLocalException)
        {
        }
        catch (OperationNotExistException)
        {
        }

        output.WriteLine("ok");

        output.Write("catching unknown non-Ice exception... ");
        output.Flush();

        try
        {
            thrower.throwNonIceException();
            test(false);
        }
        catch (UnknownException)
        {
        }

        output.WriteLine("ok");

        output.Write("catching dispatch exception... ");
        output.Flush();

        try
        {
            thrower.throwDispatchException((byte)ReplyStatus.OperationNotExist);
            test(false);
        }
        catch (OperationNotExistException ex) // remapped as expected
        {
            test(ex.Message ==
                "Dispatch failed with OperationNotExist { id = 'thrower', facet = '', operation = 'throwDispatchException' }");
        }

        try
        {
            thrower.throwDispatchException((byte)ReplyStatus.Unauthorized);
            test(false);
        }
        catch (DispatchException ex) when (ex.replyStatus == ReplyStatus.Unauthorized)
        {
            test(ex.Message == "The dispatch failed with reply status Unauthorized." ||
                ex.Message == "The dispatch failed with reply status unauthorized."); // for Swift
        }

        try
        {
            thrower.throwDispatchException(212);
            test(false);
        }
        catch (DispatchException ex) when (ex.replyStatus == (ReplyStatus)212)
        {
            test(ex.Message == "The dispatch failed with reply status 212.");
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
            test(false);
        }

        try
        {
            thrower.throwAfterException();
            test(false);
        }
        catch (Test.A)
        {
        }

        output.WriteLine("ok");

        output.Write("catching exact types with AMI mapping... ");
        output.Flush();

        try
        {
            await thrower.throwAasAAsync(1);
            test(false);
        }
        catch (Test.A ex)
        {
            test(ex.aMem == 1);
        }

        try
        {
            await thrower.throwAorDasAorDAsync(1);
            test(false);
        }
        catch (Test.A ex)
        {
            test(ex.aMem == 1);
        }
        catch (Test.D ex)
        {
            test(ex.dMem == -1);
        }

        try
        {
            await thrower.throwAorDasAorDAsync(-1);
            test(false);
        }
        catch (Test.A ex)
        {
            test(ex.aMem == 1);
        }
        catch (Test.D ex)
        {
            test(ex.dMem == -1);
        }

        try
        {
            await thrower.throwBasBAsync(1, 2);
            test(false);
        }
        catch (Test.B ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }

        try
        {
            await thrower.throwCasCAsync(1, 2, 3);
            test(false);
        }
        catch (Test.C ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }

        output.WriteLine("ok");

        output.Write("catching derived types with new AMI mapping... ");
        output.Flush();

        try
        {
            await thrower.throwBasAAsync(1, 2);
            test(false);
        }
        catch (Test.B ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        catch (Exception)
        {
            test(false);
        }

        try
        {
            await thrower.throwCasAAsync(1, 2, 3);
            test(false);
        }
        catch (Test.C ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }

        try
        {
            await thrower.throwCasBAsync(1, 2, 3);
            test(false);
        }
        catch (Test.C ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }

        output.WriteLine("ok");

        if (thrower.supportsUndeclaredExceptions())
        {
            output.Write("catching unknown user exception with new AMI mapping... ");
            output.Flush();

            try
            {
                await thrower.throwUndeclaredAAsync(1);
                test(false);
            }
            catch (UnknownUserException)
            {
            }

            try
            {
                await thrower.throwUndeclaredBAsync(1, 2);
                test(false);
            }
            catch (UnknownUserException)
            {
            }

            try
            {
                await thrower.throwUndeclaredCAsync(1, 2, 3);
                test(false);
            }
            catch (UnknownUserException)
            {
            }
            output.WriteLine("ok");
        }

        output.Write("catching object not exist exception with new AMI mapping... ");
        output.Flush();

        id = Util.stringToIdentity("does not exist");
        try
        {
            var thrower2 = Test.ThrowerPrxHelper.uncheckedCast(thrower.ice_identity(id));
            await thrower2.throwAasAAsync(1);
            test(false);
        }
        catch (ObjectNotExistException ex)
        {
            test(ex.id.Equals(id));
        }
        output.WriteLine("ok");

        output.Write("catching facet not exist exception with new AMI mapping... ");
        output.Flush();

        try
        {
            var thrower2 = Test.ThrowerPrxHelper.uncheckedCast(thrower, "no such facet");
            await thrower2.throwAasAAsync(1);
            test(false);
        }
        catch (FacetNotExistException ex)
        {
            test(ex.facet == "no such facet");
        }

        output.WriteLine("ok");

        output.Write("catching operation not exist exception with new AMI mapping... ");
        output.Flush();

        try
        {
            var thrower4 = Test.WrongOperationPrxHelper.uncheckedCast(thrower);
            await thrower4.noSuchOperationAsync();
            test(false);
        }
        catch (OperationNotExistException ex)
        {
            test(ex.operation == "noSuchOperation");
        }

        output.WriteLine("ok");

        output.Write("catching unknown local exception with new AMI mapping... ");
        output.Flush();

        try
        {
            await thrower.throwLocalExceptionAsync();
            test(false);
        }
        catch (UnknownLocalException)
        {
        }
        catch (OperationNotExistException)
        {
        }

        try
        {
            await thrower.throwLocalExceptionIdempotentAsync();
            test(false);
        }
        catch (UnknownLocalException)
        {
        }
        catch (OperationNotExistException)
        {
        }

        output.WriteLine("ok");

        output.Write("catching unknown non-Ice exception with new AMI mapping... ");
        output.Flush();

        try
        {
            await thrower.throwNonIceExceptionAsync();
            test(false);
        }
        catch (UnknownException)
        {
        }

        output.WriteLine("ok");

        if (thrower.supportsUndeclaredExceptions())
        {
            output.Write("catching unknown user exception with new AMI mapping... ");
            output.Flush();

            try
            {
                await thrower.throwUndeclaredAAsync(1);
                test(false);
            }
            catch (UnknownUserException)
            {
            }

            try
            {
                await thrower.throwUndeclaredBAsync(1, 2);
                test(false);
            }
            catch (UnknownUserException)
            {
            }

            try
            {
                await thrower.throwUndeclaredCAsync(1, 2, 3);
                test(false);
            }
            catch (UnknownUserException)
            {
            }

            output.WriteLine("ok");
        }

        output.Write("catching object not exist exception with new AMI mapping... ");
        output.Flush();

        try
        {
            id = Util.stringToIdentity("does not exist");
            var thrower2 = Test.ThrowerPrxHelper.uncheckedCast(thrower.ice_identity(id));
            await thrower2.throwAasAAsync(1);
        }
        catch (ObjectNotExistException ex)
        {
            test(ex.id.Equals(id));
        }

        output.WriteLine("ok");

        output.Write("catching facet not exist exception with new AMI mapping... ");
        output.Flush();

        try
        {
            var thrower2 = Test.ThrowerPrxHelper.uncheckedCast(thrower, "no such facet");
            await thrower2.throwAasAAsync(1);
            test(false);
        }
        catch (FacetNotExistException ex)
        {
            test(ex.facet == "no such facet");
        }

        output.WriteLine("ok");

        output.Write("catching operation not exist exception with new AMI mapping... ");
        output.Flush();

        try
        {
            var thrower4 = Test.WrongOperationPrxHelper.uncheckedCast(thrower);
            await thrower4.noSuchOperationAsync();
            test(false);
        }
        catch (OperationNotExistException ex)
        {
            test(ex.operation == "noSuchOperation");
        }

        output.WriteLine("ok");

        output.Write("catching unknown local exception with new AMI mapping... ");
        output.Flush();

        try
        {
            await thrower.throwLocalExceptionAsync();
            test(false);
        }
        catch (UnknownLocalException)
        {
        }
        catch (OperationNotExistException)
        {
        }

        try
        {
            await thrower.throwLocalExceptionIdempotentAsync();
            test(false);
        }
        catch (UnknownLocalException)
        {
        }
        catch (OperationNotExistException)
        {
        }

        output.WriteLine("ok");

        output.Write("catching dispatch exception with the AMI mapping... ");
        output.Flush();

        try
        {
            await thrower.throwDispatchExceptionAsync((byte)ReplyStatus.OperationNotExist);
            test(false);
        }
        catch (OperationNotExistException ex)
        {
            test(ex.Message ==
               "Dispatch failed with OperationNotExist { id = 'thrower', facet = '', operation = 'throwDispatchException' }");
        }

        try
        {
            await thrower.throwDispatchExceptionAsync((byte)ReplyStatus.Unauthorized);
            test(false);
        }
        catch (DispatchException ex) when (ex.replyStatus == ReplyStatus.Unauthorized)
        {
            test(ex.Message == "The dispatch failed with reply status Unauthorized." ||
                ex.Message == "The dispatch failed with reply status unauthorized."); // for Swift
        }

        try
        {
            await thrower.throwDispatchExceptionAsync(212);
            test(false);
        }
        catch (DispatchException ex) when (ex.replyStatus == (ReplyStatus)212)
        {
            test(ex.Message == "The dispatch failed with reply status 212.");
        }
        output.WriteLine("ok");

        return thrower;
    }
}
