classdef (Hidden) ReadEncaps < handle
    % Copyright (c) ZeroC, Inc.

    properties
        endPos int32
        encoding
        encoding_1_0
        decoder
        next
    end
    methods
        function reset(obj)
            obj.decoder = [];
        end
        function setEncoding(obj, encoding)
            obj.encoding = encoding;
            obj.encoding_1_0 = encoding.major == 1 && encoding.minor == 0;
        end
    end
end
