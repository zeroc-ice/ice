classdef (Abstract) SliceLoader < handle
    % SliceLoader   Creates class and exception instances from Slice type IDs.
    %
    % SliceLoader methods:
    %   newInstance - Creates a class or exception instance from a Slice type ID.

    methods(Abstract)
        % newInstance - Creates a class or exception instance from a Slice type ID.
        %
        % Parameters:
        %   typeId (char) - The Slice type ID or compact ID.
        %
        % Returns (handle or []) - The new class instance or exception instance, or an empty array if the implementation
        %   cannot find a class or exception for typeId.
        r = newInstance(obj, typeId)
    end
end
