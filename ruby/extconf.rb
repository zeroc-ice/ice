# Copyright (c) ZeroC, Inc.

require "mkmf"
require "rbconfig"
require 'open-uri'
require 'fileutils'

if RUBY_PLATFORM =~ /mswin|mingw/
    puts "MinGW is not supported with Ice for Ruby."
    exit 1
end

# On OSX & Linux bzlib.h is required.
if not have_header("bzlib.h") then
    puts "Bzip2 header files are missing."
    exit 1
end

if RUBY_PLATFORM =~ /linux/
    # On Linux openssl is required for IceSSL.
    if not have_header("openssl/ssl.h") then
        puts "OpenSSL header files are missing."
        exit 1
    end
end

$INCFLAGS << ' -Idist/ice/cpp/include'
$INCFLAGS << ' -Idist/ice/cpp/include/generated'
$INCFLAGS << ' -Idist/ice/cpp/src'
$INCFLAGS << ' -Idist/ice/cpp/src/Ice/generated'
$INCFLAGS << ' -Idist/ice/cpp/src/IceDiscovery/generated'
$INCFLAGS << ' -Idist/ice/cpp/src/IceLocatorDiscovery/generated'

$CPPFLAGS << ' -w'

if RUBY_PLATFORM =~ /darwin/
    $LOCAL_LIBS << ' -lbz2 -framework Security -framework CoreFoundation'
    $CXXFLAGS << ' -std=c++20'
elsif RUBY_PLATFORM =~ /linux/
    $LOCAL_LIBS << ' -lssl -lcrypto -lbz2 -lrt'
    $CXXFLAGS << ' -std=c++17'
end

# Setup the object and source files.
$objs = []
$srcs = []

# Add the plugin source.
Dir["*.cpp"].each do |f|
    $objs << File.basename(f, ".*") + ".o"
    $srcs << f
end

$excluded = ['DLLMain.cpp', 'Main.cpp', 'addDefaultPluginFactories_min.cpp']

def filter(f)
    # Filter sources for each platform.
    if $excluded.include?(f)
        return false
    end

    if f.include?("Schannel")
        return false
    end
    if RUBY_PLATFORM =~ /darwin/ and f.include?("OpenSSL")
        return false
    end
    if !(RUBY_PLATFORM =~ /darwin/) and f.include?("SecureTransport")
         return false
    end
    return true
end

Dir["dist/ice/**/*.cpp"].each do |f|
    if filter File.basename(f)
        $objs << File.dirname(f) + "/" + File.basename(f, ".*") + ".o"
        $srcs << f
    end
end

# The mcpp source.
Dir["dist/ice/mcpp/*.c"].each do |file|
    $objs << File.join("dist/ice/mcpp", File.basename(file, ".*") + ".o")
    $srcs << File.join("dist/ice/mcpp", file)
end

Dir["dist/IceRuby/*.cpp"].each do |file|
    $objs << File.join("dist/IceRuby", File.basename(file, ".*") + ".o")
    $srcs << File.join("dist/IceRuby", file)
end

create_makefile "IceRuby"
