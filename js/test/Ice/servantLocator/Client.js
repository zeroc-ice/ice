// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;

    function allTests(communicator, out)
    {
        var p = new Ice.Promise();

        function test(b)
        {
            if(!b)
            {
                try
                {
                    throw new Error("test failed");
                }
                catch(err)
                {
                    p.reject(err);
                    throw err;
                }
            }
        };

        function testExceptions(obj)
        {
            return obj.requestFailedException().then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.ObjectNotExistException);
                    test(ex.id.equals(obj.ice_getIdentity()));
                    test(ex.facet == obj.ice_getFacet());
                    test(ex.operation == "requestFailedException");
                }
            ).then(() => obj.unknownUserException()).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.UnknownUserException);
                    test(ex.unknown == "reason");
                }
            ).then(() => obj.unknownLocalException()).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.UnknownLocalException);
                    test(ex.unknown == "reason");
                }
            ).then(() => obj.unknownException()).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.UnknownException);
                    test(ex.unknown == "reason");
                }
            ).then(() => obj.userException()).then(() => test(false),
                (ex) =>
                {
                    if(!(ex instanceof Ice.OperationNotExistException))
                    {
                        test(ex instanceof Ice.UnknownUserException);
                        test(ex.unknown == "::Test::TestIntfUserException");
                    }
                }
            ).then(() => obj.localException()).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.UnknownLocalException);
                    test(ex.unknown.indexOf("Ice::SocketException") >= 0 ||
                         ex.unknown.indexOf("Ice.SocketException") >= 0);
                }
            ).then(() => obj.jsException()).then(() => test(false),
                (ex) =>
                {
                    if(!(ex instanceof Ice.OperationNotExistException))
                    {
                        test(ex instanceof Ice.UnknownException);
                        test(ex.unknown.indexOf("") >= 0);
                    }
                }
            ).then(() => obj.unknownExceptionWithServantException()).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.UnknownException);
                    test(ex.unknown == "reason");
                }
            ).then(() => obj.impossibleException(false)).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.UnknownUserException);
                }
            ).then(() => obj.impossibleException(true)).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.UnknownUserException);
                }
            ).then(() => obj.intfUserException(false)).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Test.TestImpossibleException);
                }
            ).then(() => obj.intfUserException(true)).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Test.TestImpossibleException);
                }
            );
        }

        var ref, base, obj, activation;
        return Ice.Promise.try(() =>
            {
                out.write("testing stringToProxy... ");
                ref = "asm:default -p 12010";
                base = communicator.stringToProxy(ref);
                test(base !== null);
                out.writeLine("ok");

                out.write("testing checked cast... ");
                return Test.TestIntfPrx.checkedCast(base);
            }).then((obj) =>
                {
                    test(obj != null);
                    test(obj.equals(base));
                    out.writeLine("ok");

                    out.write("testing ice_ids... ");
                    let o = communicator.stringToProxy("category/locate:default -p 12010");
                    return o.ice_ids();
                }
            ).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.UnknownUserException && ex.unknown == "::Test::TestIntfUserException")
                    let o = communicator.stringToProxy("category/finished:default -p 12010");
                    return o.ice_ids();
                }
            ).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.UnknownUserException && ex.unknown == "::Test::TestIntfUserException");
                    out.writeLine("ok");
                    out.write("testing servant locator... ");
                    base = communicator.stringToProxy("category/locate:default -p 12010");
                    return Test.TestIntfPrx.checkedCast(base);
                }
            ).then((obj) =>
                {
                    return Test.TestIntfPrx.checkedCast(communicator.stringToProxy("category/unknown:default -p 12010"));
                }
            ).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.ObjectNotExistException);
                    out.writeLine("ok");

                    out.write("testing default servant locator... ");
                    base = communicator.stringToProxy("anothercat/locate:default -p 12010");
                    return Test.TestIntfPrx.checkedCast(base);
                }
            ).then((obj) =>
                {
                    base = communicator.stringToProxy("locate:default -p 12010");
                    return Test.TestIntfPrx.checkedCast(base);
                }
            ).then((obj) =>
                {
                    return Test.TestIntfPrx.checkedCast(communicator.stringToProxy("anothercat/unknown:default -p 12010"));
                }
            ).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.ObjectNotExistException);
                    return Test.TestIntfPrx.checkedCast(communicator.stringToProxy("unknown:default -p 12010"));
                }
            ).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.ObjectNotExistException);
                    out.writeLine("ok");

                    out.write("testing locate exceptions... ");
                    base = communicator.stringToProxy("category/locate:default -p 12010");
                    return Test.TestIntfPrx.checkedCast(base);
                }
            ).then((obj) =>
                {
                    return testExceptions(obj);
                }
            ).then(() =>
                {
                    out.writeLine("ok");

                    out.write("testing finished exceptions... ");
                    base = communicator.stringToProxy("category/finished:default -p 12010");
                    return Test.TestIntfPrx.checkedCast(base);
                }
            ).then((o) =>
                {
                    obj = o;
                    return testExceptions(obj);
                }
            ).then(() =>
                {
                    //
                    // Only call these for category/finished.
                    //
                    return obj.asyncResponse();
                }
            ).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Test.TestImpossibleException); // Called by finished().
                    return obj.asyncException();
                }
            ).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Test.TestImpossibleException); // Called by finished().
                    out.writeLine("ok");

                    out.write("testing servant locator removal... ");
                    base = communicator.stringToProxy("test/activation:default -p 12010");
                    return Test.TestActivationPrx.checkedCast(base);
                }
            ).then((a) =>
                {
                    activation = a;
                    return activation.activateServantLocator(false);
                }
            ).then(() =>
                {
                    return obj.ice_ping();
                }
            ).then(() => test(false),
                (ex) =>
                {
                    test(ex instanceof Ice.ObjectNotExistException);
                    out.writeLine("ok");

                    out.write("testing servant locator addition... ");
                    return activation.activateServantLocator(true);
                }
            ).then(() =>
                {
                    return obj.ice_ping();
                }
            ).then(() =>
                {
                    out.writeLine("ok");
                    return obj.shutdown();
                }
            );
    }

    var run = function(out, id)
    {
        var communicator = Ice.initialize(id);
        return Ice.Promise.try(() => allTests(communicator, out)).finally(() => communicator.destroy());
    };
    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
