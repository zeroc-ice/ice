# Building Ice for JavaScript

This page describes how to build and install Ice for JavaScript from source
code. If you prefer, you can also download [binary distributions][1] for the
supported platforms.

## JavaScript Build Requirements

To build Ice for JavaScript you must have the following:

- Ice 3.6 development kit for C++
- Node.js 0.10 or later

## Building the JavaScript libraries and NodeJS packages

If you have not built Ice for C++ in the `cpp` subdirectory, set `ICE_HOME` to
the directory of your Ice for C++ installation. For example, on Unix:

    $ export ICE_HOME=/opt/Ice

On Windows:

    > set ICE_HOME=C:\Ice

Change to the Ice for JavaScript source subdirectory:

    > cd js

Run these commands to build the libraries and tests:

    > npm install
    > npm run gulp:build

## Running the JavaScript Tests

Python is required to run the test suite with Node.js. Additionally, the
Glacier2 tests require the Python module `passlib`, which you can install
with the command:

    $ pip install passlib

To start the tests simply run:

    > npm run gulp:test:run-with-node

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

To start the browser tests run:

    > npm run gulp:test:run-with-browser

This requires that you build the Java test controller from the Java subdirectory
and test servers from C++, C# or Java. Follow the instructions from the
corresponding language mapping to build the tests and the server controller.

In OS X the first time you run the script, you will be prompted for your
password. This is necessary to configure the trust setting for the HTTP
server certificate, which will enable you to connect to the HTTP server
with SSL via your web browser.

The test page (http://127.0.0.1:8080) will be opened using the system default
browser.

### Browser Information

#### Self-Signed Certificate

The browser-based tests allow you to choose whether to run the tests over
non-secure WebSocket (WS) or secure WebSocket (WSS) connections. This
distribution includes a self-signed certificate used for securing WSS
connections, located in `certs/cacert.pem`. If you select WSS, the page
will automatically reload if necessary to connect to the HTTPS port 9090.
To successfully run the tests over WSS, additional action may be necessary
depending on the browser you're using:

- Chrome
   You'll be presented with a warning about the site's security certificate.
   Click the "Proceed anyway" button to temporarily accept the certificate.

- Firefox
   You'll see a warning saying "This Connection is Untrusted". Open Firefox's
   Preferences or Options dialog, click on the Advanced section, select the
   Certificates tab and click on the "View Certificates..." button. In the
   Authorities tab, click the "Import..." button, navigate to the cacert.pem
   file, and add it as a certificate authority (CA) for trusting web sites.
   After closing the dialogs, reload the test page to continue.

- Internet Explorer and Microsoft Edge
   Run the management console (mmc.exe) and add the Certificates snap-in for
   the computer account. In the console window, open the Certificates folder.
   From the Action menu, choose All Tasks and Import. Navigate to the
   cacert.pem file and import it into the Trusted Root Certification Authorities
   store. Reload the test page to continue.

#### Windows 8

On Windows 8 and Windows 8.1, network isolation prevents Internet Explorer from
connecting to 127.0.0.1. To work around this limitation, you'll need to disable
Internet Explorer's "Protected Mode". Open the "Internet Options" dialog and in
the "Security" settings tab, deselect the "Enable Protected Mode" checkbox.

#### Secure WebSockets on iOS and Android

To use WSS on iOS and Android it may be required (depending on browser and
platform) that the server certificate's common name matches the computer hosting
the tests, and that the test certificate authority be installed on your device.

First you'll need to generate new certificates to match the IP address of the
computer hosting the tests:

    > certs/makecerts.py [IP address]

Next you must install the certificate authority on your device. The simplest way
is to email the CA certificate (`certs/cacert.pem`) to yourself and then follow
the instructions below to install the certificate on your device.

Once installed, you connect to the server using the same IP address used to
create the certificates.

##### Installing Certificates on iOS

Open the certificate (`cacert.pem`) from the device's email client. You
will be prompted to create a configuration profile containing this certificate.

##### Installing Certificates on Android

Download the certificate (`cacert.pem`) to the device from an email client.
Next go to _Settings -> Security -> Install from storage_, and choose
`cacert.pem`. Enter a name and press OK.

## Installing a Source Build

After a successful build, you can generate a npm package by running the
following command:

    > npm pack

This will generate the file `ice-3.7a2.tgz`, which can be installed by running:

    > npm install <path_to_file>/ice-3.7a2.tgz

To use Ice for JavaScript with a browser, copy the appropriate JavaScript
library files located in the `lib` directory to your web server.

[1]: https://zeroc.com/download.html
