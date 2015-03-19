The Ice Performance Test Suite (IcePERF)
========================================

The Ice Performance Test Suite provides performance evaluation tools,
configurations and build systems for multiple middleware platforms.
Currently supported middleware products are: Ice, IceE and TAO (CORBA).
IcePERF is also currently only supported on the Linux, Windows 2000 and
XP family operating systems.

The iceperf tools and configurations strive to provide the closest
common configuration so the results produced are 'fair' and as directly
comparable as possible.

Building IcePERF 
----------------

Basic configuration of your build environment is minimal. IcePERF will
build against Ice, IceE and TAO installations or built source archives.
To configure your environment:
 - set the ICE_HOME environment variable to refer to the path of the Ice
   distribution you want to test. 
 - set the ICEE_HOME environment variable to refer to the path of the
   IceE distribution you want to test. 
 - set the TAO_ROOT and ACE_ROOT environment variables to refer to the
   path of the TAO distributions you want to test. 

For Windows users only:

   The Windows build system for the IceE translator does not install the
   translator into a specific location, nor is there a environment
   varialble that can be used to identify its location. In order to
   build the IceE portion of the performance tests you will need to
   either:

     - add the location of the translators to your PATH environment
       variable if you are using the /useenv option, or

     - add the location of the translators to the list of executable
       directories in the Microsoft IDE Tools|Options|Directories
       settings.

ICEE_HOME, TAO_ROOT and ACE_ROOT are optional settings and are only
required if you want to build and run the IceE and TAO performance
tests. Ice and ICE_HOME is required for the TAO tests as the TAO
performance tests use some Ice APIs in their implementation. 

Note that if you have a pre-existing working environment configured for
using Ice and TAO, chances are you already have these variables
configured.

Once your environment's are configured, simply type gmake from the
IcePERF directory.

Running IcePERF
---------------

IcePERF contains two different sets of performance tests, the basic
latency and throughput tests that test the middleware core for each ORB
and the event service performance tests that evaluate the performance of
the IceStorm event distribution service and TAO's OMG Event Service. 

While each test can be run manually, it is recommended that you use the
runIcePerf.py or runIceStormPerf.py Python scripts. These scripts run
all of the tests automatically, gathering and summarizing the results.
The scripts use the environment variables ICE_HOME, ICEE_HOME and
TAO_ROOT to determine which tests should be run. You can also control
the tests that are run with a regular expression provided at the command
line. For example:

#
# To run all ORB tests.
#
python runIcePerf.py.

#
# To run all of the event tests
#
python runIceStormPerf.py

#
# To run all latency tests.
#
python runIcePerf.py .*latency.*

#
# To run only the the Ice event tests.
#
python runIceStormPerf.py ^Ice.*

The test scripts also allow you to run the performance tests multiple
times. The results from the multiple runs are merged together, analyze
and summarized at the end of the script. Use the '--iter' command line
option to control the number of runs per test:

#
# Run each ORB test 5 times.
# 
python runIcePerf.py --iter 5
