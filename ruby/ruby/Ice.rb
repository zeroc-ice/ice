# Copyright (c) ZeroC, Inc.

require 'IceRuby'

# First fundamental types that the generated code can use.
require_relative 'Ice/Exception.rb'
require_relative 'Ice/Proxy.rb'
require_relative 'Ice/Struct.rb'
require_relative 'Ice/Value.rb'

require_relative 'Ice/BuiltinSequences.rb'
require_relative 'Ice/CompressBatch.rb'
require_relative 'Ice/Context.rb'
require_relative 'Ice/EndpointSelectionType.rb'
require_relative 'Ice/EndpointTypes.rb'
require_relative 'Ice/IdentitySpaceship.rb'
require_relative 'Ice/InitializationData.rb'
require_relative 'Ice/LocalExceptions.rb'
require_relative 'Ice/OperationMode.rb'
require_relative 'Ice/ReplyStatus.rb'
require_relative 'Ice/SliceUtil.rb'
require_relative 'Ice/ToStringMode.rb'
require_relative 'Ice/Version.rb'

# Files that define proxies must be last.
require_relative 'Ice/ProxyFunctions.rb'
require_relative 'Ice/Locator.rb'
require_relative 'Ice/LocatorRegistry.rb'
require_relative 'Ice/Metrics.rb'
require_relative 'Ice/Process.rb'
require_relative 'Ice/RemoteLogger.rb'
require_relative 'Ice/Router.rb'

module Ice
    Protocol_1_0 = ProtocolVersion.new(1, 0)
    Encoding_1_0 = EncodingVersion.new(1, 0)
    Encoding_1_1 = EncodingVersion.new(1, 1)
end
