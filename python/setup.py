# Copyright (c) ZeroC, Inc.

import os
import pathlib
import shutil
import sys
import urllib.request

from setuptools import Extension, find_namespace_packages, setup
from setuptools.command.build_ext import build_ext as _build_ext
from setuptools.command.sdist import sdist as _sdist

# Define versions and URLs for dependencies
mcpp_version = "2.7.2.20"
mcpp_url = f"https://github.com/zeroc-ice/mcpp/archive/refs/tags/v{mcpp_version}.tar.gz"
mcpp_local_filename = f"dist/mcpp-{mcpp_version}.tar.gz"

bzip2_version = "1.0.6.6"
bzip2_url = f"https://github.com/zeroc-ice/bzip2/archive/refs/tags/v{bzip2_version}.tar.gz"
bzip2_local_filename = f"dist/bzip2-{bzip2_version}.tar.gz"

script_directory = os.path.dirname(os.path.abspath(__file__))

# Used by the Windows build
platform = os.getenv("CPP_PLATFORM", "x64")
configuration = os.getenv("CPP_CONFIGURATION", "Release")

# Define source directories for Ice C++
ice_cpp_sources = [
    "../cpp/src/Ice",
    "../cpp/src/IceDiscovery",
    "../cpp/src/IceLocatorDiscovery",
    "../cpp/src/slice2py",
    "../cpp/src/Slice",
    "../cpp/include/Ice",
    "../cpp/include/IceDiscovery",
    "../cpp/include/IceLocatorDiscovery",
]

# Include directories for the build process
include_dirs = [
    "dist/python/modules/IcePy",
    "dist/ice/cpp/include",
    "dist/ice/cpp/include/generated",
    "dist/ice/cpp/src",
    "dist/mcpp",
]

# Platform-specific adjustments
if sys.platform == "win32":
    ice_cpp_sources.append(f"../cpp/include/generated/{platform}/{configuration}/Ice")
    include_dirs.append(f"dist/bzip2-{bzip2_version}")
else:
    ice_cpp_sources.extend(
        [
            "../cpp/include/generated/Ice",
            "../cpp/include/generated/IceDiscovery",
            "../cpp/include/generated/IceLocatorDiscovery",
        ]
    )

# Define macros used during the build process
# All the /**/ macros are necessary only on Windows
define_macros = [
    ("ICE_DISABLE_PRAGMA_COMMENT", None),
    ("ICE_API", "/**/"),
    ("ICE_DISCOVERY_API", "/**/"),
    ("ICE_LOCATOR_DISCOVERY_API", "/**/"),
]

# Platform-specific compile and link arguments
if sys.platform == "darwin":
    extra_compile_args = ["-w"]
    cpp_extra_compile_args = ["-std=c++20"]
    libraries = []
    extra_link_args = ["-framework", "Security", "-framework", "CoreFoundation"]
elif sys.platform == "win32":
    define_macros.extend([("WIN32_LEAN_AND_MEAN", None), ("_WIN32_WINNT", "0x0A00"), ("BZ_EXPORT", None)])
    extra_compile_args = ["/std:c++20", "/EHsc", "/Zi"]
    extra_compile_args.extend(["/wd4018", "/wd4146", "/wd4244", "/wd4250", "/wd4251", "/wd4267", "/wd4275", "/wd4996"])
    extra_link_args = ["/DEBUG:FULL"]
    libraries = ["dbghelp", "Shlwapi", "rpcrt4", "advapi32", "Iphlpapi", "secur32", "crypt32", "ws2_32"]
else:
    extra_compile_args = ["-w"]
    cpp_extra_compile_args = ["-std=c++17"]
    extra_link_args = []
    libraries = ["ssl", "crypto", "bz2", "rt"]
    if not sys.platform.startswith("freebsd"):
        libraries.append("dl")


def filter_source(filename: str):
    # Filter out sources that are not needed for building the extension depending on the target platform.
    if "ios/" in filename:
        return False

    if "AddDefaultPluginFactories_min" in filename:
        return False

    # Bzip2lib sources
    bzip2sources = ["blocksort.c", "bzlib.c", "compress.c", "crctable.c", "decompress.c", "huffman.c", "randtable.c"]
    if "bzip2-" in filename and os.path.basename(filename) not in bzip2sources:
        return False

    if sys.platform == "win32":
        for exclude in ["SysLoggerI", "OpenSSL", "SecureTransport", "Service", "DLLMain"]:
            if exclude in filename:
                # Skip SysLoggerI, OpenSSL and SecureTransport on Windows
                return False
    elif sys.platform == "darwin":
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
        for root, _, files in os.walk("dist"):
            for file in files:
                if pathlib.Path(file).suffix.lower() in [".c", ".cpp"]:
                    sources.append(os.path.join(root, file))

        filtered = list(filter(filter_source, sources))
        self.distribution.ext_modules[0].sources = filtered
        _build_ext.run(self)

    def build_extension(self, ext: Extension):
        original_compile = self.compiler._compile

        # Monkey-patch the compiler to add extra compile args for C++ files. This works around errors with Clang and
        # GCC as they don't accept --std=c++XX when compiling C files. The MSVC backend doesn't use _compile.
        def _compile(
            obj: str, src: str, ext: str | None, cc_args: list[str], extra_postargs: list[str], pp_opts: list[str]
        ):
            original_compile(
                obj,
                src,
                ext,
                cc_args,
                cpp_extra_compile_args + extra_postargs if src.endswith(".cpp") else extra_postargs,
                pp_opts,
            )

        self.compiler._compile = _compile
        try:
            _build_ext.build_extension(self, ext)
        finally:
            self.compiler._compile = original_compile


# Customize the sdist command to ensure that the third-party sources and the generated sources are included in the
# source distribution.
class CustomSdistCommand(_sdist):
    def include_file(self, filename: str):
        filename = os.path.normpath(filename)
        if pathlib.Path(filename).suffix.lower() not in [".c", ".cpp", ".h", ".ice", ".py", ".pyi"]:
            return False
        return True

    def pre_build(self):
        # MCPP sources
        if not os.path.exists(mcpp_local_filename):
            with urllib.request.urlopen(mcpp_url) as response:
                with open(mcpp_local_filename, "wb") as f:
                    f.write(response.read())
            os.system(f"tar -xzf {mcpp_local_filename} -C dist")

        # Bzip2 sources
        if not os.path.exists(bzip2_local_filename):
            with urllib.request.urlopen(bzip2_url) as response:
                with open(bzip2_local_filename, "wb") as f:
                    f.write(response.read())
            os.system(f"tar -xzf {bzip2_local_filename} -C dist")

        # Ice sources
        if sys.platform == "win32":
            # Build ZeroC.Ice.Slice.Tools.Cpp MSBuild tasks require to build the generated sources
            msbuild_args = f'/p:Configuration={configuration} /p:Platform="Any CPU"'
            os.system(
                f"MSBuild ../cpp/tools/ZeroC.Ice.Slice.Tools.Cpp/ZeroC.Ice.Slice.Tools.Cpp.slnx {msbuild_args} /t:Restore"
            )
            os.system(f"MSBuild ../cpp/tools/ZeroC.Ice.Slice.Tools.Cpp/ZeroC.Ice.Slice.Tools.Cpp.slnx {msbuild_args}")
            # Build slice2cpp and slice2py required to generate the C++ and Python sources included in the pip source dist
            msbuild_args = f"/p:Configuration={configuration} /p:Platform={platform}"
            os.system(f"MSBuild /m ../cpp/msbuild/ice.proj {msbuild_args} /t:NuGetRestore")
            os.system(f"MSBuild /m ../cpp/msbuild/ice.slnx {msbuild_args} /t:slice2cpp;slice2py")
            # Build the SliceCompile target to generate the Ice, IceDiscovery, and IceLocatorDiscovery
            # sources included in the pip source dist
            for project in ["Ice", "IceDiscovery", "IceLocatorDiscovery"]:
                project_path = f"../cpp/src/{project}/msbuild/{project}/{project}.vcxproj"
                os.system(f"MSBuild {project_path} {msbuild_args} /t:SliceCompile")

        else:
            cpp_source_dir = os.path.join(script_directory, "..", "cpp")
            cpp_targets = "slice2cpp slice2py generate-srcs"
            os.system(f"make OPTIMIZE=yes -C {cpp_source_dir} {cpp_targets} {cpp_source_dir}")

        for source_dir in ice_cpp_sources:
            for root, dirs, files in os.walk(os.path.normpath(source_dir)):
                for file in files:
                    if self.include_file(os.path.join(root, file)):
                        if sys.platform == "win32":
                            # On Windows Skip generated files that doesn't correspond to the current platform
                            # and configuration.
                            if "generated" in root and os.path.join(platform, configuration) not in root:
                                continue
                            elif "msbuild" in root and os.path.join(platform, configuration) not in root:
                                continue

                        source_file = os.path.join(root, file)
                        relative_path = os.path.relpath(source_file, "..")

                        # The Python source distribution must use the same layout on all platforms to ensure it works
                        # on all platforms independently of the platform used to create it.
                        #
                        # In practice we use the macOS / Linux layout for all platforms.
                        if root.startswith(
                            os.path.join("..", "cpp", "include", "generated", platform, configuration, "Ice")
                        ):
                            relative_path = os.path.join("cpp", "include", "generated", "Ice", file)
                        else:
                            for name in ["Ice", "IceDiscovery", "IceLocatorDiscovery"]:
                                if root.startswith(os.path.join("..", "cpp", "src", name, "msbuild")):
                                    if file.endswith(".h"):
                                        relative_path = os.path.join("cpp", "include", "generated", name, file)
                                    else:
                                        relative_path = os.path.join("cpp", "src", name, "generated", file)
                                    break

                        target_file = os.path.join(script_directory, "dist/ice", relative_path)
                        target_dir = os.path.dirname(target_file)
                        if not os.path.exists(target_dir):
                            os.makedirs(target_dir)
                        shutil.copy(source_file, target_file)

        # IcePy sources
        for root, dirs, files in os.walk("modules/IcePy"):
            for file in files:
                if file.endswith(".cpp") or file.endswith(".h"):
                    source_file = os.path.join(root, file)
                    relative_path = os.path.relpath(source_file, ".")
                    target_file = os.path.join(script_directory, "dist/ice/python", relative_path)
                    target_dir = os.path.dirname(target_file)
                    if not os.path.exists(target_dir):
                        os.makedirs(target_dir)
                    shutil.copy(source_file, target_file)

        # Slice sources
        slice_excludes = ["IceDiscovery", "IceLocatorDiscovery"]
        for root, dirs, files in os.walk("../slice"):
            # Remove excluded directories from dirs to prevent traversal
            dirs[:] = [d for d in dirs if d not in slice_excludes]

            for file in files:
                source_file = os.path.join(root, file)
                relative_path = os.path.relpath(source_file, "..")
                target_file = os.path.join(script_directory, "dist/lib", relative_path)
                target_dir = os.path.dirname(target_file)
                if not os.path.exists(target_dir):
                    os.makedirs(target_dir)
                shutil.copy(source_file, target_file)

        # Python sources
        if sys.platform == "win32":
            os.system(
                f"MSBuild msbuild/ice.proj /t:SliceCompile /p:Configuration={configuration} /p:Platform={platform}"
            )
        else:
            os.system("make generate-srcs")

        for root, dirs, files in os.walk("python"):
            for file in files:
                if file.endswith(".py") or file.endswith(".pyi") or file == "py.typed":
                    source_file = os.path.join(root, file)
                    relative_path = os.path.relpath(source_file, "python")
                    target_file = os.path.join(script_directory, "dist/lib", relative_path)
                    target_dir = os.path.dirname(target_file)
                    if not os.path.exists(target_dir):
                        os.makedirs(target_dir)
                    shutil.copy(source_file, target_file)

    def run(self):
        if os.path.exists("../cpp"):
            self.pre_build()

        global sources  # Use the global sources list
        sources = []  # Clear the sources list
        for root, _, files in os.walk("dist"):
            for file in files:
                if self.include_file(os.path.join(root, file)):
                    sources.append(os.path.join(root, file))

        self.distribution.ext_modules[0].sources = sources
        _sdist.run(self)


# Initially empty, to be populated by PreBuildCommand
sources = []

# Define the IcePy extension module
ice_py = Extension(
    "IcePy",
    sources=sources,
    include_dirs=include_dirs,
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
    define_macros=define_macros,
    libraries=libraries,
)

# Setup configuration for the package
setup(
    packages=find_namespace_packages(where="dist/lib"),
    package_dir={"": "dist/lib"},
    package_data={"slice": ["*.ice"]},
    include_package_data=True,
    ext_modules=[ice_py],
    cmdclass={
        "build_ext": CustomBuildExtCommand,
        "sdist": CustomSdistCommand,
    },
)
