#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

require 'IceRuby'
require 'thread'

module Ice

    #
    # Convenience function for locating the directory containing the
    # Slice files.
    #
    def Ice.getSliceDir
        #
        # Get the parent of the directory containing this file
        # (Ice.rb).
        #
        rbHome = File::join(File::dirname(__FILE__), "..")

        #
        # For an installation from a source distribution, a binary
        # tarball, or a Windows installer, the "slice" directory is a
        # sibling of the "rb" directory.
        #
        dir = File::join(rbHome, "slice")
        if File::exist?(dir)
            return File::expand_path(dir)
        end

        #
        # In a source distribution, the "slice" directory is one level
        # higher.
        #
        dir = File::join(rbHome, "..", "slice")
        if File::exist?(dir)
            return File::expand_path(dir)
        end

        if RUBY_PLATFORM =~ /linux/i
            #
            # Check the default Linux location.
            #
            dir = File::join("/", "usr", "share", "ice", "slice")
            if File::exist?(dir)
                return dir
            end
        end

        return nil
    end

    #
    # Provide some common functionality for structs
    #
    module Inspect_mixin
        def inspect
            ::Ice::__stringify(self, self.class::ICE_TYPE)
        end
    end
end

#
# Pull in other files.
#

# First fundamental types that the generated code can use.
require_relative 'Ice/Exception.rb'
require_relative 'Ice/Proxy.rb'
require_relative 'Ice/Value.rb'

require_relative 'Ice/BuiltinSequences.rb'
require_relative 'Ice/CompressBatch.rb'
require_relative 'Ice/ConnectionClose.rb'
require_relative 'Ice/Context.rb'
require_relative 'Ice/EndpointSelectionType.rb'
require_relative 'Ice/EndpointTypes.rb'
require_relative 'Ice/IdentitySpaceship.rb'
require_relative 'Ice/InitializationData.rb'
require_relative 'Ice/LocalExceptions.rb'
require_relative 'Ice/OperationMode.rb'
require_relative 'Ice/ToStringMode.rb'
require_relative 'Ice/Version.rb'

# Files that define proxies must be last.
require_relative 'Ice/Locator.rb'
require_relative 'Ice/Metrics.rb'
require_relative 'Ice/Process.rb'
require_relative 'Ice/RemoteLogger.rb'
require_relative 'Ice/Router.rb'

module Ice
    Protocol_1_0 = ProtocolVersion.new(1, 0)
    Encoding_1_0 = EncodingVersion.new(1, 0)
    Encoding_1_1 = EncodingVersion.new(1, 1)
end
