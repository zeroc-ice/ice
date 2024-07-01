% ParseException   Summary of ParseException
%
% This exception is raised if there was an error while parsing a string.
%
% ParseException Properties:
%   str - Describes the failure and includes the string that could not be parsed.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef ParseException < Ice.LocalException
    properties
        % str - Describes the failure and includes the string that could not be parsed.
        str char
    end
    methods
        function obj = ParseException(errID, msg, str)
            if nargin <= 2
                str = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:ParseException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.ParseException';
            end
            obj = obj@Ice.LocalException(errID, msg);
            obj.str = str;
        end
    end
end
