classdef (Sealed) OperationNotExistException < Ice.RequestFailedException
    %OPERATIONNOTEXISTEXCEPTION The exception that is thrown when a dispatch could not find the operation carried by
    %   the request on the target servant. This is typically due to a mismatch in the Slice definitions, such as the
    %   client using Slice definitions newer than the server's.

    % Copyright (c) ZeroC, Inc.
end
