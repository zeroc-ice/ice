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
                    test(ex.id.Equals(obj.Identity));
                    test(ex.facet.Equals(obj.Facet));
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
                var @base = IObjectPrx.Parse(@ref, communicator);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                var obj = Test.TestIntfPrx.CheckedCast(@base);
                test(obj.Equals(@base));
                output.WriteLine("ok");

                output.Write("testing ice_ids... ");
                output.Flush();
                try
                {
                    var o = IObjectPrx.Parse($"category/locate:{helper.getTestEndpoint(0)}", communicator);
                    o.IceIds();
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
                    var o = IObjectPrx.Parse($"category/finished:{helper.getTestEndpoint(0)}", communicator);
                    o.IceIds();
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
                @base = IObjectPrx.Parse($"category/locate:{helper.getTestEndpoint(0)}", communicator);
                obj = Test.TestIntfPrx.CheckedCast(@base);
                try
                {
                    Test.TestIntfPrx.Parse($"category/unknown:{helper.getTestEndpoint(0)}", communicator).IcePing();
                }
                catch (Ice.ObjectNotExistException)
                {
                }
                output.WriteLine("ok");

                output.Write("testing default servant locator...");
                output.Flush();
                @base = IObjectPrx.Parse($"anothercat/locate:{helper.getTestEndpoint(0)}", communicator);
                obj = Test.TestIntfPrx.CheckedCast(@base);
                @base = IObjectPrx.Parse($"locate:{helper.getTestEndpoint(0)}", communicator);
                obj = Test.TestIntfPrx.CheckedCast(@base);
                try
                {
                    Test.TestIntfPrx.Parse($"anothercat/unknown:{helper.getTestEndpoint(0)}", communicator).IcePing();
                }
                catch (Ice.ObjectNotExistException)
                {
                }
                try
                {
                    Test.TestIntfPrx.Parse($"unknown:{helper.getTestEndpoint(0)}", communicator).IcePing();
                }
                catch (Ice.ObjectNotExistException)
                {
                }
                output.WriteLine("ok");

                output.Write("testing locate exceptions... ");
                output.Flush();
                @base = IObjectPrx.Parse($"category/locate:{helper.getTestEndpoint(0)}", communicator);
                obj = Test.TestIntfPrx.CheckedCast(@base);
                testExceptions(obj);
                output.WriteLine("ok");

                output.Write("testing finished exceptions... ");
                output.Flush();
                @base = IObjectPrx.Parse($"category/finished:{helper.getTestEndpoint(0)}", communicator);
                obj = Test.TestIntfPrx.CheckedCast(@base);
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
                @base = IObjectPrx.Parse($"test/activation:{helper.getTestEndpoint(0)}", communicator);
                var activation = Test.TestActivationPrx.CheckedCast(@base);
                activation.activateServantLocator(false);
                try
                {
                    obj.IcePing();
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
                    obj.IcePing();
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
