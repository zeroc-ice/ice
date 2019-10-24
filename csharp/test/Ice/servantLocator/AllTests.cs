//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace Ice
{
    namespace servantLocator
    {
        public class AllTests : global::Test.AllTests
        {
            public static void testExceptions(Test.TestIntfPrx obj)
            {
                try
                {
                    obj.requestFailedException();
                    test(false);
                }
                catch (Ice.ObjectNotExistException ex)
                {
                    test(ex.id.Equals(obj.ice_getIdentity()));
                    test(ex.facet.Equals(obj.ice_getFacet()));
                    test(ex.operation.Equals("requestFailedException"));
                }
                catch (Exception)
                {
                    test(false);
                }

                try
                {
                    obj.unknownUserException();
                    test(false);
                }
                catch (Ice.UnknownUserException ex)
                {
                    test(ex.unknown.Equals("reason"));
                }
                catch (Exception)
                {
                    test(false);
                }

                try
                {
                    obj.unknownLocalException();
                    test(false);
                }
                catch (Ice.UnknownLocalException ex)
                {
                    test(ex.unknown.Equals("reason"));
                }
                catch (Exception)
                {
                    test(false);
                }

                try
                {
                    obj.unknownException();
                    test(false);
                }
                catch (Ice.UnknownException ex)
                {
                    test(ex.unknown.Equals("reason"));
                }
                catch (Exception)
                {
                    test(false);
                }

                try
                {
                    obj.userException();
                    test(false);
                }
                catch (Ice.UnknownUserException ex)
                {
                    //Console.Error.WriteLine(ex.unknown);
                    test(ex.unknown.IndexOf("Test::TestIntfUserException") >= 0);
                }
                catch (Ice.OperationNotExistException)
                {
                }
                catch (Exception)
                {
                    test(false);
                }

                try
                {
                    obj.localException();
                    test(false);
                }
                catch (Ice.UnknownLocalException ex)
                {
                    test(ex.unknown.IndexOf("Ice::SocketException") >= 0 ||
                         ex.unknown.IndexOf("Ice.SocketException") >= 0);
                }
                catch (Exception)
                {
                    test(false);
                }

                try
                {
                    obj.csException();
                    test(false);
                }
                catch (Ice.UnknownException ex)
                {
                    test(ex.unknown.IndexOf("System.Exception") >= 0);
                }
                catch (Ice.OperationNotExistException)
                {
                }
                catch (Exception)
                {
                    test(false);
                }

                try
                {
                    obj.unknownExceptionWithServantException();
                    test(false);
                }
                catch (Ice.UnknownException ex)
                {
                    test(ex.unknown.Equals("reason"));
                }
                catch (Exception)
                {
                    test(false);
                }

                try
                {
                    obj.impossibleException(false);
                    test(false);
                }
                catch (Ice.UnknownUserException)
                {
                    // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
                }
                catch (Exception)
                {
                    test(false);
                }

                try
                {
                    obj.impossibleException(true);
                    test(false);
                }
                catch (Ice.UnknownUserException)
                {
                    // Operation throws TestImpossibleException, but locate() and finished() throw TestIntfUserException.
                }
                catch (Exception)
                {
                    test(false);
                }

                try
                {
                    obj.intfUserException(false);
                    test(false);
                }
                catch (Test.TestImpossibleException)
                {
                    // Operation doesn't throw, but locate() and finished() throw TestImpossibleException.
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex);
                    test(false);
                }

                try
                {
                    obj.intfUserException(true);
                    test(false);
                }
                catch (Test.TestImpossibleException)
                {
                    // Operation throws TestIntfUserException, but locate() and finished() throw TestImpossibleException.
                }
                catch (Exception)
                {
                    test(false);
                }
            }

            public static Test.TestIntfPrx allTests(global::Test.TestHelper helper)
            {
                var communicator = helper.communicator();
                var output = helper.getWriter();

                output.Write("testing stringToProxy... ");
                output.Flush();
                string @ref = "asm:" + helper.getTestEndpoint(0);
                var @base = communicator.stringToProxy(@ref);
                test(@base != null);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                var obj = Test.TestIntfPrxHelper.checkedCast(@base);
                test(obj != null);
                test(obj.Equals(@base));
                output.WriteLine("ok");

                output.Write("testing ice_ids... ");
                output.Flush();
                try
                {
                    var o = communicator.stringToProxy("category/locate:" + helper.getTestEndpoint(0));
                    o.ice_ids();
                    test(false);
                }
                catch (Ice.UnknownUserException ex)
                {
                    test(ex.unknown.Equals("::Test::TestIntfUserException"));
                }
                catch (Exception)
                {
                    test(false);
                }

                try
                {
                    var o = communicator.stringToProxy("category/finished:" + helper.getTestEndpoint(0));
                    o.ice_ids();
                    test(false);
                }
                catch (Ice.UnknownUserException ex)
                {
                    test(ex.unknown.Equals("::Test::TestIntfUserException"));
                }
                catch (Exception)
                {
                    test(false);
                }
                output.WriteLine("ok");

                output.Write("testing servant locator...");
                output.Flush();
                @base = communicator.stringToProxy("category/locate:" + helper.getTestEndpoint(0));
                obj = Test.TestIntfPrxHelper.checkedCast(@base);
                try
                {
                    Test.TestIntfPrxHelper.checkedCast(communicator.stringToProxy("category/unknown:" + helper.getTestEndpoint(0)));
                }
                catch (Ice.ObjectNotExistException)
                {
                }
                output.WriteLine("ok");

                output.Write("testing default servant locator...");
                output.Flush();
                @base = communicator.stringToProxy("anothercat/locate:" + helper.getTestEndpoint(0));
                obj = Test.TestIntfPrxHelper.checkedCast(@base);
                @base = communicator.stringToProxy("locate:" + helper.getTestEndpoint(0));
                obj = Test.TestIntfPrxHelper.checkedCast(@base);
                try
                {
                    Test.TestIntfPrxHelper.checkedCast(communicator.stringToProxy("anothercat/unknown:" + helper.getTestEndpoint(0)));
                }
                catch (Ice.ObjectNotExistException)
                {
                }
                try
                {
                    Test.TestIntfPrxHelper.checkedCast(communicator.stringToProxy("unknown:" + helper.getTestEndpoint(0)));
                }
                catch (Ice.ObjectNotExistException)
                {
                }
                output.WriteLine("ok");

                output.Write("testing locate exceptions... ");
                output.Flush();
                @base = communicator.stringToProxy("category/locate:" + helper.getTestEndpoint(0));
                obj = Test.TestIntfPrxHelper.checkedCast(@base);
                testExceptions(obj);
                output.WriteLine("ok");

                output.Write("testing finished exceptions... ");
                output.Flush();
                @base = communicator.stringToProxy("category/finished:" + helper.getTestEndpoint(0));
                obj = Test.TestIntfPrxHelper.checkedCast(@base);
                testExceptions(obj);

                //
                // Only call these for category/finished.
                //
                try
                {
                    obj.asyncResponse();
                }
                catch (Test.TestIntfUserException)
                {
                    test(false);
                }
                catch (Test.TestImpossibleException)
                {
                    //
                    // Called by finished().
                    //
                }

                //
                // Only call these for category/finished.
                //
                try
                {
                    obj.asyncException();
                }
                catch (Test.TestIntfUserException)
                {
                    test(false);
                }
                catch (Test.TestImpossibleException)
                {
                    //
                    // Called by finished().
                    //
                }

                output.WriteLine("ok");

                output.Write("testing servant locator removal... ");
                output.Flush();
                @base = communicator.stringToProxy("test/activation:" + helper.getTestEndpoint(0));
                var activation = Test.TestActivationPrxHelper.checkedCast(@base);
                activation.activateServantLocator(false);
                try
                {
                    obj.ice_ping();
                    test(false);
                }
                catch (Ice.ObjectNotExistException)
                {
                    output.WriteLine("ok");
                }
                output.Write("testing servant locator addition... ");
                output.Flush();
                activation.activateServantLocator(true);
                try
                {
                    obj.ice_ping();
                    output.WriteLine("ok");
                }
                catch (Exception)
                {
                    test(false);
                }
                return obj;
            }
        }
    }
}
