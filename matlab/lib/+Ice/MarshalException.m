classdef (Sealed) MarshalException < Ice.ProtocolException
    %MARSHALEXCEPTION The exception that is thrown when an error occurs during marshaling or unmarshaling.

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
