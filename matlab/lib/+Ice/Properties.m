classdef Properties < IceInternal.WrapperObject
    % Properties   Summary of Properties
    %
    % A property set used to configure Ice and Ice applications.
    % Properties are key/value pairs, with both keys and values
    % being strings. By convention, property keys should have the
    % form application-name[.category[.sub-category]].name.
    %
    % Property Methods:
    %   getProperty - Get a property by key.
    %   getPropertyWithDefault - Get a property by key.
    %   getPropertyAsInt - Get a property as an integer.
    %   getPropertyAsIntWithDefault - Get a property as an integer.
    %   getPropertyAsList - Get a property as a list of strings.
    %   getPropertyAsListWithDefault - Get a property as a list of strings.
    %   getPropertiesForPrefix - Get all properties whose keys begins with
    %     a prefix.
    %   setProperty - Set a property.
    %   getCommandLineOptions - Get a sequence of command-line options that
    %     is equivalent to this property set.
    %   parseCommandLineOptions - Convert a sequence of command-line options
    %     into properties.
    %   parseIceCommandLineOptions - Convert a sequence of command-line options
    %     into properties.
    %   load - Load properties from a file.
    %   clone - Create a copy of this property set.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    methods
        function obj = Properties(impl)
            if ~isa(impl, 'lib.pointer')
                throw(MException('Ice:ArgumentException', 'invalid argument'));
            end
            obj = obj@IceInternal.WrapperObject(impl);
        end
        function r = getProperty(obj, key)
            % getProperty - Get a property by key. If the property is not set,
            %   an empty string is returned.
            %
            % Parameters:
            %   key (char) - The property key.
            %
            % Returns (char) - The property value.

            r = obj.iceCallWithResult('getProperty', key);
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

            v = libpointer('int32Ptr', 0);
            obj.iceCall('getPropertyAsInt', key, v);
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
            % Returns (cell arry of char) - The property value interpreted as
            %   a list of strings.

            r = obj.iceCallWithResult('getPropertyAsList', key);
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
            %   def (cell array of char) - The default value to use if the
            %     property is not set.
            %
            % Returns (cell arry of char) - The property value interpreted as
            %   a list of strings, or the default value.

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
            % Returns (containers.Map) - The matching property set.

            r = obj.iceCallWithResult('getPropertiesForPrefix', prefix);
        end
        function setProperty(obj, key, value)
            % setProperty - Set a property. To unset a property, set it to
            %   the empty string.
            %
            % Parameters:
            %   key (char) - The property key.
            %   value (char) - The property value.

            obj.iceCall('setProperty', key, value);
        end
        function r = getCommandLineOptions(obj)
            % getCommandLineOptions - Get a sequence of command-line options
            %   that is equivalent to this property set. Each element of the
            %   returned sequence is a command-line option of the form
            %   --key=value.
            %
            % Returns (cell array of char) - The command line options for this
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
            %   options (cell array of char) - The command-line options.
            %
            % Returns (cell array of char) The command-line options that do
            %   not start with the specified prefix, in their original order.

            r = obj.iceCallWithResult('parseCommandLineOptions', prefix, options);
        end
        function r = parseIceCommandLineOptions(obj, options)
            % parseIceCommandLineOptions - Convert a sequence of command-line
            %   options into properties. All options that begin with one of the
            %   following prefixes are converted into properties: "--Ice",
            %   "--IceBox", "--IceGrid", "--IcePatch2", "--IceSSL",
            %   "--IceStorm", "--Freeze", and "--Glacier2".
            %
            % Parameters:
            %   options (cell array of char) - The command-line options.
            %
            % Returns (cell array of char) - The command-line options that do
            %   not start with one of the listed prefixes, in their original
            %   order.

            r = obj.iceCallWithResult('parseIceCommandLineOptions', options);
        end
        function load(obj, file)
            % load - Load properties from a file.
            %
            % Parameters:
            %   file (char) - The property file.

            obj.iceCall('load', file);
        end
        function r = clone(obj)
            % clone - Create a copy of this property set.
            %
            % Returns (Ice.Properties) - A copy of this property set.

            impl = libpointer('voidPtr');
            obj.iceCall('clone', impl);
            r = Ice.Properties(impl);
        end
    end
end
