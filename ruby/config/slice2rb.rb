#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$:.unshift File.join(File.dirname(__FILE__), "..", "ruby")

require 'IceRuby'

rc = Ice::compile(ARGV)
exit(rc)
