# Copyright (c) ZeroC, Inc.

module Ice
    class InitializationData
        def initialize(properties=nil, logger=nil, sliceLoader=nil)
            @properties = properties
            @logger = logger
            @sliceLoader = sliceLoader
        end

        attr_accessor :properties, :logger, :sliceLoader
    end
end
