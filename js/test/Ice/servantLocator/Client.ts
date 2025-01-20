// Copyright (c) ZeroC, Inc.

import { Ice } from "ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests() {
        async function testExceptions(obj: Test.TestIntfPrx) {
            try {
                await obj.requestFailedException();
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.ObjectNotExistException, ex);
                test(ex.id.equals(obj.ice_getIdentity()));
                test(ex.facet == obj.ice_getFacet());
                test(ex.operation == "requestFailedException");
            }

            try {
                await obj.unknownUserException();
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.UnknownUserException, ex);
                test(ex.unknown == "reason");
            }

            try {
                await obj.unknownLocalException();
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.UnknownLocalException, ex);
                test(ex.unknown == "reason");
            }

            try {
                await obj.unknownException();
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.UnknownException, ex);
                test(ex.unknown == "reason");
            }

            try {
                await obj.userException();
                test(false);
            } catch (ex) {
                test(
                    ex instanceof Ice.OperationNotExistException ||
                        (ex instanceof Ice.UnknownUserException &&
                            ex.message.includes("::Test::TestIntfUserException")),
                    ex,
                );
            }

            try {
                await obj.localException();
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.UnknownLocalException, ex);
                test(ex.message.includes("Ice::SocketException") || ex.message.includes("Ice.SocketException"));
            }

            try {
                await obj.jsException();
                test(false);
            } catch (ex) {
                test(
                    ex instanceof Ice.OperationNotExistException ||
                        ex instanceof Ice.UnknownException ||
                        ex.unknown.indexOf("") >= 0,
                    ex,
                );
            }

            try {
                await obj.unknownExceptionWithServantException();
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.UnknownException, ex);
                test(ex.unknown == "reason");
            }

            try {
                await obj.impossibleException(false);
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.UnknownUserException, ex);
            }

            try {
                await obj.impossibleException(true);
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.UnknownUserException, ex);
            }

            try {
                await obj.intfUserException(false);
                test(false);
            } catch (ex) {
                test(ex instanceof Test.TestImpossibleException, ex);
            }

            try {
                await obj.intfUserException(true);
                test(false);
            } catch (ex) {
                test(ex instanceof Test.TestImpossibleException, ex);
            }
        }

        const out = this.getWriter();
        const communicator = this.communicator();

        let obj = new Test.TestIntfPrx(communicator, `asm:${this.getTestEndpoint()}`);

        out.write("testing ice_ids... ");
        try {
            const o = communicator.stringToProxy("category/locate:" + this.getTestEndpoint());
            await o.ice_ids();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.UnknownUserException && ex.unknown == "::Test::TestIntfUserException", ex);
        }

        try {
            const o = communicator.stringToProxy("category/finished:" + this.getTestEndpoint());
            await o.ice_ids();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.UnknownUserException && ex.unknown == "::Test::TestIntfUserException", ex);
        }
        out.writeLine("ok");

        out.write("testing servant locator... ");
        obj = new Test.TestIntfPrx(communicator, `category/locate:${this.getTestEndpoint()}`);
        await obj.ice_ping();

        try {
            obj = new Test.TestIntfPrx(communicator, `category/unknown:${this.getTestEndpoint()}`);
            await obj.ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.ObjectNotExistException, ex);
        }
        out.writeLine("ok");

        out.write("testing default servant locator... ");
        obj = new Test.TestIntfPrx(communicator, `anothercat/locate:${this.getTestEndpoint()}`);
        await obj.ice_ping();
        obj = new Test.TestIntfPrx(communicator, `locate:${this.getTestEndpoint()}`);
        await obj.ice_ping();
        try {
            obj = new Test.TestIntfPrx(communicator, `anothercat/unknown:${this.getTestEndpoint()}`);
            await obj.ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.ObjectNotExistException, ex);
        }

        try {
            obj = new Test.TestIntfPrx(communicator, `unknown:${this.getTestEndpoint()}`);
            await obj.ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.ObjectNotExistException, ex);
        }
        out.writeLine("ok");

        out.write("testing locate exceptions... ");
        obj = new Test.TestIntfPrx(communicator, `category/locate:${this.getTestEndpoint()}`);
        await testExceptions(obj);
        out.writeLine("ok");

        out.write("testing finished exceptions... ");
        obj = new Test.TestIntfPrx(communicator, `category/finished:${this.getTestEndpoint()}`);
        await testExceptions(obj);

        try {
            //
            // Only call these for category/finished.
            //
            await obj.asyncResponse();
            test(false);
        } catch (ex) {
            test(ex instanceof Test.TestImpossibleException, ex); // Called by finished().
        }

        try {
            await obj.asyncException();
            test(false);
        } catch (ex) {
            test(ex instanceof Test.TestImpossibleException, ex); // Called by finished().
        }
        out.writeLine("ok");

        out.write("testing servant locator removal... ");
        const activation = new Test.TestActivationPrx(communicator, `test/activation:${this.getTestEndpoint()}`);
        await activation.activateServantLocator(false);
        try {
            await obj.ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.ObjectNotExistException, ex);
        }
        out.writeLine("ok");

        out.write("testing servant locator addition... ");
        await activation.activateServantLocator(true);
        await obj.ice_ping();
        out.writeLine("ok");

        await obj.shutdown();
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            [communicator] = this.initialize(args);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
