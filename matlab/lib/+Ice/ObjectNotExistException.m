% ObjectNotExistException   Summary of ObjectNotExistException
%
% This exception is raised if an object does not exist on the server, that is, if no facets with the given identity
% exist.

% Copyright (c) ZeroC, Inc.

classdef (Sealed) ObjectNotExistException < Ice.RequestFailedException
end
