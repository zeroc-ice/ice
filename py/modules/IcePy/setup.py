import os

from distutils.core import setup, Extension

IcePy = Extension(
                'IcePy',
                sources = [
                    'Communicator.cpp',
                    'Current.cpp',
                    'Identity.cpp',
                    'Init.cpp',
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
                libraries = ['Slice', 'Ice', 'IceUtil'])

setup (name = 'IcePy', version = '1.6', description = 'Ice', ext_modules = [IcePy])
