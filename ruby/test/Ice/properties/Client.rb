#!/usr/bin/env ruby
# encoding: UTF-8
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'Ice'

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

class Client < Ice::Application
    def run(args)
        print "testing load properties from UTF-8 path using Ice::Application... "
        properties = Ice::Application::communicator().getProperties()
        test(properties.getProperty("Ice.Trace.Network") == "1")
        test(properties.getProperty("Ice.Trace.Protocol") == "1")
        test(properties.getProperty("Config.Path").eql? "./config/中国_client.config")
        test(properties.getProperty("Ice.ProgramName") == "PropertiesClient")
        test(Ice::Application::appName() == properties.getProperty("Ice.ProgramName"))
        puts "ok"
        return true
    end
end

print "testing load properties from UTF-8 path... "
properties = Ice.createProperties(ARGV)
properties.load("./config/中国_client.config")
test(properties.getProperty("Ice.Trace.Network") == "1")
test(properties.getProperty("Ice.Trace.Protocol") == "1")
test(properties.getProperty("Config.Path").eql? "./config/中国_client.config")
test(properties.getProperty("Ice.ProgramName") == "PropertiesClient")
puts "ok"

app = Client.new()
app.main(ARGV, "./config/中国_client.config")

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

exit(0)
