classdef Properties < IceInternal.WrapperObject
    %PROPERTIES Represents a set of properties used to configure Ice and Ice-based applications.
    %   A property is a key/value pair, where both the key and the value are strings. By convention, property keys
    %   should have the form `application-name>[.category[.sub-category]].name`.
    %
    %   Ice.Properties Methods:
    %     Properties - Constructs a new property set.
    %     clone - Creates a copy of this property set.
    %     disp - Displays this property set.
    %     getCommandLineOptions - Gets the command-line options for this property set.
    %     getIceProperty - Gets the value of an Ice property.
    %     getIcePropertyAsInt - Gets the value of an Ice property as an integer.
    %     getIcePropertyAsList - Gets the value of an Ice property as a list.
    %     getPropertiesForPrefix - Gets all properties whose keys begin with the specified prefix.
    %     getProperty - Gets the value of a property.
    %     getPropertyAsInt - Gets the value of a property as an integer.
    %     getPropertyAsIntWithDefault - Gets the value of a property as an integer with a default value.
    %     getPropertyAsList - Gets the value of a property as a list.
    %     getPropertyAsListWithDefault - Gets the value of a property as a list with a default value.
    %     getPropertyWithDefault - Gets the value of a property with a default value.
    %     load - Loads properties from a file.
    %     parseCommandLineOptions - Parses command-line options and sets properties.
    %     parseIceCommandLineOptions - Parses Ice command-line options and sets properties.
    %     setProperty - Sets a property to the given value.

    % Copyright (c) ZeroC, Inc.

    methods
        function [obj, remArgs] = Properties(args, defaults, impl)
            %PROPERTIES Constructs a new property set.
            %
            %   Examples
            %     properties = Ice.Properties()
            %     [properties, remArgs] = Ice.Properties(args)
            %     [properties, remArgs] = Ice.Properties(args, defaults)
            %
            %   Input Arguments
            %     args - A command-line argument vector, possibly containing options to set properties. If the
            %       command-line options include an --Ice.Config option, the corresponding configuration file is parsed.
            %       If the same property is set in a configuration file and in the argument vector, the argument vector
            %       takes precedence.
            %       empty cell array (default) | cell array of character | string array
            %     defaults - A property set used to initialize the default state of the new property set. Settings in
            %       configuration files and the argument vector override these defaults.
            %       Ice.Properties empty array (default) | Ice.Properties scalar
            %
            %   Output Arguments
            %     obj - A new property set initialized with the property settings that were removed from the argument
            %       vector and the default property set.
            %       Ice.Properties scalar
            %     remArgs - Contains the remaining command-line arguments that were not used to set properties.
            %       string array
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
            %GETPROPERTY Gets a property by key. If the property is not set, an empty string is returned.
            %
            %   Input Arguments
            %     key - The property key.
            %       character vector
            %
            %   Output Arguments
            %     r - The property value.
            %       character vector

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
            end
            r = obj.iceCallWithResult('getProperty', key);
        end

        function r = getIceProperty(obj, key)
            %GETICEPROPERTY Gets an Ice property by key. If the property is not set, its default value is returned.
            %
            %   Input Arguments
            %     key - The property key.
            %       character vector
            %
            %   Output Arguments
            %     r - The property value.
            %       character vector

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
            end
            r = obj.iceCallWithResult('getIceProperty', key);
        end

        function r = getPropertyWithDefault(obj, key, def)
            %GETPROPERTYWITHDEFAULT Gets a property by key. If the property is not set, the given default value is
            %   returned.
            %
            %   Input Arguments
            %     key - The property key.
            %       character vector
            %     def - The default value to use if the property does not exist.
            %       character vector
            %
            %   Output Arguments
            %     r - The property value or the default value.
            %       character vector

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
                def (1, :) char
            end
            r = obj.iceCallWithResult('getPropertyWithDefault', key, def);
        end

        function r = getPropertyAsInt(obj, key)
            %GETPROPERTYASINT Gets a property as an integer. If the property is not set, 0 is returned.
            %
            %   Input Arguments
            %     key - The property key.
            %       character vector
            %
            %   Output Arguments
            %     r - The property value interpreted as an integer.
            %       int32 scalar

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
            end
            v = libpointer('int32Ptr', 0);
            obj.iceCall('getPropertyAsInt', key, v);
            r = v.Value;
        end

        function r = getIcePropertyAsInt(obj, key)
            %GETICEPROPERTYASINT Gets an Ice property as an integer. If the property is not set, its default value is
            %   returned.
            %
            %   Input Arguments
            %     key - The property key.
            %       character vector
            %
            %   Output Arguments
            %     r - The property value interpreted as an integer.
            %       int32 scalar

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
            end
            v = libpointer('int32Ptr', 0);
            obj.iceCall('getIcePropertyAsInt', key, v);            r = v.Value;
        end

        function r = getPropertyAsIntWithDefault(obj, key, def)
            %GETPROPERTYASINTWITHDEFAULT Gets a property as an integer. If the property is not set, the given default
            %   value is returned.
            %
            %   Input Arguments
            %     key - The property key.
            %       character vector
            %     def - The default value to use if the property does not exist.
            %       int32 scalar
            %
            %   Output Arguments
            %     r - The property value interpreted as an integer, or the default value.
            %       int32 scalar

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
            %GETPROPERTYASLIST Gets a property as a list of strings.
            %   The strings must be separated by whitespace or comma. If the property is not set, an empty list is
            %   returned. The strings in the list can contain whitespace and commas if they are enclosed in single or
            %   double quotes. If quotes are mismatched, an empty list is returned. Within single quotes or double
            %   quotes, you can escape the quote in question with \, e.g. O'Reilly can be written as O'Reilly,
            %   "O'Reilly" or 'O\'Reilly'.
            %
            %   Input Arguments
            %     key - The property key.
            %       character vector
            %
            %   Output Arguments
            %     r - The property value interpreted as a list of strings.
            %       string array

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
            end
            r = obj.iceCallWithResult('getPropertyAsList', key);
        end

        function r = getIcePropertyAsList(obj, key)
            %GETICEPROPERTYASLIST Gets an Ice property as a list of strings.
             %  The strings must be separated by whitespace or comma. If the property is not set, an empty list is
            %   returned. The strings in the list can contain whitespace and commas if they are enclosed in single or
            %   double quotes. If quotes are mismatched, an empty list is returned. Within single quotes or double
            %   quotes, you can escape the quote in question with \, e.g. O'Reilly can be written as O'Reilly,
            %   "O'Reilly" or 'O\'Reilly'.
            %
            %   Input Arguments
            %     key - The property key.
            %       character vector
            %
            %   Output Arguments
            %     r - The property value interpreted as a list of strings.
            %       string array

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
            end
            r = obj.iceCallWithResult('getIcePropertyAsList', key);
        end

        function r = getPropertyAsListWithDefault(obj, key, def)
            %GETPROPERTYASLISTWITHDEFAULT Gets a property as a list of strings.
            %   The strings must be separated by whitespace or comma. If the property is not set, the default value is
            %   returned. The strings in the list can contain whitespace and commas if they are enclosed in single or
            %   double quotes. If quotes are mismatched, an empty list is returned. Within single quotes or double
            %   quotes, you can escape the quote in question with \, e.g. O'Reilly can be written as O'Reilly,
            %   "O'Reilly" or 'O\'Reilly'.
            %
            %   Input Arguments
            %     key - The property key.
            %       character vector
            %     def - The default value to use if the property is not set.
            %       cell array of character | string array
            %
            %   Output Arguments
            %     r - The property value interpreted as a list of strings, or the default value.
            %       string array

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
                def (1, :)
            end
            r = obj.iceCallWithResult('getPropertyAsListWithDefault', key, def);
        end

        function r = getPropertiesForPrefix(obj, prefix)
            %GETPROPERTIESFORPREFIX Gets all properties whose keys begins with prefix. If prefix is an empty string,
            %   then all properties are returned.
            %
            %   Input Arguments
            %     prefix - The prefix to search for (empty string if none).
            %       character vector
            %
            %   Output Arguments
            %     r - The matching property set.
            %       dictionary(string, string) scalar

            arguments
                obj (1, 1) Ice.Properties
                prefix (1, :) char
            end
            r = obj.iceCallWithResult('getPropertiesForPrefix', prefix);
        end

        function disp(obj)
            %DISP Displays this Properties object.
            %   This method is called when the object is displayed in the command window.

            arguments
                obj (1, 1) Ice.Properties
            end
            dict = obj.getPropertiesForPrefix('');
            disp(dict); % can't use builtin('disp') with dictionary
        end

        function setProperty(obj, key, value)
            %SETPROPERTY Sets a property. To unset a property, set it to the empty string.
            %
            %   Input Arguments
            %     key - The property key.
            %       character vector
            %     value - The property value.
            %       character vector

            arguments
                obj (1, 1) Ice.Properties
                key (1, :) char
                value (1, :) char
            end
            obj.iceCall('setProperty', key, value);
        end

        function r = getCommandLineOptions(obj)
            %GETCOMMANDLINEOPTIONS Gets a sequence of command-line options that is equivalent to this property set. Each
            %   element of the returned sequence is a command-line option of the form --key=value.
            %
            %   Output Arguments
            %     r - The command line options for this property set.
            %       string array

            arguments
                obj (1, 1) Ice.Properties
            end
            r = obj.iceCallWithResult('getCommandLineOptions');
        end

        function r = parseCommandLineOptions(obj, prefix, options)
            %PARSECOMMANDLINEOPTIONS Converts a sequence of command-line options into properties. All options that begin
            %   with "--prefix." are converted into properties. If the prefix is empty, all options that begin with "--"
            %   are converted to properties.
            %
            %   Input Arguments
            %     prefix - The property prefix, or an empty string to convert all options starting with "--".
            %       character vector
            %     options - The command-line options.
            %       cell array of character | string array
            %
            %   Output Arguments
            %     r - The command-line options that do not start with the specified prefix, in their original order.
            %       string array

            arguments
                obj (1, 1) Ice.Properties
                prefix (1, :) char
                options (1, :)
            end
            r = obj.iceCallWithResult('parseCommandLineOptions', prefix, options);
        end

        function r = parseIceCommandLineOptions(obj, options)
            %PARSEICECOMMANDLINEOPTIONS Converts a sequence of command-line options into properties. All options that
            %   begin with one of the following prefixes are converted into properties: "--Ice", "--IceBox",
            %   "--IceGrid", "--IceSSL", "--IceStorm", and "--Glacier2".
            %
            %   Input Arguments
            %     options - The command-line options.
            %       cell array of character | string array
            %
            %   Output Arguments
            %     r - The command-line options that do not start with one of the listed prefixes, in their original order.
            %       string array

            arguments
                obj (1, 1) Ice.Properties
                options (1, :)
            end
            r = obj.iceCallWithResult('parseIceCommandLineOptions', options);
        end

        function load(obj, file)
            %LOAD Loads properties from a file.
            %
            %   Input Arguments
            %     file - The property file.
            %       character vector

            arguments
                obj (1, 1) Ice.Properties
                file (1, :) char
            end
            obj.iceCall('load', file);
        end

        function r = clone(obj)
            %CLONE Creates a copy of this property set.
            %
            %   Output Arguments
            %     r - A copy of this property set.
            %       Ice.Properties scalar

            arguments
                obj (1, 1) Ice.Properties
            end
            impl = libpointer('voidPtr');
            obj.iceCall('clone', impl);
            r = Ice.Properties({}, Ice.Properties.empty, impl);
        end
    end
end
