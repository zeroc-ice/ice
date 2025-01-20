% MarshalException   Summary of MarshalException
%
% This exception is raised for errors during marshaling or unmarshaling data.

% Copyright (c) ZeroC, Inc.

classdef (Sealed) MarshalException < Ice.ProtocolException
    methods
        % Convenience constructor without an errID.
        function obj = MarshalException(what)
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
