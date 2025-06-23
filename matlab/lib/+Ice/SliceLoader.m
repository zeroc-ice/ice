classdef (Abstract) SliceLoader < handle
    %SLICELOADER Creates class and exception instances from Slice type IDs.
    %
    %   SliceLoader Methods:
    %     newInstance - Creates a class or exception instance from a Slice type ID.

    methods(Abstract)
        %NEWINSTANCE Creates a class or exception instance from a Slice type ID.
        %
        %   Input Arguments
        %     typeId - The Slice type ID or compact ID.
        %       character vector
        %
        %    Output Arguments
        %      r - The new class instance or exception instance, or an empty array if the implementation cannot find a
        %        class or exception for typeId.
        %        Ice.Value | Ice.UserException | empty array
        r = newInstance(obj, typeId)
    end
end
