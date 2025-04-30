# Copyright (c) ZeroC, Inc.

module Ice
    class InitializationData
        def initialize(properties=nil, sliceLoader=nil)
            @properties = properties
            @sliceLoader = sliceLoader
        end

        attr_accessor :properties, :sliceLoader
    end
end
