% Copyright (c) ZeroC, Inc.

classdef CustomSliceLoader < Ice.SliceLoader
    methods
        function r = newInstance(obj, typeId)
            switch typeId
                case '::Test::Preserved'
                    obj.preservedCounter = obj.preservedCounter + 1;
                    r = Test.Preserved();
                case '::Test::PNode'
                    obj.nodeCounter = obj.nodeCounter + 1;
                    r = Test.PNode();
                otherwise
                    r = [];
            end
        end
    end
    properties
        preservedCounter int32 = 0
        nodeCounter int32 = 0
    end
end
