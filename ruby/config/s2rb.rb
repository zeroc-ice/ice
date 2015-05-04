#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$:.unshift File.join(File.dirname(__FILE__), "..", "ruby")

# Fix up the environment PATH under Windows so that the plugin can load.
if RUBY_PLATFORM =~ /mswin|mingw|cygwin/
        path = ENV["PATH"]

        iceBinDir = File.join(File.dirname(__FILE__), "..", "..", "cpp", "bin")
        programFiles = "ProgramFiles"
        suffix = ""

        # 64 bit windows machine?
        arch1 = ENV['PROCESSOR_ARCHITECTURE']
        arch2 = ENV['PROCESSOR_ARCHITEW6432']
        if arch1 == "AMD64" || arch1 == "IA64" || arch2 == "AMD64" || arch2 == "IA64"
                programFiles += "(x86)"
                
                # 64 bit ruby?
                if RUBY_PLATFORM == "x64-mingw32"
                        suffix = "x64"
                end
        end
        path = path + ";" + File.join(ENV[programFiles], "ZeroC","Ice-3.6.0-ThirdParty", "bin", suffix)
        path = path + ";" + File.join(iceBinDir, suffix)

        ENV['PATH'] = path
end

require 'IceRuby'

rc = Ice::compile(ARGV)
exit(rc)
