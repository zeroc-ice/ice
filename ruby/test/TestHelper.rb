#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require "Ice"

class TestHelper
    def initialize
        @communicator = nil
    end

    def getTestEndpoint(properties:nil, num:0, protocol:"")
        if properties.nil?
            properties = @communicator.getProperties()
        end

        if protocol.empty?
            protocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "default")
        end

        port = properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num

        return "#{protocol} -p #{port}"
    end

    def getTestHost(properties:nil)
        if properties.nil?
            properties = @communicator.getProperties()
        end
        return properties.getPropertyWithDefault("Ice.Default.Host", "127.0.0.1")
    end

    def getTestProtocol(properties:nil)
        if properties.nil?
            properties = @communicator.getProperties()
        end
        return properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp")
    end

    def getTestPort(properties:nil, num:0)
        if properties.nil?
            properties = @communicator.getProperties()
        end
        return properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num
    end

    def createTestProperties(args:[])
        properties = Ice::createProperties(args)
        properties.parseCommandLineOptions("Test", args)
        return properties
    end

    def init(initData:nil, properties:nil, args:[])
        if initData.nil?
            initData = Ice::InitializationData.new
            unless properties.nil?
                initData.properties = properties
            else
                initData.properties = self.createTestProperties(args:args)
            end
        end

        communicator = Ice.initialize(initData)
        if @communicator.nil?
            @communicator = communicator
        end
        begin
            yield communicator
        ensure
            communicator.destroy()
        end
    end

    def communicator
        return @communicator
    end

    def shutdown
        unless @communicator.nil?
            @communicator.shutdown()
        end
    end

    def self.run
        begin
            moduleName = File.basename(ARGV[0], ".rb")
            require moduleName
            cls = Object.const_get moduleName
            helper = cls.new
            helper.run(ARGV.drop(1))
            exit(0)
        rescue => ex
            puts $!
            print ex.backtrace.join("\n")
            exit(1)
        end
    end
end

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

TestHelper.run()
