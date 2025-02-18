// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { TestHelper } from "../../Common/TestHelper.js";
import * as fs from "fs";
import * as path from "path";

const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests(args: string[]) {
        const out = this.getWriter();
        out.write("testing configuration file escapes... ");
        const props = new Map();
        props.set("Foo\tBar", "3");
        props.set("Foo\\tBar", "4");
        props.set("Escape\\ Space", "2");
        props.set("Prop1", "1");
        props.set("Prop2", "2");
        props.set("Prop3", "3");
        props.set("My Prop1", "1");
        props.set("My Prop2", "2");
        props.set("My.Prop1", "a property");
        props.set("My.Prop2", "a     property");
        props.set("My.Prop3", "  a     property  ");
        props.set("My.Prop4", "  a     property  ");
        props.set("My.Prop5", "a \\ property");
        props.set("foo=bar", "1");
        props.set("foo#bar", "2");
        props.set("foo bar", "3");
        props.set("A", "1");
        props.set("B", "2 3 4");
        props.set("C", "5=#6");
        props.set("AServer", "\\\\server\\dir");
        props.set("BServer", "\\server\\dir");

        const properties = Ice.createProperties();
        /* eslint-disable no-sync */
        if (!TestHelper.isBrowser()) {
            //
            // We are running with NodeJS we load the properties file from the file system.
            //
            properties.parse(
                fs.readFileSync(path.join(args[4], "config", "escapes.cfg"), {
                    encoding: "utf8",
                }),
            );
            for (const [key, value] of props) {
                test(properties.getProperty(key) == value);
            }
        }
        out.writeLine("ok");

        {
            out.write("testing property regexp pattern...");
            const properties = Ice.createProperties();
            properties.setProperty("Ice.Default.Locator.Context.Foo", "Bar");
            const value = properties.getIceProperty("Ice.Default.Locator.Context.Foo");
            test(value == "Bar");
            out.writeLine("ok");
        }

        {
            out.write("testing ice properties with set default values...");
            const properties = Ice.createProperties();

            const toStringMode = properties.getIceProperty("Ice.ToStringMode");
            test(toStringMode == "Unicode");

            const closeTimeout = properties.getIcePropertyAsInt("Ice.Connection.Client.CloseTimeout");
            test(closeTimeout == 10);

            const retryIntervals = properties.getIcePropertyAsList("Ice.RetryIntervals");
            test(retryIntervals.length == 1);
            test(retryIntervals[0] == "0");

            out.writeLine("ok");
        }

        {
            out.write("testing ice properties with unset default values...");
            const properties = Ice.createProperties();

            const stringValue = properties.getIceProperty("Ice.Default.Host");
            test(stringValue == "");

            const intValue = properties.getIcePropertyAsInt("Ice.Default.Host");
            test(intValue == 0);

            const listValue = properties.getIcePropertyAsList("Ice.Default.Host");
            test(listValue.length == 0);

            out.writeLine("ok");
        }

        {
            out.write("testing that getting an unknown ice property throws an exception...");
            const properties = Ice.createProperties();
            try {
                properties.getIceProperty("Ice.UnknownProperty");
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.PropertyException);
            }
            out.writeLine("ok");
        }

        {
            out.write("testing that setting an unknown ice property throws an exception...");
            const properties = Ice.createProperties();
            try {
                properties.setProperty("Ice.UnknownProperty", "bar");
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.PropertyException);
            }
            out.writeLine("ok");
        }

        {
            await using communicator = Ice.initialize(args);
            const properties = communicator.getProperties();

            out.write("testing that creating an object adapter with unknown properties throws an exception...");
            properties.setProperty("FooOA.Router", "router:tcp -h 127.0.0.1 -p 10000");
            properties.setProperty("FooOA.UnknownProperty", "bar");
            try {
                await communicator.createObjectAdapter("FooOA");
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.PropertyException);
            }
            out.writeLine("ok");

            out.write("testing that creating a proxy with unknown properties throws an exception...");
            properties.setProperty("FooProxy", "test:tcp -h 127.0.0.1 -p 10000");
            properties.setProperty("FooProxy.UnknownProperty", "bar");
            try {
                communicator.propertyToProxy("FooProxy");
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.PropertyException);
            }
            out.writeLine("ok");

            communicator.shutdown();
        }

        {
            out.write("testing that passing a property multiple times on the command line uses the last value... ");
            const properties = Ice.createProperties(["--Ice.MessageSizeMax=10", "--Ice.MessageSizeMax=20"]);
            test(properties.getIceProperty("Ice.MessageSizeMax") == "20");
            out.writeLine("ok");
        }

        {
            out.write("testing that trying to read a non-numeric value as an int throws... ");
            const properties = Ice.createProperties();
            try {
                properties.setProperty("Foo", "bar");
                properties.getPropertyAsInt("Foo");
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.PropertyException);
            }
            out.writeLine("ok");
        }
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            [communicator, args] = this.initialize(args);
            await this.allTests(args);
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
