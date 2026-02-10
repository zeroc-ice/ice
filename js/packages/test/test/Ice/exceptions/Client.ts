// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper, test } from "../../Common/TestHelper.js";

export class Client extends TestHelper {
    async allTests() {
        class EmptyI extends Test.Empty {}

        class ServantLocatorI implements Ice.ServantLocator {
            locate(_current: Ice.Current): [Ice.Object | null, object | null] {
                return [null, null];
            }

            finished(_current: Ice.Current, _servant: Ice.Object, _cookie: object) {}

            deactivate(_category: string) {}
        }

        const out = this.getWriter();
        const communicator = this.communicator();
        out.write("testing object adapter registration exceptions... ");
        try {
            await communicator.createObjectAdapter("TestAdapter0");
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.InitializationException, ex as Error); // Expected
        }

        communicator.getProperties().setProperty("TestAdapter0.Endpoints", "default");
        try {
            await communicator.createObjectAdapter("TestAdapter0");
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.PropertyException, ex as Error); // Expected
        }
        out.writeLine("ok");

        out.write("testing servant registration exceptions... ");
        {
            const adapter = await communicator.createObjectAdapter("");
            adapter.add(new EmptyI(), Ice.stringToIdentity("x"));
            try {
                adapter.add(new EmptyI(), Ice.stringToIdentity("x"));
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.AlreadyRegisteredException, ex as Error);
            }

            try {
                adapter.add(new EmptyI(), Ice.stringToIdentity(""));
                test(false);
            } catch (ex) {
                test(ex instanceof TypeError, ex as Error);
            }

            try {
                adapter.add(null!, Ice.stringToIdentity("x"));
                test(false);
            } catch (ex) {
                test(ex instanceof TypeError, ex as Error);
            }

            adapter.remove(Ice.stringToIdentity("x"));
            try {
                adapter.remove(Ice.stringToIdentity("x"));
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.NotRegisteredException, ex as Error);
            }
            adapter.destroy();
        }
        out.writeLine("ok");

        out.write("testing servant locator registration exceptions... ");
        {
            const adapter = await communicator.createObjectAdapter("");
            adapter.addServantLocator(new ServantLocatorI(), "x");
            try {
                adapter.addServantLocator(new ServantLocatorI(), "x");
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.AlreadyRegisteredException, ex as Error);
            }
            adapter.destroy();
            out.writeLine("ok");
        }
        out.writeLine("ok");

        const thrower = new Test.ThrowerPrx(communicator, `thrower:${this.getTestEndpoint()}`);

        out.write("catching exact types... ");
        try {
            await thrower.throwAasA(1);
            test(false);
        } catch (ex) {
            if (ex instanceof Test.A) {
                test(ex.aMem === 1);
            } else {
                test(false, ex as Error);
            }
        }

        try {
            await thrower.throwAorDasAorD(1);
            test(false);
        } catch (ex) {
            if (ex instanceof Test.A) {
                test(ex.aMem === 1);
            } else {
                test(false, ex as Error);
            }
        }

        try {
            await thrower.throwAorDasAorD(-1);
            test(false);
        } catch (ex) {
            if (ex instanceof Test.D) {
                test(ex.dMem === -1);
            } else {
                test(false, ex as Error);
            }
        }

        try {
            await thrower.throwBasB(1, 2);
            test(false);
        } catch (ex) {
            if (ex instanceof Test.B) {
                test(ex.aMem == 1);
                test(ex.bMem == 2);
            } else {
                test(false, ex as Error);
            }
        }

        try {
            await thrower.throwCasC(1, 2, 3);
            test(false);
        } catch (ex) {
            if (ex instanceof Test.C) {
                test(ex.aMem == 1);
                test(ex.bMem == 2);
                test(ex.cMem == 3);
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("catching base types... ");
        try {
            await thrower.throwBasB(1, 2);
            test(false);
        } catch (ex) {
            if (ex instanceof Test.A) {
                test(ex.aMem == 1);
            } else {
                test(false, ex as Error);
            }
        }

        try {
            await thrower.throwCasC(1, 2, 3);
            test(false);
        } catch (ex) {
            if (ex instanceof Test.B) {
                test(ex.aMem == 1);
                test(ex.bMem == 2);
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("catching derived types... ");
        try {
            await thrower.throwBasA(1, 2);
            test(false);
        } catch (ex) {
            if (ex instanceof Test.B) {
                test(ex instanceof Test.B, ex);
                test(ex.aMem == 1);
                test(ex.bMem == 2);
            } else {
                test(false, ex as Error);
            }
        }

        try {
            await thrower.throwCasA(1, 2, 3);
            test(false);
        } catch (ex) {
            if (ex instanceof Test.C) {
                test(ex instanceof Test.C, ex);
                test(ex.aMem == 1);
                test(ex.bMem == 2);
                test(ex.cMem == 3);
            } else {
                test(false, ex as Error);
            }
        }

        try {
            await thrower.throwCasB(1, 2, 3);
            test(false);
        } catch (ex) {
            if (ex instanceof Test.C) {
                test(ex instanceof Test.C, ex);
                test(ex.aMem == 1);
                test(ex.bMem == 2);
                test(ex.cMem == 3);
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        if (await thrower.supportsUndeclaredExceptions()) {
            out.write("catching unknown user exception... ");
            try {
                await thrower.throwUndeclaredA(1);
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.UnknownUserException, ex as Error);
            }

            try {
                await thrower.throwUndeclaredB(1, 2);
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.UnknownUserException, ex as Error);
            }

            try {
                await thrower.throwUndeclaredC(1, 2, 3);
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.UnknownUserException, ex as Error);
            }
            out.writeLine("ok");
        }

        if (await thrower.supportsAssertException()) {
            out.write("testing assert in the server... ");
            try {
                await thrower.throwAssertException();
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.ConnectionLostException || ex instanceof Ice.UnknownException, ex as Error);
            }
            out.writeLine("ok");
        }

        out.write("testing memory limit marshal exception...");
        try {
            await thrower.throwMemoryLimitException(new Uint8Array(0));
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.MarshalException, ex as Error);
        }

        try {
            await thrower.throwMemoryLimitException(new Uint8Array(20 * 1024));
            test(false);
        } catch (ex) {
            const err = ex as Error;
            test(err.toString().indexOf("ConnectionLostException") > 0, err);
        }
        out.writeLine("ok");

        let retries = 30;
        while (--retries > 0) {
            // The above test can cause a close connection between the echo server and
            // bidir server, we need to wait until the bidir server has reopen the
            // connection with the echo server.

            try {
                await thrower.ice_ping();
                break;
            } catch (ex) {
                if (ex instanceof Ice.ObjectNotExistException && retries > 0) {
                    await Ice.Promise.delay(20);
                } else {
                    throw ex;
                }
            }
        }

        out.write("catching object not exist exception... ");
        try {
            const thrower2 = Test.ThrowerPrx.uncheckedCast(
                thrower.ice_identity(Ice.stringToIdentity("does not exist")),
            );
            await thrower2.ice_ping();
            test(false);
        } catch (ex) {
            if (ex instanceof Ice.ObjectNotExistException) {
                test(ex.id.equals(Ice.stringToIdentity("does not exist")));
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("catching facet not exist exception... ");
        try {
            const thrower2 = Test.ThrowerPrx.uncheckedCast(thrower, "no such facet");
            await thrower2.ice_ping();
            test(false);
        } catch (ex) {
            if (ex instanceof Ice.FacetNotExistException) {
                test(ex.facet == "no such facet");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("catching operation not exist exception... ");
        try {
            const thrower2 = Test.WrongOperationPrx.uncheckedCast(thrower);
            await thrower2.noSuchOperation();
            test(false);
        } catch (ex) {
            if (ex instanceof Ice.OperationNotExistException) {
                test(ex.operation == "noSuchOperation");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("catching unknown local exception... ");
        try {
            await thrower.throwLocalException();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.UnknownLocalException, ex as Error);
        }

        try {
            await thrower.throwLocalExceptionIdempotent();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.UnknownLocalException || ex instanceof Ice.OperationNotExistException, ex as Error);
        }
        out.writeLine("ok");

        out.write("catching unknown non-Ice exception... ");
        try {
            await thrower.throwNonIceException();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.UnknownException, ex as Error);
        }
        out.writeLine("ok");

        out.write("catching dispatch exception... ");

        try {
            await thrower.throwDispatchException(Ice.ReplyStatus.OperationNotExist.value);
            test(false);
        } catch (ex) {
            if (ex instanceof Ice.OperationNotExistException) {
                test(
                    ex.message ===
                        "Dispatch failed with OperationNotExist { id = 'thrower', facet = '', operation = 'throwDispatchException' }",
                );
            } else {
                test(false, ex as Error);
            }
        }

        try {
            await thrower.throwDispatchException(Ice.ReplyStatus.Unauthorized.value);
            test(false);
        } catch (ex) {
            if (ex instanceof Ice.DispatchException) {
                test(ex.replyStatus == Ice.ReplyStatus.Unauthorized);
                test(
                    ex.message === "The dispatch failed with reply status Unauthorized." ||
                        ex.message === "The dispatch failed with reply status unauthorized.",
                );
            } else {
                test(false, ex as Error);
            }
        }

        try {
            await thrower.throwDispatchException(212);
            test(false);
        } catch (ex) {
            if (ex instanceof Ice.DispatchException) {
                test(ex.replyStatus.value === 212);
                test(ex.message === "The dispatch failed with reply status 212.");
            } else {
                test(false, ex as Error);
            }
        }

        out.writeLine("ok");

        out.write("testing asynchronous exceptions... ");
        await thrower.throwAfterResponse();
        try {
            await thrower.throwAfterException();
            test(false);
        } catch (ex) {
            test(ex instanceof Test.A, ex as Error);
        }
        out.writeLine("ok");

        await thrower.shutdown();
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.MessageSizeMax", "10");
            properties.setProperty("Ice.Warn.Connections", "0");
            [communicator] = this.initialize(properties);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
