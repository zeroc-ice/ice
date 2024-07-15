#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

module Ice
    class InitializationData
        def initialize(properties=nil, logger=nil)
            @properties = properties
            @logger = logger
        end

        attr_accessor :properties, :logger
    end
end
