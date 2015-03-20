# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

# Always prefer setuptools over distutils
try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

from distutils.extension import Extension
import distutils.sysconfig
import sys, os, shutil, fnmatch, glob

#
# Sort out packages, package_dir and package_data from the lib dir.
#
packages = [ '' ]
package_dir={'' : 'lib'}
for f in os.listdir('lib'):
    p = os.path.join('lib', f)
    if os.path.isdir(p):
        package_dir[f] = p
        packages.append(f)
package_data = { 'slice' : ['*/*.ice'] }

include_dirs=['src', 'src/ice/cpp/include', 'src/ice/cpp/src']
define_macros=[('HAVE_CONFIG_H', 1), ('MCPP_LIB', 1), ('ICE_STATIC_LIBS', None)]
extra_compile_args=[]

platform = sys.platform
if platform[:6] == 'darwin':
    platform = 'darwin'

if platform == 'darwin':
    if not 'ARCHFLAGS' in os.environ:
        os.environ['ARCHFLAGS'] = '-arch x86_64'
    extra_compile_args.append('-w')
    extra_link_args = ['-framework','Security', '-framework','CoreFoundation']
    libraries=['iconv']
    shutil.copyfile('src/ice/mcpp/config.h.Darwin', 'src/ice/mcpp/config.h')
    # Don't compile the bzip2 source under darwin or linux.
    def filterName(path):
        d = os.path.dirname(path)
        if d.find('bzip2') != -1:
            return False
        return True

elif platform == 'linux2':

    extra_compile_args.append('-w')
    extra_link_args = []
    libraries=['ssl', 'crypto', 'bz2']
    shutil.copyfile('src/ice/mcpp/config.h.Linux', 'src/ice/mcpp/config.h')
    # Don't compile the bzip2 source under darwin or linux.
    def filterName(path):
        d = os.path.dirname(path)
        if d.find('bzip2') != -1:
            return False
        return True

elif platform == 'win32':
    extra_link_args = []
    libraries=[]
    define_macros.append(('WIN32_LEAN_AND_MEAN', None))
    define_macros.append(('ICE_NO_PRAGMA_COMMENT', None))
    include_dirs.append('src/ice/bzip2')
    extra_compile_args.append('/EHsc')
    libraries=['rpcrt4','advapi32','Iphlpapi','secur32','crypt32','ws2_32']
    shutil.copyfile('src/ice/mcpp/config.h.win32', 'src/ice/mcpp/config.h')
    # SysLoggerI.cpp shouldn't be built under Windows.
    def filterName(path):
        b = os.path.basename(path)
        if b == 'SysLoggerI.cpp':
            return False
        return True

# Gather the list of sources to compile.
sources = []
for root, dirnames, filenames in os.walk('src'):
  for filename in fnmatch.filter(filenames, '*.cpp'):
        n = os.path.join(root, filename)
        if filterName(n):
            sources.append(n)
  for filename in fnmatch.filter(filenames, '*.c'):
        n = os.path.join(root, filename)
        if filterName(n):
            sources.append(n)

with open('README.rst') as file:
    long_description = file.read()

setup(
    name='zeroc-ice',

    version='3.6b',

    description="Ice is a modern object-oriented toolkit that enables you to build distributed applications with minimal effort.",

    long_description=long_description,

    # The project's main homepage.
    url='https://www.zeroc.com',

    # Author details
    author='ZeroC, Inc.',
    author_email='info@zeroc.com',

    # Choose your license
    license='GPL v2 with exceptions',

    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
        # How mature is this project? Common values are
        #   3 - Alpha
        #   4 - Beta
        #   5 - Production/Stable
        'Development Status :: 4 - Beta',

        # Indicate who your project is intended for
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Build Tools',

        # Pick your license as you wish (should match "license" above)
        'License :: OSI Approved :: GNU General Public License v2 (GPLv2)',

        # Specify the Python versions you support here. In particular, ensure
        # that you indicate whether you support Python 2, Python 3 or both.
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.0',
        'Programming Language :: Python :: 3.1',
        'Programming Language :: Python :: 3.2',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
    ],

    # What does your project relate to?
    keywords='distributed systems development',

    packages = packages,
    package_dir = package_dir,
    package_data = package_data,

    entry_points = {
        'console_scripts': ['slice2py=slice2py:main'],
    },

    ext_modules=[
        Extension('IcePy', sources,
          extra_link_args=extra_link_args,
          define_macros=define_macros,
          include_dirs=include_dirs,
          extra_compile_args=extra_compile_args,
          libraries=libraries)
        ]
)
