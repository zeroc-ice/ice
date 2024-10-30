#!/usr/bin/env ruby
# encoding: utf-8
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

require 'Ice'

class Client < ::TestHelper
    def run(args)

        print "testing load properties exception... "
        props = Ice.createProperties()
        begin
            props.load("./config/xxxx.config")
        rescue Ice::LocalException => ex
            # The corresponding C++ exception (Ice::FileException) is not mapped to Ruby.
            test(ex.message["error while accessing file './config/xxxx.config'"])
        end
        puts "ok"

        print "testing load properties from UTF-8 path... "
        properties = Ice.createProperties(args)
        properties.load("./config/中国_client.config")
        test(properties.getProperty("Ice.Trace.Network") == "1")
        test(properties.getProperty("Ice.Trace.Protocol") == "1")
        test(properties.getProperty("Config.Path").eql? "./config/中国_client.config")
        test(properties.getProperty("Ice.ProgramName") == "PropertiesClient")
        puts "ok"

        print "testing using Ice.Config with multiple config files... "
        properties = Ice.createProperties(["--Ice.Config=config/config.1, config/config.2, config/config.3"]);
        test(properties.getProperty("Config1") == "Config1");
        test(properties.getProperty("Config2") == "Config2");
        test(properties.getProperty("Config3") == "Config3");
        puts "ok"

        print "testing configuration file escapes... "
        properties = Ice.createProperties(["--Ice.Config=config/escapes.cfg"])

        props = { "Foo\tBar" => "3",
                  "Foo\\tBar" => "4",
                  "Escape\\ Space" => "2",
                  "Prop1" => "1",
                  "Prop2" => "2",
                  "Prop3" => "3",
                  "My Prop1" => "1",
                  "My Prop2" => "2",
                  "My.Prop1" => "a property",
                  "My.Prop2" => "a     property",
                  "My.Prop3" => "  a     property  ",
                  "My.Prop4" => "  a     property  ",
                  "My.Prop5" => "a \\ property",
                  "foo=bar" => "1",
                  "foo#bar" => "2",
                  "foo bar" => "3",
                  "A" => "1",
                  "B" => "2 3 4",
                  "C" => "5=#6",
                  "AServer" => "\\\\server\\dir",
                  "BServer" => "\\server\\dir" }

        props.each do |key, value|
            test(properties.getProperty(key) == value)
        end
        puts "ok"

        print "testing ice properties with set default values..."
        properties = Ice.createProperties(args)

        toStringMode = properties.getIceProperty("Ice.ToStringMode")
        test(toStringMode == "Unicode")

        closeTimeout = properties.getIcePropertyAsInt("Ice.Connection.Client.CloseTimeout")
        test(closeTimeout == 10)

        retryIntervals = properties.getIcePropertyAsList("Ice.RetryIntervals")
        test(retryIntervals == ["0"])
        puts "ok"

        print "testing ice properties with unset default values..."
        properties = Ice.createProperties(args)

        stringValue = properties.getIceProperty("Ice.Admin.Router")
        test(stringValue == "")

        intValue = properties.getIcePropertyAsInt("Ice.Admin.Router")
        test(intValue == 0)

        listValue = properties.getIcePropertyAsList("Ice.Admin.Router")
        test(listValue == [])
        puts "ok"

        print "testing that getting an unknown ice property throws an exception..."
        begin
            properties = Ice.createProperties(args)
            properties.getIceProperty("Ice.UnknownProperty")
            test(false)
        rescue Ice::PropertyException => ex
            test(ex.message["unknown Ice property: Ice.UnknownProperty"])
        end
        puts "ok"

    end
end
