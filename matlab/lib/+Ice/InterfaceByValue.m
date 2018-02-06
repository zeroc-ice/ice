classdef InterfaceByValue < Ice.Value
    % InterfaceByValue   Summary of InterfaceByValue
    %
    % Base class for interoperating with existing applications that pass
    % interfaces by value. The constructor expects the Slice type ID of
    % the interface.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    methods
        function obj = InterfaceByValue(id)
            obj.id = id;
        end
        function id = ice_id(obj)
            id = obj.id;
        end
    end
    methods(Access=protected)
        function iceWriteImpl(obj, os)
            os.startSlice(obj.id, -1, true);
            os.endSlice();
        end
        function iceReadImpl(obj, is)
            is.startSlice();
            is.endSlice();
        end
    end
    properties(Access=private)
        id
    end
end
