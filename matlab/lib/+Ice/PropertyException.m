% Copyright (c) ZeroC, Inc.

classdef (Sealed) PropertyException < Ice.LocalException
    %PROPERTYEXCEPTION This exception is raised when there is an error while getting or setting a property. For
    %   example, when trying to set an unknown Ice property.
end
