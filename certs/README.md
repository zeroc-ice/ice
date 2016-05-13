# Ice Test Suite Certificates

This directory contains certificates used by the clients and servers in Ice test
suite. These certificates are for testing puproses only and should **never** be
used in a production environment.

As provided, the server certificates use `localhost` for the Common Name and for
the DNS name, and 127.0.0.1 as the IP address. This works fine when you run the
client and server on the same host. However, if you want to run them on separate
hosts, you may need to regenerate the certificates (This is especially true for
the JavaScript).

We've included the Python script `makecerts.py` to simplify this task.

## Prerequisites

You'll need Python to run the script. The script also depends on a utility package
from a separate [ZeroC repository][1]. You can install this package as follows:

    pip install zeroc-icecertutils

## Usage

Running the script with `-h` displays the following usage information:

```
Usage: certs/makecerts.py [options]

Options:
-h               Show this message.
-d | --debug     Debugging output.
--ip <ip>        The IP address for the server certificate.
--dns <dns>      The DNS name for the server certificate.
--use-dns        Use the DNS name for the server certificate common
                 name (default is to use the IP address).
```

The `--ip`, `--dns`, and `--use-dns` options affect the generation of the server
certificate. Without any arguments, the script uses `localhost` as the server's
DNS name, 127.0.0.1 as the server's IP address, and 127.0.0.1 as the server's
Common Name.

You can specify an alternate IP address using `--ip` and an alternate DNS name
using `--dns`. The `--use-dns` flag forces the script to use the DNS name as
the server's Common Name instead of the IP address.

[1]: https://github.com/zeroc-ice/icecertutils
