import sys, os

from distutils.core import setup, Extension

if not os.environ.has_key('ICE_HOME'):
    print "ICE_HOME is not defined"
    sys.exit(1)

libs = ['Slice', 'Ice', 'IceUtil']

if sys.platform == 'sunos5':
    libs.append('Cstd')
    libs.append('Crun')

IcePy = Extension(
                'IcePy',
                sources = [
                    'Communicator.cpp',
                    'Current.cpp',
                    'Init.cpp',
                    'Logger.cpp',
                    'ObjectAdapter.cpp',
                    'ObjectFactory.cpp',
                    'Operation.cpp',
                    'Properties.cpp',
                    'Proxy.cpp',
                    'Slice.cpp',
                    'Types.cpp',
                    'Util.cpp'
                ],
                include_dirs = [os.environ['ICE_HOME'] + '/include', '.'],
                library_dirs = [os.environ['ICE_HOME'] + '/lib'],
                libraries = libs)

setup (name = 'IcePy', version = '2.1', description = 'Ice', ext_modules = [IcePy])
