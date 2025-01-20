% Copyright (c) ZeroC, Inc.

classdef SliceType
    % Don't use an enumeration as comparing enumerators with integral values is significantly slower.
    properties(Constant)
        NoSlice = uint8(0)
        ValueSlice = uint8(1)
        ExceptionSlice = uint8(2)
    end
end
