#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$:.unshift File.join(File.dirname(__FILE__), "..", "ruby")

# Fix up the environment PATH under Windows so that the plugin can load.
if RUBY_PLATFORM =~ /mswin|mingw|cygwin/
    ENV['PATH'] = "%s;%s;%s" % [ENV['PATH'],
        File.join(File.dirname(__FILE__), "..", "..", "cpp", "third-party-packages", 
        	      "bzip2.mingw4.7.2", "build", "native", "bin", 
        	      RUBY_PLATFORM == "x64-mingw32" ? "x64" : "Win32"),
        File.join(File.dirname(__FILE__), "..", "..", "cpp", "bin", 
                  RUBY_PLATFORM == "x64-mingw32" ? "x64" : "")]
end

require 'IceRuby'

rc = Ice::compile(ARGV)
exit(rc)
