//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const TestHelper = require("TestHelper").TestHelper;
    const test = TestHelper.test;

    class Client extends TestHelper
    {
        async allTests()
        {
            async function testExceptions(obj)
            {
                try
                {
                    await obj.requestFailedException();
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.ObjectNotExistException, ex);
                    test(ex.id.equals(obj.ice_getIdentity()));
                    test(ex.facet == obj.ice_getFacet());
                    test(ex.operation == "requestFailedException");
                }

                try
                {
                    await obj.unknownUserException();
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.UnknownUserException, ex);
                    test(ex.unknown == "reason");
                }

                try
                {
                    await obj.unknownLocalException();
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.UnknownLocalException, ex);
                    test(ex.unknown == "reason");
                }

                try
                {
                    await obj.unknownException();
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.UnknownException, ex);
                    test(ex.unknown == "reason");
                }

                try
                {
                    await obj.userException();
                    test(false);
                }
                catch(ex)
                {
                    test((ex instanceof Ice.OperationNotExistException) ||
                         (ex instanceof Ice.UnknownUserException &&
                          ex.unknown == "::Test::TestIntfUserException"), ex);
                }

                try
                {
                    await obj.localException();
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.UnknownLocalException, ex);
                    test(ex.unknown.indexOf("Ice::SocketException") >= 0 ||
                         ex.unknown.indexOf("Ice.SocketException") >= 0);
                }

                try
                {
                    await obj.jsException();
                    test(false);
                }
                catch(ex)
                {
                    test((ex instanceof Ice.OperationNotExistException) ||
                         (ex instanceof Ice.UnknownException || ex.unknown.indexOf("") >= 0), ex);
                }

                try
                {
                    await obj.unknownExceptionWithServantException();
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.UnknownException, ex);
                    test(ex.unknown == "reason");
                }

                try
                {
                    await obj.impossibleException(false);
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.UnknownUserException, ex);
                }

                try
                {
                    await obj.impossibleException(true);
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.UnknownUserException, ex);
                }

                try
                {
                    await obj.intfUserException(false);
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Test.TestImpossibleException, ex);
                }

                try
                {
                    await obj.intfUserException(true);
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Test.TestImpossibleException, ex);
                }
            }

            const out = this.getWriter();
            const communicator = this.communicator();
            out.write("testing stringToProxy... ");
            const ref = "asm:" + this.getTestEndpoint();
            let base = communicator.stringToProxy(ref);
            test(base !== null);
            out.writeLine("ok");

            out.write("testing checked cast... ");
            let obj = await Test.TestIntfPrx.checkedCast(base);
            test(obj !== null);
            test(obj.equals(base));
            out.writeLine("ok");

            out.write("testing ice_ids... ");
            try
            {
                const o = communicator.stringToProxy("category/locate:" + this.getTestEndpoint());
                await o.ice_ids();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.UnknownUserException && ex.unknown == "::Test::TestIntfUserException", ex);
            }

            try
            {
                const o = communicator.stringToProxy("category/finished:" + this.getTestEndpoint());
                await o.ice_ids();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.UnknownUserException && ex.unknown == "::Test::TestIntfUserException", ex);
            }
            out.writeLine("ok");

            out.write("testing servant locator... ");
            base = communicator.stringToProxy("category/locate:" + this.getTestEndpoint());
            obj = await Test.TestIntfPrx.checkedCast(base);

            try
            {
                await Test.TestIntfPrx.checkedCast(
                    communicator.stringToProxy("category/unknown:" + this.getTestEndpoint()));
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ObjectNotExistException, ex);
            }
            out.writeLine("ok");

            out.write("testing default servant locator... ");
            base = communicator.stringToProxy("anothercat/locate:" + this.getTestEndpoint());
            obj = await Test.TestIntfPrx.checkedCast(base);
            base = communicator.stringToProxy("locate:" + this.getTestEndpoint());
            obj = await Test.TestIntfPrx.checkedCast(base);
            try
            {
                await Test.TestIntfPrx.checkedCast(
                    communicator.stringToProxy("anothercat/unknown:" + this.getTestEndpoint()));
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ObjectNotExistException, ex);
            }

            try
            {
                await Test.TestIntfPrx.checkedCast(communicator.stringToProxy("unknown:" + this.getTestEndpoint()));
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ObjectNotExistException, ex);
            }
            out.writeLine("ok");

            out.write("testing locate exceptions... ");
            base = communicator.stringToProxy("category/locate:" + this.getTestEndpoint());
            obj = await Test.TestIntfPrx.checkedCast(base);
            await testExceptions(obj);
            out.writeLine("ok");

            out.write("testing finished exceptions... ");
            base = communicator.stringToProxy("category/finished:" + this.getTestEndpoint());
            obj = await Test.TestIntfPrx.checkedCast(base);
            await testExceptions(obj);

            try
            {
                //
                // Only call these for category/finished.
                //
                await obj.asyncResponse();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Test.TestImpossibleException, ex); // Called by finished().
            }

            try
            {
                await obj.asyncException();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Test.TestImpossibleException, ex); // Called by finished().
            }
            out.writeLine("ok");

            out.write("testing servant locator removal... ");
            base = communicator.stringToProxy("test/activation:" + this.getTestEndpoint());
            const activation = await Test.TestActivationPrx.checkedCast(base);
            await activation.activateServantLocator(false);
            try
            {
                await obj.ice_ping();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ObjectNotExistException, ex);
            }
            out.writeLine("ok");

            out.write("testing servant locator addition... ");
            await activation.activateServantLocator(true);
            await obj.ice_ping();
            out.writeLine("ok");

            await obj.shutdown();
        }

        async run(args)
        {
            let communicator;
            try
            {
                [communicator] = this.initialize(args);
                await this.allTests();
            }
            finally
            {
                if(communicator)
                {
                    await communicator.destroy();
                }
            }
        }
    }
    exports.Client = Client;

}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
