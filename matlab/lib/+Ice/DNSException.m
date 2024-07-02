% DNSException   Summary of DNSException
%
% This exception indicates a DNS problem. For details on the cause, DNSException.error should be inspected.
%
% DNSException Properties:
%   error - The error number describing the DNS problem.
%   host - The host name that could not be resolved.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef (Sealed) DNSException < Ice.SyscallException
end
