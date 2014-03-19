#!/usr/bin/env ruby
# encoding: UTF-8
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

exit(0)
