classdef ImplicitContext < IceInternal.WrapperObject
    %IMPLICITCONTEXT Represents the request context associated with a communicator.
    %   When you make a remote invocation without an explicit request context parameter, Ice uses the per-proxy request
    %   context (if any) combined with the ImplicitContext associated with your communicator. The property
    %   "Ice.ImplicitContext" controls if your communicator has an associated implicit context, and when it does,
    %   whether this implicit context is per-thread or shared by all threads.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden, Access = ?Ice.Communicator)
        function obj = ImplicitContext(impl)
            assert(isa(impl, 'lib.pointer'));
            obj@IceInternal.WrapperObject(impl);
        end
    end
    methods
        function r = getContext(obj)
            %GETCONTEXT Gets a copy of the underlying context.
            %
            %   Output Arguments
            %     r - A copy of the underlying context.
            %       dictionary(string, string) scalar

            arguments
                obj (1, 1) Ice.ImplicitContext
            end
            r = obj.iceCallWithResult('getContext');
        end

        function setContext(obj, newContext)
            %SETCONTEXT Sets the underlying context.
            %
            %   Input Arguments
            %     newContext - The new context.
            %       dictionary(string, string)

            arguments
                obj (1, 1) Ice.ImplicitContext
                newContext (1, 1) dictionary {Ice.mustBeStringStringDictionary}
            end
            obj.iceCall('setContext', newContext);
        end

        function r = containsKey(obj, key)
            %CONTAINSKEY Checks if this key has an associated value in the underlying context.
            %
            %   Input Arguments
            %     key - The key.
            %       character vector
            %
            %   Output Arguments
            %     r - True if the key has an associated value, false otherwise.
            %       logical scalar

            arguments
                obj (1, 1) Ice.ImplicitContext
                key (1, :) char
            end
            r = obj.iceCallWithResult('containsKey', key);
        end

        function r = get(obj, key)
            %GET Gets the value associated with the given key in the underlying context. Returns an empty string if no
            %   value is associated with the key.
            %
            %   See also containsKey.
            %
            %   Input Arguments
            %     key - The key.
            %       character vector
            %
            %   Output Arguments
            %     r - The value associated with the key.
            %       character vector

            arguments
                obj (1, 1) Ice.ImplicitContext
                key (1, :) char
            end
            r = obj.iceCallWithResult('get', key);
        end

        function r = put(obj, key, value)
            %PUT Creates or updates a key/value entry in the underlying context.
            %
            %   Input Arguments
            %     key - The key.
            %       character vector
            %     value - The value.
            %       character vector
            %
            %   Output Arguments
            %     r - The previous value associated with the key, if any.
            %       character vector

            arguments
                obj (1, 1) Ice.ImplicitContext
                key (1, :) char
                value (1, :) char
            end
            r = obj.iceCallWithResult('put', key, value);
        end

        function r = remove(obj, key)
            %REMOVE Removes the entry for the given key in the underlying context.
            %
            %   Input Arguments
            %     key - The key.
            %       character vector
            %
            %   Output Arguments
            %     r - The value associated with the key, if any.
            %       character vector

            arguments
                obj (1, 1) Ice.ImplicitContext
                key (1, :) char
            end
            r = obj.iceCallWithResult('remove', key);
        end
    end
end
