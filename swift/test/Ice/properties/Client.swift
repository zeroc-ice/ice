// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI, @unchecked Sendable {
    override public func run(args _: [String]) async throws {
        let output = getWriter()

        output.write("testing load properties exception... ")
        do {
            let properties = Ice.createProperties()
            try properties.load("./config/xxxx.config")
            try test(false)
        } catch let error as LocalException {
            // We don't map FileException to Swift - this allows us to test unmapped C++ exceptions.
            try test(error.ice_id() == "::Ice::FileException")
        }
        output.writeLine("ok")

        do {
            output.write("testing load properties from UTF-8 path... ")
            let properties = Ice.createProperties()
            try properties.load("./config/中国_client.config")
            try test(properties.getIceProperty("Ice.Trace.Network") == "1")
            try test(properties.getIceProperty("Ice.Trace.Protocol") == "1")
            try test(properties.getProperty("Config.Path") == "./config/中国_client.config")
            try test(properties.getIceProperty("Ice.ProgramName") == "PropertiesClient")
            output.writeLine("ok")
        }

        do {
            output.write("testing using Ice.Config with multiple config files... ")
            let args1 = ["--Ice.Config=config/config.1, config/config.2, config/config.3"]
            let properties = try Ice.createProperties(args1)
            try test(properties.getProperty("Config1") == "Config1")
            try test(properties.getProperty("Config2") == "Config2")
            try test(properties.getProperty("Config3") == "Config3")
            output.writeLine("ok")
        }

        do {
            output.write("testing configuration file escapes... ")
            let args1 = ["--Ice.Config=config/escapes.cfg"]
            let properties = try Ice.createProperties(args1)

            let props = [
                ("Foo\tBar", "3"),
                ("Foo\\tBar", "4"),
                ("Escape\\ Space", "2"),
                ("Prop1", "1"),
                ("Prop2", "2"),
                ("Prop3", "3"),
                ("My Prop1", "1"),
                ("My Prop2", "2"),
                ("My.Prop1", "a property"),
                ("My.Prop2", "a     property"),
                ("My.Prop3", "  a     property  "),
                ("My.Prop4", "  a     property  "),
                ("My.Prop5", "a \\ property"),
                ("foo=bar", "1"),
                ("foo#bar", "2"),
                ("foo bar", "3"),
                ("A", "1"),
                ("B", "2 3 4"),
                ("C", "5=#6"),
                ("AServer", "\\\\server\\dir"),
                ("BServer", "\\server\\dir"),
            ]

            for prop in props {
                try test(properties.getProperty(prop.0) == prop.1)
            }
            output.writeLine("ok")
        }

        do {
            output.write("testing arg parsing...")
            var args1 = ["--Foo=1", "--Ice.Default.InvocationTimeout=12345", "-T", "--Bar=2"]
            let properties1 = try Ice.createProperties(args1)
            var properties2 = try Ice.createProperties(&args1)
            try test(properties1.getIcePropertyAsInt("Ice.Default.InvocationTimeout") == 12345)
            try test(properties2.getIcePropertyAsInt("Ice.Default.InvocationTimeout") == 12345)
            try test(args1 == ["--Foo=1", "-T", "--Bar=2"])

            args1 = ["--Ice.Default.InvocationTimeout=10000"]
            properties2 = try Ice.createProperties(&args1)
            try test(args1 == [])

            args1 = ["--Foo=1", "--Ice.Default.InvocationTimeout=12345", "-T", "--Bar=2"]
            let communicator = try Ice.initialize(&args1)
            defer {
                communicator.destroy()
            }
            try test(
                communicator.getProperties().getIcePropertyAsInt("Ice.Default.InvocationTimeout")
                    == 12345)
            try test(args1 == ["--Foo=1", "-T", "--Bar=2"])
            output.writeLine("ok")
        }

        do {
            output.write("testing ice properties with set default values...")
            let properties = Ice.createProperties()

            let toStringMode = properties.getIceProperty("Ice.ToStringMode")
            try test(toStringMode == "Unicode")

            let closeTimeout = try properties.getIcePropertyAsInt(
                "Ice.Connection.Client.CloseTimeout")
            try test(closeTimeout == 10)

            let retryIntervals = properties.getIcePropertyAsList("Ice.RetryIntervals")
            try test(retryIntervals == ["0"])

            output.writeLine("ok")
        }

        do {
            output.write("testing ice properties with unset default values...")
            let properties = Ice.createProperties()

            let stringValue = properties.getIceProperty("Ice.Admin.Router")
            try test(stringValue == "")

            let intValue = try properties.getIcePropertyAsInt("Ice.Admin.Router")
            try test(intValue == 0)

            let listValue = properties.getIcePropertyAsList("Ice.Admin.Router")
            try test(listValue == [])

            output.writeLine("ok")
        }

        do {
            output.write("testing that trying to read a non-numeric value as an int throws...")
            let properties = Ice.createProperties()

            do {
                // Cannot test in Swift since this generates a fatal error.
                properties.setProperty(key: "Foo", value: "bar")
                _ = try properties.getPropertyAsInt("Foo")
                try test(false)

            } catch _ as PropertyException {
            }

            output.writeLine("ok")
        }
    }
}
