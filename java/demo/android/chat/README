This directory contains an Eclipse project for the chat client.

The chat client provides Android devices with native access to our
sample chat system. This client demonstrates several useful techniques
that are instructive for Android developers:

 * Issuing asynchronous Ice invocations to maintain a responsive user
   interface
 * Using Glacier2 and bidirectional connections to receive "push
   notifications" from a remote server
 * Proper separation of UI and application logic

By default the client uses "demo.zeroc.com" as its server, which
refers to the public chat server hosted by ZeroC. For more information
about the chat system, or to download the source code for the chat
system so that you can run your own server, please visit the link
below:

  http://www.zeroc.com/chat/index.html

Please refer to the release notes for instructions on configuring
your environment to use Ice in an Android project.


SSL Certificates
----------------

This demo is configured by default to use ZeroC's public chat server.

If you want to test the Android client with a local chat server, you
will need to modify the Glacier2 configuration to use the SSL
certificates that are included in this distribution.

The simplest solution is to update the IceSSL.DefaultDir property in
the Chat Demo configuration file for Glacier2 to point to the certs
directory of this distribution.
