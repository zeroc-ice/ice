# Building Ice for JavaScript

This file describes how to build and install Ice for JavaScript from source
code. If you prefer, you can also download [binary distributions][1] for the
supported platforms.

<!-- TOC depthFrom:2 -->

- [JavaScript Build Requirements](#javascript-build-requirements)
- [Building the JavaScript libraries and NodeJS packages](#building-the-javascript-libraries-and-nodejs-packages)
- [Running the JavaScript Tests](#running-the-javascript-tests)
    - [Browser Information](#browser-information)
        - [Self-Signed Certificate](#self-signed-certificate)
        - [Windows 8](#windows-8)
        - [Secure WebSockets on iOS and Android](#secure-websockets-on-ios-and-android)
            - [Installing Certificates on iOS](#installing-certificates-on-ios)
            - [Installing Certificates on Android](#installing-certificates-on-android)
- [Installing a Source Build](#installing-a-source-build)

<!-- /TOC -->

## JavaScript Build Requirements

To build Ice for JavaScript you must have the following:

- The `slice2js` compiler from Ice for C++. If you have not built Ice for C++
  in this source distribution, refer to the
  [C++ build instructions](../cpp/README.md).
- Node.js 4.6 or later

## Building the JavaScript libraries and NodeJS packages

Change to the Ice for JavaScript source subdirectory:
```
cd js
```

Run these commands to build the libraries and tests:
```
npm install
npm run build
```

On Windows, you need to set the platform and configuration in order to locate
`slice2js`. For example, if you have built C++ with the x64 Release
configuration, you can use the following command to build JavaScript:
```
npm run build -- --cppPlatform x64 --cppConfiguration Release
```

Alternatively you can use the `CPP_PLATFORM` and `CPP_CONFIGURATION` environment
variables:
```
set CPP_PLATFORM=x64
set CPP_CONFIGURATION=Debug
npm run build
```

Upon successful completion, the build generates libraries in the `lib`
subdirectory, including compressed and minified versions.

For older browsers that do not support all of the required ECMAScript 6
features used by Ice for JavaScript, we provide pre-compiled versions of
the libraries using the [Babel][2] JavaScript compiler. These libraries
depend on the `core-js`[3] and `regenerator-runtime`[4] packages
and  are available in the `lib/es5` subdirectory with the same names as
the main libraries.

## Running the JavaScript Tests

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:
```
pip install passlib
```

The scripts also require Ice for Python, you can build Ice for Python from
[python](../python) folder of this source distribution or install the Python
module `zeroc-ice`, using the following command:
```
pip install zeroc-ice
```

The test suites require that the Ice for C++ tests be built in the `cpp`
subdirectory of this source distribution.

You can start the NodeJS tests with:
```
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

You can start the browser tests with:
```
python allTests.py --browser Manual
```

Then open the test page (http://127.0.0.1:8080/start) using a web browser.
These tests require a web browser with ECMAScript 6 support, such as
a recent version of Chrome, Firefox, Microsoft Edge or Safari.

If you are using another web browser, such as Internet Explorer, you should
use instead:
```
python allTests.py --browser Manual --es5
```

Then open the test page (http://127.0.0.1:8080/es5/start)

This runs a version of the test suite transpiled to ECMAScript 5 using [Babel][2].

On macOS the first time you run the tests, you will be prompted for your
password. This is necessary to configure the trust setting for the HTTP
server certificate, which will enable you to connect to the HTTP server
with SSL via your web browser.

### Browser Information

#### Self-Signed Certificate

The browser-based tests allow you to choose whether to run the tests over
non-secure WebSocket (WS) or secure WebSocket (WSS) connections. The WSS
connections in these tests rely on a self-signed certificate, `cacert`,
provided in the certs(../certs) directory.

To successfully run the tests over WSS, additional action may be necessary
depending on the browser you're using:

- Chrome
   You'll be presented with a warning about the site's security certificate.
   Click the "Proceed anyway" button to temporarily accept the certificate.

- Firefox
   You'll see a warning saying "This Connection is Untrusted". Open Firefox's
   Preferences or Options dialog, click on the Advanced section, select the
   Certificates tab and click on the "View Certificates..." button. In the
   Authorities tab, click the "Import..." button, navigate to the `cacert.pem`
   file, and add it as a certificate authority (CA) for trusting web sites.
   After closing the dialogs, reload the test page to continue. You should
   uninstall this certificate after running the tests.

- Internet Explorer and Microsoft Edge
   Run the management console (mmc.exe) and add the Certificates snap-in for
   the computer account. Then select Console Root > Certificates (Local
   Computer) > Trusted Root Certificate Authorities. In the Action menu, choose
   All Tasks and Import. Navigate to the `cacert.der` file and import it into
   the Trusted Root Certificate Authorities. Reload the test page to continue.
   You should uninstall this certificate after running the tests.

#### Windows 8

On Windows 8, network isolation prevents Internet Explorer from
connecting to 127.0.0.1. To work around this limitation, you'll need to disable
Internet Explorer's "Protected Mode". Open the "Internet Options" dialog and in
the "Security" settings tab, deselect the "Enable Protected Mode" checkbox.

#### Secure WebSockets on iOS and Android

To use WSS on iOS and Android it may be required (depending on browser and
platform) that the server certificate's common name matches the computer hosting
the tests, and that the test certificate authority be installed on your device.

First you'll need to generate new certificates to match the IP address of the
computer hosting the tests:
```
certs/makecerts.py [IP address]
```

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

After a successful build, you can generate a package by running the
following command:
```
npm pack
```

This will generate the file `ice-3.7.9.tgz`, which can be installed by running:
```
npm install ice-3.7.9.tgz
```

To use Ice for JavaScript with a browser, copy the appropriate JavaScript
library files located in the `lib` directory to your web server.

[1]: https://zeroc.com/downloads/ice
[2]: https://babeljs.io
[3]: https://www.npmjs.com/package/core-js
[4]: https://www.npmjs.com/package/regenerator-runtime
