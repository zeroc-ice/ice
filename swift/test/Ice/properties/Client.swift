//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

open class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Client()
    }
}

public class Client: TestHelperI {
    public override func run(args _: [String]) throws {
        let writer = getWriter()
        do {
            writer.write("testing load properties from UTF-8 path... ")
            let (properties, _) = try Ice.createProperties()
            try properties.load("./config/中国_client.config")
            try test(properties.getProperty("Ice.Trace.Network") == "1")
            try test(properties.getProperty("Ice.Trace.Protocol") == "1")
            try test(properties.getProperty("Config.Path") == "./config/中国_client.config")
            try test(properties.getProperty("Ice.ProgramName") == "PropertiesClient")
            writer.writeLine("ok")
        }

        do {
            writer.write("testing using Ice.Config with multiple config files... ")
            let args1 = ["--Ice.Config=config/config.1, config/config.2, config/config.3"]
            let (properties, _) = try Ice.createProperties(args: args1)
            try test(properties.getProperty("Config1") == "Config1")
            try test(properties.getProperty("Config2") == "Config2")
            try test(properties.getProperty("Config3") == "Config3")
            writer.writeLine("ok")
        }

        do {
            writer.write("testing configuration file escapes... ")
            let args1 = ["--Ice.Config=config/escapes.cfg"]
            let (properties, _) = try Ice.createProperties(args: args1)

            let props = [("Foo\tBar", "3"),
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
                         ("BServer", "\\server\\dir")]

            for prop in props {
                try test(properties.getProperty(prop.0) == prop.1)
            }
            writer.writeLine("ok")
        }
    }
}
