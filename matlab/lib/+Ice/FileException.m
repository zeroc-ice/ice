% FileException   Summary of FileException
%
% This exception indicates file errors.
%
% FileException Properties:
%   path - The path of the file responsible for the error.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef FileException < Ice.SyscallException
    properties
        % path - The path of the file responsible for the error.
        path char
    end
    methods
        function obj = FileException(ice_exid, ice_exmsg, error, path)
            if nargin <= 2
                error = 0;
                path = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:FileException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.FileException';
            end
            obj = obj@Ice.SyscallException(ice_exid, ice_exmsg, error);
            obj.path = path;
        end
        function id = ice_id(~)
            id = '::Ice::FileException';
        end
    end
end
