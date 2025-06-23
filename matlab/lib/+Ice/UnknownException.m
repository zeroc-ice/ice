classdef UnknownException < Ice.DispatchException
    %UNKNOWNEXCEPTION This exception is raised if an operation call on a server raises an unknown exception. For
    %   example, for C++, this exception is raised if the server throws a C++ exception that is not directly or
    %   indirectly derived from Ice::LocalException or Ice::UserException.

    % Copyright (c) ZeroC, Inc.
end
