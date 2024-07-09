# Copyright (c) ZeroC, Inc. All rights reserved.

from setuptools import setup, Extension, Command
from setuptools.command.build_ext import build_ext as _build_ext
from setuptools.command.sdist import sdist as _sdist
import urllib.request
import os
import shutil
import sys
import pathlib

# Define versions and URLs for dependencies
mcpp_version = "2.7.2.18"
mcpp_url = f"https://github.com/zeroc-ice/mcpp/archive/refs/tags/v{mcpp_version}.tar.gz"
mcpp_local_filename = f"dist/mcpp-{mcpp_version}.tar.gz"

bzip2_version = "1.0.6.6"
bzip2_url = f"https://github.com/zeroc-ice/bzip2/archive/refs/tags/v{bzip2_version}.tar.gz"
bzip2_local_filename = f"dist/bzip2-{bzip2_version}.tar.gz"

script_directory = os.path.dirname(os.path.abspath(__file__))

# Used by the Windows build
platform = os.getenv('CPP_PLATFORM', "x64")
configuration = os.getenv('CPP_CONFIGURATION', "Release")

# Define Python packages to be included
packages = [
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

# Define source directories for Ice C++
ice_cpp_sources = [
    "../cpp/src/Ice",
    "../cpp/src/IceDiscovery",
    "../cpp/src/IceLocatorDiscovery",
    "../cpp/src/slice2py",
    "../cpp/src/Slice",
    "../cpp/include/Ice"]

# Include directories for the build process
include_dirs = [
    'dist/python/modules/IcePy',
    'dist/ice/cpp/include',
    'dist/ice/cpp/src',
    'dist/mcpp']

# Platform-specific adjustments
if sys.platform == 'win32':
    ice_cpp_sources.append(f"../cpp/include/generated/{platform}/{configuration}/Ice")
    include_dirs.extend([
        f"dist/ice/cpp/include/generated/{platform}/{configuration}",
        f"dist/ice/cpp/src/IceDiscovery/msbuild/icediscovery/{platform}/{configuration}",
        f"dist/ice/cpp/src/IceLocatorDiscovery/msbuild/icelocatordiscovery/{platform}/{configuration}",
        f"dist/bzip2-{bzip2_version}"])
else:
    ice_cpp_sources.append("../cpp/include/generated/Ice")
    include_dirs.extend([
        "dist/ice/cpp/include/generated",
        "dist/ice/cpp/src/IceDiscovery/generated",
        "dist/ice/cpp/src/IceLocatorDiscovery/generated"])

# Define macros used during the build process
define_macros = [
    ('ICE_STATIC_LIBS', None),
    ('ICE_PYPI', None),
    ('ICE_BUILDING_SRC', None),
    ('ICE_BUILDING_ICE', None),
    ('ICE_BUILDING_ICE_LOCATOR_DISCOVERY', None)]

# Platform-specific compile and link arguments
if sys.platform == 'darwin':
    extra_compile_args = ['-w']
    cpp_extra_compile_args = ['-std=c++20']
    libraries = ['iconv']
    extra_link_args = ['-framework', 'Security', '-framework', 'CoreFoundation']
elif sys.platform == 'win32':
    define_macros.extend([('WIN32_LEAN_AND_MEAN', None), ('_WIN32_WINNT', '0x0A00'), ('BZ_EXPORT', None)])
    extra_compile_args = ['/std:c++20', '/EHsc', '/Zi']
    extra_compile_args.extend(['/wd4018', '/wd4146', '/wd4244', '/wd4250', '/wd4251', '/wd4267', '/wd4275', '/wd4996'])
    extra_link_args = ['/DEBUG:FULL']
    libraries = ['dbghelp', 'Shlwapi', 'rpcrt4', 'advapi32', 'Iphlpapi', 'secur32', 'crypt32', 'ws2_32']
else:
    extra_compile_args = ['-w']
    cpp_extra_compile_args = ['-std=c++17']
    extra_link_args = []
    libraries = ['ssl', 'crypto', 'bz2', 'rt']
    if not sys.platform.startswith('freebsd'):
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
            filename = os.path.normpath(filename)
            # For Windows, ensure we only include generated files for the current platform
            if pathlib.Path(filename).suffix.lower() not in ['.c', '.cpp', '.h']:
                return False
            if sys.platform == 'win32':
                if "generated" in filename and not os.path.join(platform, configuration) in filename:
                    return False
                elif "msbuild" in filename and not os.path.join(platform, configuration) in filename:
                    return False
            return True

        # MCPP sources
        if not os.path.exists(mcpp_local_filename):
            with urllib.request.urlopen(mcpp_url) as response:
                with open(mcpp_local_filename, 'wb') as f:
                    f.write(response.read())
            os.system(f"tar -xzf {mcpp_local_filename} -C dist")

        for root, dirs, files in os.walk(f"dist/mcpp-{mcpp_version}"):
            for file in files:
                if include_file(os.path.join(root, file)):
                    sources.append(os.path.join(root, file))

        # Bzip2 sources
        if not os.path.exists(bzip2_local_filename):
            with urllib.request.urlopen(bzip2_url) as response:
                with open(bzip2_local_filename, 'wb') as f:
                    f.write(response.read())
            os.system(f"tar -xzf {bzip2_local_filename} -C dist")

        for root, dirs, files in os.walk(f"dist/bzip2-{bzip2_version}"):
            for file in files:
                if include_file(os.path.join(root, file)):
                    sources.append(os.path.join(root, file))

        # Ice sources
        if sys.platform == 'win32':
            os.system(f"MSBuild msbuild/ice.proj /p:Configuration={configuration} /p:Platform={platform} /t:BuildDist")
        else:
            os.system('make srcs OPTIMIZE=yes -C ' + os.path.join(script_directory, '..', 'cpp'))

        for source_dir in ice_cpp_sources:
            for root, dirs, files in os.walk(source_dir):
                for file in files:
                    if include_file(os.path.join(root, file)):
                        source_file = os.path.join(root, file)
                        relative_path = os.path.relpath(source_file, '..')
                        target_file = os.path.join(script_directory, 'dist/ice', relative_path)
                        target_dir = os.path.dirname(target_file)
                        if not os.path.exists(target_dir):
                            os.makedirs(target_dir)
                        shutil.copy(source_file, target_file)
                        sources.append(os.path.relpath(target_file, script_directory))

        # IcePy sources
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

        # Slice sources
        for root, dirs, files in os.walk("../slice"):
            for file in files:
                if file.endswith('.ice') and "IceDiscovery" not in root and "IceLocatorDiscovery" not in root:
                    source_file = os.path.join(root, file)
                    relative_path = os.path.relpath(source_file, '..')
                    target_file = os.path.join(script_directory, 'dist/lib', relative_path)
                    target_dir = os.path.dirname(target_file)
                    if not os.path.exists(target_dir):
                        os.makedirs(target_dir)
                    shutil.copy(source_file, target_file)
                    sources.append(os.path.relpath(target_file, script_directory))

        # Python sources
        if sys.platform == 'win32':
            os.system(f"MSbuild msbuild/ice.proj /p:Configuration={configuration} /p:Platform={platform} /t:BuildDist")
        else:
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
    # Filter out sources that are not needed for building the extension depending on the target platform.
    if "ios/" in filename:
        return False
    
    # Bzip2lib sources
    bzip2sources = ["blocksort.c", "bzlib.c", "compress.c", "crctable.c", "decompress.c", "huffman.c", "randtable.c"]
    if "bzip2-" in filename and os.path.basename(filename) not in bzip2sources:
        return False

    if sys.platform == 'win32':
        for exclude in ["SysLoggerI", "OpenSSL", "SecureTransport"]:
            if exclude in filename:
                # Skip SysLoggerI, OpenSSL and SecureTransport on Windows
                return False
    elif sys.platform == 'darwin':
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

# Customize the build_ext command to filter sources and add extra compile args for C++ files
class CustomBuildExtCommand(_build_ext):

    def run(self):
        sources = []
        for root, dirs, files in os.walk("dist"):
            for file in files:
                if pathlib.Path(file).suffix.lower() in ['.c', '.cpp']:
                    sources.append(os.path.join(root, file))

        filtered = list(filter(filter_source, sources))
        self.distribution.ext_modules[0].sources = filtered
        _build_ext.run(self)

    def build_extension(self, ext):
        original_compile = self.compiler.compile
        
        # Monkey-patch the compiler to add extra compile args for C++ files. This works around errors with Clang and
        # GCC as they don't accept --std=c++XX when compiling C files. The MSVC backend doesn't use _compile.
        def _compile(obj, src, ext, cc_args, extra_postargs, pp_opts):
            original_compile(
                obj,
                src,
                ext,
                cc_args,
                extra_postargs + cpp_extra_compile_args if src.endswith('.cpp') else extra_postargs,
                pp_opts)
            sys.exit(0)

        self.compiler._compile = _compile
        try:
            _build_ext.build_extension(self, ext)
        finally:
            self.compiler.compile = original_compile

# Customize the sdist command to ensure that the third-party sources and the generated sources are included in the
# source distribution.
class CustomSdistCommand(_sdist):
    def run(self):
        self.run_command('pre_build')
        self.distribution.ext_modules[0].sources = sources
        _sdist.run(self)

# Initially empty, to be populated by PreBuildCommand
sources = []

# Define the IcePy extension module
ice_py = Extension(
    'IcePy',
    sources=sources,
    include_dirs=include_dirs,
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
    define_macros=define_macros,
    libraries=libraries)

# Setup configuration for the package
setup(
    name='zeroc-ice',
    version='3.8.0a0',
    packages=packages,
    package_dir={'': 'dist/lib'},
    package_data={'': ['*.ice']},
    include_package_data=True,
    ext_modules=[ice_py],
    cmdclass={
        'pre_build': PreBuildCommand,
        'build_ext': CustomBuildExtCommand,
        'sdist': CustomSdistCommand,
    },
)
