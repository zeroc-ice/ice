classdef (Abstract) SliceLoader < handle
    % SliceLoader   Summary of SliceLoader
    %
    % Creates class and exception instances from Slice type IDs.

    % Copyright (c) ZeroC, Inc.

    methods(Abstract)
        % newInstance - Creates a class or exception instance from a Slice type ID.
        %
        % @param obj This Slice loader instance.
        % @param typeId The Slice type ID or compact ID.
        % @return r The new class instance or exception instance, or an empty array if the implementation
        %         cannot find a class or exception for typeId.
        r = newInstance(obj, typeId)
    end
end
