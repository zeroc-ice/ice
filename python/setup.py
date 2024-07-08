# Copyright (c) ZeroC, Inc. All rights reserved.

from setuptools import setup, Extension, Command
from setuptools.command.build_ext import build_ext as _build_ext
from setuptools.command.sdist import sdist as _sdist
import urllib.request
import os
import shutil
import sys
import pathlib

platform = sys.platform
if platform[:6] == 'darwin':
    platform = 'darwin'
elif platform[:5] == 'linux':
    platform = 'linux'
elif platform[:7] == 'freebsd':
    platform = 'freebsd'

mcpp_version = "2.7.2.18"
mcpp_url = f"https://github.com/zeroc-ice/mcpp/archive/refs/tags/v{mcpp_version}.tar.gz"
mcpp_local_filename = f"dist/mcpp-{mcpp_version}.tar.gz"

bzip2_version = "1.0.6.6"
bzip2_url = f"https://github.com/zeroc-ice/bzip2/archive/refs/tags/v{bzip2_version}.tar.gz"
bzip2_local_filename = f"dist/bzip2-{bzip2_version}.tar.gz"

script_directory = os.path.dirname(os.path.abspath(__file__))

packages=[
    'Glacier2',
    'Ice',
    'IceBox',
    'IceGrid',
    'IceMX',
    'IceStorm',
    'slice',
    'slice.Glacier2',
    'slice.Ice',
    'slice.IceBox',
    'slice.IceGrid',
    'slice.IceStorm']

ice_cpp_sources = [
    "../cpp/src/Ice",
    "../cpp/src/IceDiscovery",
    "../cpp/src/IceLocatorDiscovery",
    "../cpp/src/slice2py",
    "../cpp/src/Slice",
    "../cpp/include/Ice",
    "../cpp/include/generated/Ice"]

extra_compile_args=[]
include_dirs=[
    'dist/python/modules/IcePy',
    'dist/ice/cpp/include',
    'dist/ice/cpp/include/generated',
    'dist/ice/cpp/src',
    'dist/ice/cpp/src/IceDiscovery/generated',
    'dist/ice/cpp/src/IceLocatorDiscovery/generated',
    'dist/mcpp']

define_macros =[('ICE_STATIC_LIBS', None)]

if platform == 'darwin':
    cpp_extra_compile_args = ['-std=c++20']
    extra_compile_args.append('-w')
    libraries=['iconv']
    extra_link_args = ['-framework','Security', '-framework','CoreFoundation']
elif platform == 'win32':
    extra_link_args = []
    cpp_extra_compile_args = []
    libraries=[]
    define_macros.append(('WIN32_LEAN_AND_MEAN', None))
    define_macros.append(('_WIN32_WINNT', '0x0A00'))
    include_dirs.append('dist/bzip2')
    extra_compile_args.append('/EHsc')
    extra_compile_args.extend(['/wd4018', '/wd4146', '/wd4244','/wd4250', '/wd4251', '/wd4267','/wd4275', '/wd4996'])
    extra_compile_args.append('/Zi')
    extra_link_args.append('/DEBUG:FULL')
    libraries=['dbghelp', 'Shlwapi', 'rpcrt4','advapi32','Iphlpapi','secur32','crypt32','ws2_32']
else:
    cpp_extra_compile_args = ['-std=c++17']
    extra_compile_args.append('-w')
    extra_link_args = []
    libraries=['ssl', 'crypto', 'bz2', 'rt']
    if platform != 'freebsd':
        libraries.append('dl')


class PreBuildCommand(Command):
    """A custom command to run pre-build steps"""
    description = 'run pre-build commands'
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        global sources  # Use the global sources list

        if not os.path.exists('dist'):
            os.mkdir('dist')

        sources = []  # Clear the sources list

        def include_file(filename):
            return pathlib.Path(filename).suffix.lower() in ['.c', '.cpp', '.h']

        # MCPP sources
        if not os.path.exists(mcpp_local_filename):
            with urllib.request.urlopen(mcpp_url) as response:
                with open(mcpp_local_filename, 'wb') as f:
                    f.write(response.read())
            os.system(f"tar -xzf {mcpp_local_filename} -C dist")

        for root, dirs, files in os.walk(f"dist/mcpp-{mcpp_version}"):
            sources.extend(os.path.join(root, file) for file in filter(include_file, files))

        # Bzip2 sources
        if not os.path.exists(bzip2_local_filename):
            with urllib.request.urlopen(bzip2_url) as response:
                with open(bzip2_local_filename, 'wb') as f:
                    f.write(response.read())
            os.system(f"tar -xzf {bzip2_local_filename} -C dist")

        for root, dirs, files in os.walk(f"dist/bzip2-{bzip2_version}"):
            sources.extend(os.path.join(root, file) for file in filter(include_file, files))

        # Copy Ice sources to the dist directory
        os.system('make srcs OPTIMIZE=yes -C ' + os.path.join(script_directory, '..', 'cpp'))
        for source_dir in ice_cpp_sources:
            for root, dirs, files in os.walk(source_dir):
                for file in files:
                    if include_file(file):
                        source_file = os.path.join(root, file)
                        relative_path = os.path.relpath(source_file, '..')
                        target_file = os.path.join(script_directory, 'dist/ice', relative_path)
                        target_dir = os.path.dirname(target_file)
                        if not os.path.exists(target_dir):
                            os.makedirs(target_dir)
                        shutil.copy(source_file, target_file)
                        sources.append(os.path.relpath(target_file, script_directory))

        # Copy IcePy sources to the dist directory
        for root, dirs, files in os.walk("modules/IcePy"):
            for file in files:
                if file.endswith('.cpp') or file.endswith('.h'):
                    source_file = os.path.join(root, file)
                    relative_path = os.path.relpath(source_file, '.')
                    target_file = os.path.join(script_directory, 'dist/ice/python', relative_path)
                    target_dir = os.path.dirname(target_file)
                    if not os.path.exists(target_dir):
                        os.makedirs(target_dir)
                    shutil.copy(source_file, target_file)
                    sources.append(os.path.relpath(target_file, script_directory))

        # Copy Slice sources to the dist directory
        for root, dirs, files in os.walk("../slice"):
            for file in files:
                if file.endswith('.ice'):
                    source_file = os.path.join(root, file)
                    relative_path = os.path.relpath(source_file, '..')
                    target_file = os.path.join(script_directory, 'dist/lib', relative_path)
                    target_dir = os.path.dirname(target_file)
                    if not os.path.exists(target_dir):
                        os.makedirs(target_dir)
                    shutil.copy(source_file, target_file)
                    sources.append(os.path.relpath(target_file, script_directory))

        # Copy Python sources to the dist directory
        os.system('make OPTIMIZE=yes')
        for root, dirs, files in os.walk("python"):
            for file in files:
                if file.endswith('.py'):
                    source_file = os.path.join(root, file)
                    relative_path = os.path.relpath(source_file, 'python')
                    target_file = os.path.join(script_directory, 'dist/lib', relative_path)
                    target_dir = os.path.dirname(target_file)
                    if not os.path.exists(target_dir):
                        os.makedirs(target_dir)
                    shutil.copy(source_file, target_file)
                    sources.append(os.path.relpath(target_file, script_directory))


def filter_source(filename):

    if "ios/" in filename:
        return False

    if filename.endswith(".h"):
        # Skip header files when building the extension
        return False

    if platform == 'win32':
        for exclude in ["SysLoggerI", "OpenSSL", "SecureTransport"]:
            if exclude in filename:
                # Skip SysLoggerI, OpenSSL and SecureTransport on Windows
                return False
    elif platform == 'darwin':
        for exclude in ["DLLMain", "Schannel", "OpenSSL", "bzip2-"]:
            if exclude in filename:
                # Skip Schannel, OpenSSL and bzip2 on macOS
                return False
    else:
        for exclude in ["DLLMain", "Schannel", "SecureTransport", "bzip2-"]:
            if exclude in filename:
                # Skip Schannel, SecureTransport and bzip2 on Linux
                return False

    return True


class CustomBuildExtCommand(_build_ext):
    """A custom command to run pre-build and build_ext"""

    def run(self):
        # Ensure sources are included for build_ext
        sources = []
        for root, dirs, files in os.walk("dist"):
            for file in files:
                if file.endswith('.c') or file.endswith('.cpp'):
                    sources.append(os.path.join(root, file))

        filtered = list(filter(filter_source, sources))
        print("Filtered sources:")
        print(filtered)
        self.distribution.ext_modules[0].sources = filtered
        _build_ext.run(self)

    def build_extension(self, ext):

        original_compile = self.compiler._compile

        def _compile(obj, src, ext, cc_args, extra_postargs, pp_opts):
            original_compile(
                obj,
                src,
                ext,
                cc_args,
                cpp_extra_compile_args if src.endswith('.cpp') else extra_postargs,
                pp_opts)

        self.compiler._compile = _compile
        try:
            _build_ext.build_extension(self, ext)
        finally:
            self.compiler._compile = original_compile


class CustomSdistCommand(_sdist):
    """A custom command to run pre-build and sdist"""

    def run(self):
        # Run the pre-build step to prepare sources
        self.run_command('pre_build')
        self.distribution.ext_modules[0].sources = sources
        # Run the standard sdist
        _sdist.run(self)


# Initially empty, to be populated by PreBuildCommand
sources = []

ice_py = Extension(
    'IcePy',
    sources=sources,
    include_dirs=include_dirs,
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
    define_macros=define_macros,
    libraries=libraries)

setup(
    name='zeroc-ice',
    version='3.8.0a0',
    packages=packages,
    package_dir={'': 'dist/lib'},
    package_data = {'': ['*.ice']},
    include_package_data = True,
    ext_modules=[ice_py],
    cmdclass={
        'pre_build': PreBuildCommand,
        'build_ext': CustomBuildExtCommand,
        'sdist': CustomSdistCommand,
    },
)
