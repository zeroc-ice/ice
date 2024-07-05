% ConnectionManuallyClosedException   Summary of ConnectionManuallyClosedException
%
% This exception is raised by an operation call if the application closes the connection locally using
% Connection.close.
%
% See also Connection.close

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef (Sealed) ConnectionManuallyClosedException < Ice.LocalException
end
