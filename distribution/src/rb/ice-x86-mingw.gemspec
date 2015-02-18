# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

Gem::Specification.new do |s|
  s.name        = 'zeroc-ice-x86-mingw'
  s.version     = '3.6b1'
  s.date        = '2015-02-17'
  s.summary     = "ZeroC Ice for Ruby"
  s.description = <<-eos
The Internet Communications Engine (Ice) provides a robust, proven
platform for developing mission-critical networked applications
with minimal effort. Let Ice handle all of the low-level details
such as network connections, serialization, and concurrency so that
you can focus on your application logic.

This package includes the Ice extension for Ruby, the standard Slice
definition files, and the Slice-to-Ruby compiler. You will need to
install a full Ice distribution if you want to use other Ice language
mappings, or Ice services such as IceGrid, IceStorm and Glacier2.


We provide extensive online documentation for Ice, the Ruby extension,
and the other Ice language mappings and services.

Join us on our user forums if you have questions about Ice.
eos
  s.authors     = ["ZeroC, Inc."]
  s.email       = 'info@zeroc.com'
  s.files       = %w[ICE_LICENSE LICENSE ice-x86-mingw.gemspec] + Dir.glob('lib/**/*.rb') + Dir.glob('lib/*.so') + Dir.glob("slice/**/*.ice")
  s.homepage    = 'https://www.zeroc.com'
  s.license     = 'GPL v2 with exceptions'
  s.rdoc_options = %w[--exclude=IceRuby.so$ --exclude=lib/slice2rb$]
  s.executables << 'slice2rb'
  s.required_ruby_version = '~> 2.1'
end
