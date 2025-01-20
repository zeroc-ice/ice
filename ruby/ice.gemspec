# Copyright (c) ZeroC, Inc.

Gem::Specification.new do |spec|
  spec.name        = 'zeroc-ice'
  spec.version     = '3.8.0a'
  spec.summary     = "ZeroC Ice for Ruby"
  spec.description = <<-eos
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
and the other Ice language mappings and servicespec.

Join us on our user forums if you have questions about Ice.
eos
  spec.authors     = ["ZeroC, Inc."]
  spec.email       = 'info@zeroc.com'
  spec.files       = ['ice.gemspec'] +
                     Dir['dist/lib/**/*.rb'] +
                     Dir['dist/ice/**/*.*'] +
                     Dir['dist/IceRuby/*.*']
                     Dir['scripts/*']
  spec.homepage    = 'https://zeroc.com'
  spec.license     = 'GPL-2.0-only'
  spec.required_ruby_version = '>= 3.0.0'
  spec.require_paths = ['lib', 'dist/lib']
  spec.extensions = ['extconf.rb']
  spec.rdoc_options = ['--exclude=ext/IceRuby/.*\.o$' '--exclude=IceRuby\.(bundle|so)$' '--exclude=scripts/slice2rb$']
  spec.executables   = ['slice2rb']
  spec.bindir        = 'scripts'
end
