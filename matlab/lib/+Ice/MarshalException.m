classdef (Sealed) MarshalException < Ice.ProtocolException
    %MARSHALEXCEPTION This exception is raised for errors during marshaling or unmarshaling data.

    % Copyright (c) ZeroC, Inc.

    methods(Hidden)
        function obj = MarshalException(what)
            % Convenience constructor without an errID.
            if nargin == 0 % default constructor
                superArgs = {};
            else
                assert(nargin == 1, 'Invalid number of arguments');
                superArgs = {'Ice:MarshalException', what};
            end
            obj@Ice.ProtocolException(superArgs{:});
        end
    end
end
