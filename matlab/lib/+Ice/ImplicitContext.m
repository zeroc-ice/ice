classdef ImplicitContext < IceInternal.WrapperObject
    % ImplicitContext   Summary of ImplicitContext
    %
    % An interface to associate implicit contexts with communicators.
    %
    % When you make a remote invocation without an explicit context parameter,
    % Ice uses the per-proxy context (if any) combined with the ImplicitContext
    % associated with the communicator.
    %
    % Ice provides several implementations of ImplicitContext. The implementation
    % used depends on the value of the Ice.ImplicitContext property.
    %
    % * None (default)
    %   No implicit context at all.
    % * PerThread
    %   The implementation maintains a context per thread.
    % * Shared
    %   The implementation maintains a single context shared by all threads.
    %
    % ImplicitContext also provides a number of operations to create, update or
    % retrieve an entry in the underlying context without first retrieving a
    % copy of the entire context.
    %
    % ImplicitContext Methods:
    %   getContext - Get a copy of the underlying context.
    %   setContext - Set the underlying context.
    %   containsKey - Check if this key has an associated value in the
    %     underlying context.
    %   get - Get the value associated with the given key in the underlying
    %     context.
    %   put - Create or update a key/value entry in the underlying context.
    %   remove - Remove the entry for the given key in the underlying context.

    % Copyright (c) ZeroC, Inc.

    methods(Hidden, Access = ?Ice.Communicator)
        function obj = ImplicitContext(impl)
            assert(isa(impl, 'lib.pointer'))
            obj@IceInternal.WrapperObject(impl);
        end
    end
    methods
        function r = getContext(obj)
            % getContext - Get a copy of the underlying context.
            %
            % Returns (dictionary): A copy of the underlying context.

            arguments
                obj (1, 1) Ice.ImplicitContext
            end
            r = obj.iceCallWithResult('getContext');
        end
        function setContext(obj, newContext)
            % setContext - Set the underlying context.
            %
            % Parameters:
            %   newContext (dictionary) - The new context.

            arguments
                obj (1, 1) Ice.ImplicitContext
                newContext (1, 1) dictionary % must be configured
            end
            obj.iceCall('setContext', newContext);
        end
        function r = containsKey(obj, key)
            % containsKey - Check if this key has an associated value in the
            %   underlying context.
            %
            % Parameters:
            %   key (char) - The key.
            %
            % Returns (logical) - True if the key has an associated value,
            %   false otherwise.

            arguments
                obj (1, 1) Ice.ImplicitContext
                key (1, :) char
            end
            r = obj.iceCallWithResult('containsKey', key);
        end
        function r = get(obj, key)
            % get - Get the value associated with the given key in the
            %   underlying context. Returns an empty string if no value is
            %   associated with the key. containsKey allows you to distinguish
            %   between an empty string value and no value at all.
            %
            % Parameters:
            %   key (char) - The key.
            %
            % Returns (char) - The value associated with the key.

            arguments
                obj (1, 1) Ice.ImplicitContext
                key (1, :) char
            end
            r = obj.iceCallWithResult('get', key);
        end
        function r = put(obj, key, value)
            % put - Create or update a key/value entry in the underlying
            %   context.
            %
            % Parameters:
            %   key (char) - The key.
            %   value (char) - The value.
            %
            % Returns (char) - The previous value associated with the key,
            %   if any.

            arguments
                obj (1, 1) Ice.ImplicitContext
                key (1, :) char
                value (1, :) char
            end
            r = obj.iceCallWithResult('put', key, value);
        end
        function r = remove(obj, key)
            % remove - Remove the entry for the given key in the underlying
            %   context.
            %
            % Parameters:
            %   key (char) - The key.
            %
            % Returns (char) - The value associated with the key, if any.

            arguments
                obj (1, 1) Ice.ImplicitContext
                key (1, :) char
            end
            r = obj.iceCallWithResult('remove', key);
        end
    end
end
