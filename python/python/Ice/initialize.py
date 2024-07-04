#
# Ice.initialize()
#

import IcePy
from .CommunicatorI import CommunicatorI

def initialize(args=None, data=None):
    """Initializes a new communicator. The optional arguments represent
    an argument list (such as sys.argv) and an instance of InitializationData.
    You can invoke this function as follows:

    Ice.initialize()
    Ice.initialize(args)
    Ice.initialize(data)
    Ice.initialize(args, data)

    If you supply an argument list, the function removes those arguments from
    the list that were recognized by the Ice run time."""
    communicator = IcePy.Communicator(args, data)
    return CommunicatorI(communicator)
