# Copyright (c) ZeroC, Inc.

from typing import final, Self

@final
class Properties:
    """
    A property set used to configure Ice and Ice applications. Properties are key/value pairs, with both keys and
    values being strings. By convention, property keys should have the form
    application-name[.category[.sub-category]].name.
    """

    def __init__(self, impl):
        self._impl = impl

    def getProperty(self, key: str) -> str:
        """
        Get a property by key.

        If the property is not set, an empty string is returned.

        Parameters
        ----------
        key : str
            The property key.

        Returns
        -------
        str
            The property value, or an empty string if the property is not set.
        """
        return self._impl.getProperty(key)

    def getIceProperty(self, key: str) -> str:
        """
        Get an Ice property by key.

        If the property is not set, its default value is returned.

        Parameters
        ----------
        key : str
            The property key.

        Returns
        -------
        str
            The property value, or the default value if the property is not set.

        Raises
        ------
        PropertyException
            When the property is not a known Ice property.
        """
        return self._impl.getIceProperty(key)

    def getPropertyWithDefault(self, key: str, value: str) -> str:
        """
        Get a property by key.

        If the property is not set, the given default value is returned.

        Parameters
        ----------
        key : str
            The property key.
        value : str
            The default value to use if the property does not exist.

        Returns
        -------
        str
            The property value, or the default value if the property does not exist.
        """
        return self._impl.getPropertyWithDefault(key, value)

    def getPropertyAsInt(self, key: str) -> int:
        """
        Get a property as an integer.

        If the property is not set, 0 is returned.

        Parameters
        ----------
        key : str
            The property key.

        Returns
        -------
        int
            The property value interpreted as an integer, or 0 if the property is not set.

        Raises
        ------
        PropertyException
            When the property value is not a valid integer.
        """
        return self._impl.getPropertyAsInt(key)

    def getIcePropertyAsInt(self, key: str) -> int:
        """
        Get an Ice property as an integer.

        If the property is not set, its default value is returned.

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
            When the property is not a known Ice property or the property value is not a valid integer.
        """
        return self._impl.getIcePropertyAsInt(key)

    def getPropertyAsIntWithDefault(self, key: str, value: int) -> int:
        """
        Get a property as an integer.

        If the property is not set, the given default value is returned.

        Parameters
        ----------
        key : str
            The property key.
        value : int
            The default value to use if the property does not exist.

        Returns
        -------
        int
            The property value interpreted as an integer, or the default value if the property does not exist.

        Raises
        ------
        PropertyException
            When the property value is not a valid integer.
        """
        return self._impl.getPropertyAsIntWithDefault(key, value)

    def getPropertyAsList(self, key: str) -> list[str]:
        """
        Get a property as a list of strings.

        The strings must be separated by whitespace or comma. If the property is not set, an empty list is returned.
        The strings in the list can contain whitespace and commas if they are enclosed in single or double quotes.
        If quotes are mismatched, an empty list is returned. Within single or double quotes, you can escape the quote
        in question with a backslash, e.g., O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.

        Parameters
        ----------
        key : str
            The property key.

        Returns
        -------
        list of str
            The property value interpreted as a list of strings.
        """
        return self._impl.getPropertyAsList(key)

    def getIcePropertyAsList(self, key: str) -> list[str]:
        """
        Get an Ice property as a list of strings.

        The strings must be separated by whitespace or comma. If the property is not set, its default value is returned.
        The strings in the list can contain whitespace and commas if they are enclosed in single or double quotes.
        If quotes are mismatched, the default value is returned. Within single or double quotes, you can escape the quote
        in question with a backslash, e.g., O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.

        Parameters
        ----------
        key : str
            The property key.

        Returns
        -------
        list of str
            The property value interpreted as a list of strings, or the default value if the property is not set.

        Raises
        ------
        PropertyException
            When the property is not a known Ice property.
        """
        return self._impl.getIcePropertyAsList(key)

    def getPropertyAsListWithDefault(self, key: str, value: list[str]) -> list[str]:
        """
        Get a property as a list of strings.

        The strings must be separated by whitespace or comma. If the property is not set, the default value is returned.
        The strings in the list can contain whitespace and commas if they are enclosed in single or double quotes.
        If quotes are mismatched, the default value is returned. Within single or double quotes, you can escape the quote
        in question with a backslash, e.g., O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.

        Parameters
        ----------
        key : str
            The property key.
        value : list of str
            The default value to use if the property is not set.

        Returns
        -------
        list of str
            The property value interpreted as a list of strings, or the default value if the property is not set.
        """
        return self._impl.getPropertyAsListWithDefault(key, value)

    def getPropertiesForPrefix(self, prefix: str) -> dict[str, str]:
        """
        Get all properties whose keys begin with the given prefix.

        If the prefix is an empty string, then all properties are returned.

        Parameters
        ----------
        prefix : str
            The prefix to search for (empty string if none).

        Returns
        -------
        dict[str, str]
            The matching property set with keys and values as strings.
        """
        return self._impl.getPropertiesForPrefix(prefix)

    def setProperty(self, key: str, value: str) -> None:
        """
        Set a property.

        To unset a property, set it to the empty string.

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
        Get a sequence of command-line options that is equivalent to this property set.

        Each element of the returned sequence is a command-line option of the form --key=value.

        Returns
        -------
        list of str
            The command line options for this property set.
        """
        return self._impl.getCommandLineOptions()

    def parseCommandLineOptions(self, prefix: str, options: list[str]) -> list[str]:
        """
        Convert a sequence of command-line options into properties.

        All options that begin with `--prefix.` are converted into properties. If the prefix is empty, all options that
        begin with `--` are converted to properties.

        Parameters
        ----------
        prefix : str
            The property prefix, or an empty string to convert all options starting with `--`.
        options : list of str
            The command-line options.

        Returns
        -------
        list of str
            The command-line options that do not start with the specified prefix, in their original order.
        """
        return self._impl.parseCommandLineOptions(prefix, options)

    def parseIceCommandLineOptions(self, options: list[str]) -> list[str]:
        """
        Convert a sequence of command-line options into properties.

        All options that begin with one of the following prefixes are converted into properties: `--Ice`, `--IceBox`,
        `--IceGrid`, `--IceSSL`, `--IceStorm`, and `--Glacier2`.

        Parameters
        ----------
        options : list of str
            The command-line options.

        Returns
        -------
        list of str
            The command-line options that do not start with one of the listed prefixes, in their original order.
        """
        return self._impl.parseIceCommandLineOptions(options)

    def load(self, file: str) -> None:
        """
        Load properties from a file.

        Parameters
        ----------
        file : str
            The property file.
        """
        self._impl.load(file)

    def clone(self) -> Self:
        """
        Create a copy of this property set.

        Returns
        -------
        Properties
            A copy of this property set.
        """
        return Properties(self._impl.clone())

    def __iter__(self):
        dict = self._impl.getPropertiesForPrefix("")
        return iter(dict)

    def __str__(self) -> str:
        return str(self._impl)
