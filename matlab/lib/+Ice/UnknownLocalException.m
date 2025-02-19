% UnknownLocalException   Summary of UnknownLocalException
%
% This exception is raised if an operation call on a server raises a  local exception. Because local exceptions are
% not transmitted by the Ice protocol, the client receives all local exceptions raised by the server as
% UnknownLocalException. The only exception to this rule are all exceptions derived from
% DispatchException, which are transmitted by the Ice protocol even though they are declared local.

% Copyright (c) ZeroC, Inc.

classdef (Sealed) UnknownLocalException < Ice.UnknownException
end
