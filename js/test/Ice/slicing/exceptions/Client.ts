// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../../Common/TestHelper.js";

const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests() {
        const out = this.getWriter();
        const communicator = this.communicator();

        const prx = new Test.TestIntfPrx(communicator, `Test:${this.getTestEndpoint()} -t 10000`);

        out.write("base... ");
        try {
            await prx.baseAsBase();
            test(false);
        } catch (ex) {
            if (ex instanceof Test.Base) {
                test(ex.b == "Base.b");
                test(ex.ice_id() == "::Test::Base");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("slicing of unknown derived... ");
        try {
            await prx.unknownDerivedAsBase();
            test(false);
        } catch (ex) {
            if (ex instanceof Test.Base) {
                test(ex.b == "UnknownDerived.b");
                test(ex.ice_id() == "::Test::Base");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("non-slicing of known derived as base... ");
        try {
            await prx.knownDerivedAsBase();
            test(false);
        } catch (ex) {
            if (ex instanceof Test.KnownDerived) {
                test(ex.b == "KnownDerived.b");
                test(ex.kd == "KnownDerived.kd");
                test(ex.ice_id() == "::Test::KnownDerived");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("non-slicing of known derived as derived... ");
        try {
            await prx.knownDerivedAsKnownDerived();
            test(false);
        } catch (ex) {
            if (ex instanceof Test.KnownDerived) {
                test(ex.b == "KnownDerived.b");
                test(ex.kd == "KnownDerived.kd");
                test(ex.ice_id() == "::Test::KnownDerived");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("slicing of unknown intermediate as base... ");
        try {
            await prx.unknownIntermediateAsBase();
            test(false);
        } catch (ex) {
            if (ex instanceof Test.Base) {
                test(ex.b == "UnknownIntermediate.b");
                test(ex.ice_id() == "::Test::Base");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("slicing of known intermediate as base... ");
        try {
            await prx.knownIntermediateAsBase();
            test(false);
        } catch (ex) {
            if (ex instanceof Test.KnownIntermediate) {
                test(ex.b == "KnownIntermediate.b");
                test(ex.ki == "KnownIntermediate.ki");
                test(ex.ice_id() == "::Test::KnownIntermediate");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("slicing of known most derived as base... ");
        try {
            await prx.knownMostDerivedAsBase();
            test(false);
        } catch (ex) {
            if (ex instanceof Test.KnownMostDerived) {
                test(ex.b == "KnownMostDerived.b");
                test(ex.ki == "KnownMostDerived.ki");
                test(ex.kmd == "KnownMostDerived.kmd");
                test(ex.ice_id() == "::Test::KnownMostDerived");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("non-slicing of known intermediate as intermediate... ");
        try {
            await prx.knownIntermediateAsKnownIntermediate();
            test(false);
        } catch (ex) {
            if (ex instanceof Test.KnownIntermediate) {
                test(ex.b == "KnownIntermediate.b");
                test(ex.ki == "KnownIntermediate.ki");
                test(ex.ice_id() == "::Test::KnownIntermediate");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("non-slicing of known most derived as intermediate... ");
        try {
            await prx.knownMostDerivedAsKnownIntermediate();
            test(false);
        } catch (ex) {
            if (ex instanceof Test.KnownMostDerived) {
                test(ex.b == "KnownMostDerived.b");
                test(ex.ki == "KnownMostDerived.ki");
                test(ex.kmd == "KnownMostDerived.kmd");
                test(ex.ice_id() == "::Test::KnownMostDerived");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("non-slicing of known most derived as most derived... ");
        try {
            await prx.knownMostDerivedAsKnownMostDerived();
            test(false);
        } catch (ex) {
            if (ex instanceof Test.KnownMostDerived) {
                test(ex.b == "KnownMostDerived.b");
                test(ex.ki == "KnownMostDerived.ki");
                test(ex.kmd == "KnownMostDerived.kmd");
                test(ex.ice_id() == "::Test::KnownMostDerived");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        await prx.shutdown();
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
