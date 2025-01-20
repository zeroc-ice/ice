#  Copyright (c) ZeroC, Inc.


class ValueFactory(object):
    """
    A factory for values.

    Value factories are used by Ice when unmarshaling class instances. Value factories must be
    implemented by the application writer and registered with the communicator.
    """

    def __init__(self):
        if type(self) is ValueFactory:
            raise RuntimeError("Ice.ValueFactory is an abstract class")

    def create(self, typeId):
        """
        Create a new value for a given value type.

        The type is the absolute Slice type ID, i.e., the ID relative to the unnamed top-level Slice module.
        For example, the absolute Slice type ID for an interface `Bar` in the module `Foo` is "::Foo::Bar".
        Note that the leading "::" is required.

        Parameters
        ----------
        typeId : str
            The Slice type ID.

        Returns
        -------
        Ice.Value or None
            The value created for the given type, or None if the factory is unable to create the value.
        """
        raise NotImplementedError("method 'create' not implemented")

    def __str__(self):
        return "::Ice::ValueFactory"

    __module__ = "Ice"
