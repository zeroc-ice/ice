# Copyright (c) ZeroC, Inc.

from typing import Self, final, overload

import IcePy


@final
class Properties:
    """
    Represents a set of properties used to configure Ice and Ice-based applications. A property is a key/value pair,
    where both the key and the value are strings. By convention, property keys should have the form
    ``application-name[.category[.sub-category]].name``.
    """

    @overload
    def __init__(self, args: list[str] | None = None, defaults: Self | None = None): ...

    @overload
    def __init__(self, *, properties: IcePy.Properties): ...

    def __init__(
        self, args: list[str] | None = None, defaults: Self | None = None, properties: IcePy.Properties | None = None
    ):
        """
        Initialize a new instance of the Properties class.

        This constructor loads the configuration files specified by the ``Ice.Config`` property or the
        ``ICE_CONFIG`` environment variable, and then parses Ice properties from ``args``.

        Parameters
        ----------
        args : list[str] | None, optional
            The command-line arguments. Arguments starting with ``--`` and one of the reserved prefixes
            (``Ice``, ``IceSSL``, etc.) are parsed as properties and removed from the list. If there is
            an argument starting with ``--Ice.Config``, the specified configuration file is loaded. When
            the same property is set in both a configuration file and a command-line argument, the
            command-line setting takes precedence.
        defaults : Self | None, optional
            Default values for the new Properties object. Settings in configuration files and command-line
            arguments override these defaults.

        Notes
        -----
        When there is no ``--Ice.Config`` command-line argument, this constructor loads properties from
        the files specified by the ``ICE_CONFIG`` environment variable.

        Examples
        --------
        .. code-block:: python
            # Create a new empty property set.
            properties = Ice.Properties()

            # Create a property set from command-line arguments.
            properties = Ice.Properties(sys.argv)

            # Create a property set using default values.
            defaultProperties = Ice.Properties()
            defaultProperties.setProperty("Ice.Trace.Protocol", "1")
            properties = Ice.Properties(defaults=defaultProperties)

            # Combine command-line parsing with default values.
            defaultProperties = Ice.Properties()
            defaultProperties.setProperty("Ice.Trace.Protocol", "1")
            properties = Ice.Properties(sys.argv, defaultProperties)
        """
        if isinstance(properties, IcePy.Properties):
            self._impl = properties
        else:
            self._impl = IcePy.createProperties(args, defaults)

    def getProperty(self, key: str) -> str:
        """
        Gets a property by key.

        Parameters
        ----------
        key : str
            The property key.

        Returns
        -------
        str
            The property value, or the empty string if the property is not set.
        """
        return self._impl.getProperty(key)

    def getIceProperty(self, key: str) -> str:
        """
        Gets an Ice property by key.

        Parameters
        ----------
        key : str
            The property key.

        Returns
        -------
        str
            The property value, or the default value for this property if the property is not set.

        Raises
        ------
        PropertyException
            If the property is not a known Ice property.
        """
        return self._impl.getIceProperty(key)

    def getPropertyWithDefault(self, key: str, value: str) -> str:
        """
        Gets a property by key.

        Parameters
        ----------
        key : str
            The property key.
        value : str
            The default value to return if the property is not set.

        Returns
        -------
        str
            The property value or the default value if the property is not set.
        """
        return self._impl.getPropertyWithDefault(key, value)

    def getPropertyAsInt(self, key: str) -> int:
        """
        Gets a property as an integer.

        Parameters
        ----------
        key : str
            The property key.

        Returns
        -------
        int
            The property value interpreted as an integer, or ``0`` if the property is not set.

        Raises
        ------
        PropertyException
            If the property value is not a valid integer.
        """
        return self._impl.getPropertyAsInt(key)

    def getIcePropertyAsInt(self, key: str) -> int:
        """
        Gets an Ice property as an integer.

        Parameters
        ----------
        key : str
            The property key.

        Returns
        -------
        int
            The property value interpreted as an integer, or the default value if the property is not set.

        Raises
        ------
        PropertyException
            If the property is not a known Ice property or the value is not a valid integer.
        """
        return self._impl.getIcePropertyAsInt(key)

    def getPropertyAsIntWithDefault(self, key: str, value: int) -> int:
        """
        Gets a property as an integer.

        Parameters
        ----------
        key : str
            The property key.
        value : int
            The default value to return if the property does not exist.

        Returns
        -------
        int
            The property value interpreted as an integer, or the default value if the property is not set.

        Raises
        ------
        PropertyException
            If the property value is not a valid integer.
        """
        return self._impl.getPropertyAsIntWithDefault(key, value)

    def getPropertyAsList(self, key: str) -> list[str]:
        """
        Gets a property as a list of strings.

        The strings must be separated by whitespace or comma. The strings in the list can contain whitespace and commas
        if they are enclosed in single or double quotes. If quotes are mismatched, an empty list is returned.
        Within single quotes or double quotes, you can escape the quote in question with a backslash,
        e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.

        Parameters
        ----------
        key : str
            The property key.

        Returns
        -------
        list[str]
            The property value interpreted as a list of strings, or an empty list if the property is not set.
        """
        return self._impl.getPropertyAsList(key)

    def getIcePropertyAsList(self, key: str) -> list[str]:
        """
        Gets an Ice property as a list of strings.

        The strings must be separated by whitespace or comma. The strings in the list can contain whitespace and commas
        if they are enclosed in single or double quotes. If quotes are mismatched, the default list is returned.
        Within single quotes or double quotes, you can escape the quote in question with a backslash,
        e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.

        Parameters
        ----------
        key : str
            The property key.

        Returns
        -------
        list[str]
            The property value interpreted as a list of strings, or the default value if the property is not set.

        Raises
        ------
        PropertyException
            If the property is not a known Ice property.
        """
        return self._impl.getIcePropertyAsList(key)

    def getPropertyAsListWithDefault(self, key: str, value: list[str]) -> list[str]:
        """
        Gets a property as a list of strings.

        The strings must be separated by whitespace or comma. The strings in the list can contain whitespace and commas
        if they are enclosed in single or double quotes. If quotes are mismatched, the default list is returned.
        Within single quotes or double quotes, you can escape the quote in question with a backslash,
        e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.

        Parameters
        ----------
        key : str
            The property key.
        value : list[str]
            The default value to use if the property is not set.

        Returns
        -------
        list[str]
            The property value interpreted as a list of strings, or the default value if the property is not set.
        """
        return self._impl.getPropertyAsListWithDefault(key, value)

    def getPropertiesForPrefix(self, prefix: str) -> dict[str, str]:
        """
        Gets all properties whose keys begin with the given prefix.
        If ``prefix`` is the empty string, then all properties are returned.

        Parameters
        ----------
        prefix : str
            The prefix to search for.

        Returns
        -------
        dict[str, str]
            The matching property set.
        """
        return self._impl.getPropertiesForPrefix(prefix)

    def setProperty(self, key: str, value: str) -> None:
        """
        Sets a property. To unset a property, set it to the empty string.

        Parameters
        ----------
        key : str
            The property key.
        value : str
            The property value.
        """
        self._impl.setProperty(key, value)

    def getCommandLineOptions(self) -> list[str]:
        """
        Gets a sequence of command-line options that is equivalent to this property set.
        Each element of the returned sequence is a command-line option of the form ``--key=value``.

        Returns
        -------
        list[str]
            The command line options for this property set.
        """
        return self._impl.getCommandLineOptions()

    def parseCommandLineOptions(self, prefix: str, options: list[str]) -> list[str]:
        """
        Converts a sequence of command-line options into properties.

        All options that start with ``--prefix.`` are converted into properties.
        If the prefix is empty, all options that begin with ``--`` are converted to properties.

        Parameters
        ----------
        prefix : str
            The property prefix, or the empty string to convert all options starting with ``--``.
        options : list[str]
            The command-line options.

        Returns
        -------
        list[str]
            The command-line options that do not start with the specified prefix, in their original order.
        """
        return self._impl.parseCommandLineOptions(prefix, options)

    def parseIceCommandLineOptions(self, options: list[str]) -> list[str]:
        """
        Converts a sequence of command-line options into properties.

        All options that start with one of the reserved Ice prefixes (``--Ice``, ``--IceSSL``, etc.) are converted into
        properties.

        Parameters
        ----------
        options : list[str]
            The command-line options.

        Returns
        -------
        list[str]
            The command-line options that do not start with one of the reserved prefixes, in their original order.
        """
        return self._impl.parseIceCommandLineOptions(options)

    def load(self, file: str) -> None:
        """
        Loads properties from a file.

        Parameters
        ----------
        file : str
            The property file.
        """
        self._impl.load(file)

    def clone(self) -> "Properties":
        """
        Creates a copy of this property set.

        Returns
        -------
        Properties
            A copy of this property set.
        """
        return Properties(properties=self._impl.clone())

    def __iter__(self):
        dict = self._impl.getPropertiesForPrefix("")
        return iter(dict)

    def __str__(self) -> str:
        return str(self._impl)
