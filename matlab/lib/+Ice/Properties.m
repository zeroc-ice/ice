classdef Properties < IceInternal.WrapperObject
    %PROPERTIES Represents a set of properties used to configure Ice and Ice-based applications.
    %    A property is a key/value pair, where both the key and the value are strings. By convention, property keys
    %    should have the form `application-name>[.category[.sub-category]].name`.

    % Copyright (c) ZeroC, Inc.

    methods
        function [obj, remArgs] = Properties(args, defaults, impl)
            % Properties - Constructs a new property set.
            %
            % Examples:
            %   properties = Ice.Properties();
            %   [properties, remArgs] = Ice.Properties(args);
            %   [properties, remArgs] = Ice.Properties(args, defaults);
            %
            % Parameters:
            %   args (cell array of char or string array) - A command-line argument vector, possibly containing options
            %     to set properties. If the command-line options include a --Ice.Config option, the corresponding
            %     configuration files are parsed. If the same property is set in a configuration file and in the
            %     argument vector, the argument vector takes precedence.
            %   defaults (Ice.Properties) - A property set used to initialize the default state of the new property set.
            %     Settings in configuration files and the argument vector override these defaults.
            % Returns:
            %   obj (Ice.Properties) - A new property set initialized with the property settings that were removed from
            %      the argument vector and the default property set.
            %   remArgs (string array) - Contains the remaining command-line arguments that were not used to set
            %     properties.
            arguments
                args (1, :) = {}
                defaults Ice.Properties {mustBeScalarOrEmpty} = Ice.Properties.empty
                impl = libpointer('voidPtr') % internal
            end
            if isNull(impl)
                if isempty(defaults)
                    defaultsImpl = libpointer('voidPtr');
                else
                    defaultsImpl = defaults.impl_;
                end
                remArgs = IceInternal.Util.callWithResult('Ice_createProperties', args, defaultsImpl, impl);
            else
                % We ignore args and defaults and use the provided impl.
                remArgs = string.empty(1, 0);
            end

            obj@IceInternal.WrapperObject(impl);
        end

        function r = getProperty(obj, key)
            % getProperty - Get a property by key. If the property is not set,
            %   an empty string is returned.
            %
            % Parameters:
            %   key (char) - The property key.
            %
            % Returns (char) - The property value.

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
            end
            r = obj.iceCallWithResult('getProperty', key);
        end
        function r = getIceProperty(obj, key)
            % getIceProperty - Get an Ice property by key. If the property is not set,
            %   its default value is returned.
            %
            % Parameters:
            %   key (char) - The property key.
            %
            % Returns (char) - The property value.

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
            end
            r = obj.iceCallWithResult('getIceProperty', key);
        end
        function r = getPropertyWithDefault(obj, key, def)
            % getPropertyWithDefault - Get a property by key. If the property
            %   is not set, the given default value is returned.
            %
            % Parameters:
            %   key (char) - The property key.
            %   def (char) - The default value to use if the property does not
            %     exist.
            %
            % Returns (char) - The property value or the default value.

            r = obj.iceCallWithResult('getPropertyWithDefault', key, def);
        end
        function r = getPropertyAsInt(obj, key)
            % getPropertyAsInt - Get a property as an integer. If the property
            %   is not set, 0 is returned.
            %
            % Parameters:
            %   key (char) - The property key.
            %
            % Returns (int32) - The property value interpreted as an integer.

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
            end
            v = libpointer('int32Ptr', 0);
            obj.iceCall('getPropertyAsInt', key, v);
            r = v.Value;
        end
        function r = getIcePropertyAsInt(obj, key)
            % getIcePropertyAsInt - Get an Ice property as an integer. If the property
            %   is not set, its default value is returned.
            %
            % Parameters:
            %   key (char) - The property key.
            %
            % Returns (int32) - The property value interpreted as an integer.

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
            end
            v = libpointer('int32Ptr', 0);
            obj.iceCall('getIcePropertyAsInt', key, v);
            r = v.Value;
        end
        function r = getPropertyAsIntWithDefault(obj, key, def)
            % getPropertyAsIntWithDefault - Get a property as an integer. If
            %   the property is not set, the given default value is returned.
            %
            % Parameters:
            %   key (char) - The property key.
            %   def (int32) - The default value to use if the property does
            %     not exist.
            %
            % Returns (int32) - The property value interpreted as an integer,
            %   or the default value.

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
                def (1, 1) int32
            end
            v = libpointer('int32Ptr', 0);
            obj.iceCall('getPropertyAsIntWithDefault', key, def, v);
            r = v.Value;
        end
        function r = getPropertyAsList(obj, key)
            % getPropertyAsList - Get a property as a list of strings. The
            %   strings must be separated by whitespace or comma. If the
            %   property is not set, an empty list is returned. The strings
            %   in the list can contain whitespace and commas if they are
            %   enclosed in single or double quotes. If quotes are mismatched,
            %   an empty list is returned. Within single quotes or double
            %   quotes, you can escape the quote in question with \, e.g.
            %   O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
            %
            % Parameters:
            %   key (char) - The property key.
            %
            % Returns (string array) - The property value interpreted as
            %   a list of strings.

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
            end
            r = obj.iceCallWithResult('getPropertyAsList', key);
        end
        function r = getIcePropertyAsList(obj, key)
            % getIcePropertyAsList - Get an Ice property as a list of strings. The
            %   strings must be separated by whitespace or comma. If the
            %   property is not set, its default is returned. The strings
            %   in the list can contain whitespace and commas if they are
            %   enclosed in single or double quotes. If quotes are mismatched,
            %   an empty list is returned. Within single quotes or double
            %   quotes, you can escape the quote in question with \, e.g.
            %   O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
            %
            % Parameters:
            %   key (char) - The property key.
            %
            % Returns (string array) - The property value interpreted as
            %   a list of strings.

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
            end
            r = obj.iceCallWithResult('getIcePropertyAsList', key);
        end
        function r = getPropertyAsListWithDefault(obj, key, def)
            % getPropertyAsListWithDefault - Get a property as a list of
            %   strings. The strings must be separated by whitespace or comma.
            %   If the property is not set, the given default value is returned.
            %   The strings in the list can contain whitespace and commas if
            %   they are enclosed in single or double quotes. If quotes are
            %   mismatched, an empty list is returned. Within single quotes or
            %   double quotes, you can escape the quote in question with \, e.g.
            %   O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
            %
            % Parameters:
            %   key (char) - The property key.
            %   def (cell array of char or string array) - The default value to use if the
            %     property is not set.
            %
            % Returns (string array) - The property value interpreted as
            %   a list of strings, or the default value.

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
                def (1, :) string
            end
            r = obj.iceCallWithResult('getPropertyAsListWithDefault', key, def);
        end
        function r = getPropertiesForPrefix(obj, prefix)
            % getPropertiesForPrefix - Get all properties whose keys begins with
            %   prefix. If prefix is an empty string, then all properties are
            %   returned.
            %
            % Parameters:
            %   prefix (char) - The prefix to search for (empty string if none).
            %
            % Returns (dictionary) - The matching property set.

            arguments
                obj (1, 1) Ice.Properties
                prefix (1, :) char
            end
            r = obj.iceCallWithResult('getPropertiesForPrefix', prefix);
        end
        function disp(obj)
            % disp - Displays this Properties object.
            %
            % This method is called when the object is displayed in the command window.

            arguments
                obj (1, 1) Ice.Properties
            end
            dict = obj.getPropertiesForPrefix('');
            disp(dict); % can't use builtin('disp') with dictionary
        end
        function setProperty(obj, key, value)
            % setProperty - Set a property. To unset a property, set it to
            %   the empty string.
            %
            % Parameters:
            %   key (char) - The property key.
            %   value (char) - The property value.

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
                value (1, :) char
            end
            obj.iceCall('setProperty', key, value);
        end
        function r = getCommandLineOptions(obj)
            % getCommandLineOptions - Get a sequence of command-line options
            %   that is equivalent to this property set. Each element of the
            %   returned sequence is a command-line option of the form
            %   --key=value.
            %
            % Returns (string array) - The command line options for this
            %   property set.

            r = obj.iceCallWithResult('getCommandLineOptions');
        end
        function r = parseCommandLineOptions(obj, prefix, options)
            % parseCommandLineOptions - Convert a sequence of command-line
            %   options into properties. All options that begin with
            %   "--prefix." are converted into properties. If the prefix is
            %   empty, all options that begin with "--" are converted to
            %   properties.
            %
            % Parameters:
            %   prefix (char) - The property prefix, or an empty string to
            %     convert all options starting with "--".
            %   options (cell array of char or string array) - The command-line options.
            %
            % Returns (string array) The command-line options that do
            %   not start with the specified prefix, in their original order.

            arguments
                obj (1, 1) Ice.Properties
                prefix (1, :) char
                options (1, :) string
            end
            r = obj.iceCallWithResult('parseCommandLineOptions', prefix, options);
        end
        function r = parseIceCommandLineOptions(obj, options)
            % parseIceCommandLineOptions - Convert a sequence of command-line
            %   options into properties. All options that begin with one of the
            %   following prefixes are converted into properties: "--Ice",
            %   "--IceBox", "--IceGrid", "--IceSSL",
            %   "--IceStorm", and "--Glacier2".
            %
            % Parameters:
            %   options (cell array of char or string array) - The command-line options.
            %
            % Returns (string array) - The command-line options that do
            %   not start with one of the listed prefixes, in their original
            %   order.

            arguments
                obj (1, 1) Ice.Properties
                options (1, :) string
            end
            r = obj.iceCallWithResult('parseIceCommandLineOptions', options);
        end
        function load(obj, file)
            % load - Load properties from a file.
            %
            % Parameters:
            %   file (char) - The property file.

            arguments
                obj (1, 1) Ice.Properties
                file (1, :) char
            end
            obj.iceCall('load', file);
        end
        function r = clone(obj)
            % clone - Create a copy of this property set.
            %
            % Returns (Ice.Properties) - A copy of this property set.

            arguments
                obj (1, 1) Ice.Properties
            end
            impl = libpointer('voidPtr');
            obj.iceCall('clone', impl);
            r = Ice.Properties({}, Ice.Properties.empty, impl);
        end
    end
end
