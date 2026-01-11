#!/usr/bin/env python3
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#
# This script creates the Swift Package Manager source distribution for Ice.
# It compiles the Slice translators, generates source files, and assembles
# the SPM package structure under packaging/swift/spm.
#

import argparse
import glob
import os
import shutil
import subprocess
import sys


def info(msg):
    """Print info message."""
    print(f"[INFO] {msg}")


def error(msg):
    """Print error message and exit."""
    print(f"[ERROR] {msg}", file=sys.stderr)
    sys.exit(1)


def run_command(cmd, cwd=None, check=True):
    """Run a shell command."""
    info(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if check and result.returncode != 0:
        error(f"Command failed: {result.stderr}")
    return result


def copy_files(src_pattern, dest_dir, excludes=None):
    """Copy files matching a glob pattern to destination directory."""
    excludes = excludes or []
    for f in glob.glob(src_pattern):
        basename = os.path.basename(f)
        if os.path.isfile(f) and not any(basename.startswith(e) for e in excludes):
            shutil.copy2(f, dest_dir)


def copy_tree(src_dir, dest_dir):
    """Copy entire directory tree."""
    if os.path.exists(dest_dir):
        shutil.rmtree(dest_dir)
    shutil.copytree(src_dir, dest_dir)


def main():
    parser = argparse.ArgumentParser(description="Build Ice Swift Package Manager source distribution")
    parser.add_argument("--output-dir", default=None,
                        help="Output directory for SPM sources (default: packaging/swift/spm)")
    args = parser.parse_args()

    # Determine Ice repository root from script location
    # Script is in ice/packaging/swift, so ice root is ../../
    script_dir = os.path.dirname(os.path.abspath(__file__))
    ice_dir = os.path.abspath(os.path.join(script_dir, "..", ".."))

    # Determine output directory
    if args.output_dir:
        output_dir = os.path.abspath(args.output_dir)
    else:
        output_dir = os.path.join(ice_dir, "packaging", "swift", "spm")

    packaging_dir = os.path.dirname(output_dir)
    cpp_dir = os.path.join(ice_dir, "cpp")
    swift_dir = os.path.join(ice_dir, "swift")
    slice_dir = os.path.join(ice_dir, "slice")

    info(f"Ice repository: {ice_dir}")
    info(f"Output directory: {output_dir}")

    # Create output directory structure
    src_dir = output_dir
    sources_dir = os.path.join(src_dir, "Sources")

    directories = [
        os.path.join(sources_dir, "IceImpl", "include"),
        os.path.join(sources_dir, "IceLocatorDiscoveryCpp", "include", "IceLocatorDiscovery"),
        os.path.join(sources_dir, "IceCpp", "include", "Ice"),
        os.path.join(sources_dir, "IceCpp", "include", "IceUtil"),
        os.path.join(sources_dir, "IceCpp", "include", "IceIAP"),
        os.path.join(sources_dir, "IceSSLCpp", "include", "IceSSL"),
        os.path.join(sources_dir, "IceDiscoveryCpp", "include", "IceDiscovery"),
        os.path.join(sources_dir, "Ice"),
        os.path.join(sources_dir, "Glacier2"),
        os.path.join(sources_dir, "IceGrid"),
        os.path.join(sources_dir, "IceStorm"),
    ]

    info("Creating directory structure")
    for d in directories:
        os.makedirs(d, exist_ok=True)

    # Build translators
    info("Building slice2cpp and slice2swift")
    run_command(["make", "slice2cpp", "slice2swift"], cwd=cpp_dir)

    slice2cpp = os.path.join(cpp_dir, "bin", "slice2cpp")
    slice2swift = os.path.join(cpp_dir, "bin", "slice2swift")

    # Generate C++ sources from Slice files
    info("Generating C++ sources from Slice files")
    generated_cpp_dir = os.path.join(src_dir, ".generated", "cpp")

    for component in ["Ice", "IceSSL", "IceDiscovery", "IceLocatorDiscovery", "IceIAP"]:
        component_generated = os.path.join(generated_cpp_dir, component)
        os.makedirs(component_generated, exist_ok=True)
        for ice_file in glob.glob(os.path.join(slice_dir, component, "*.ice")):
            run_command([
                slice2cpp,
                f"-I{slice_dir}",
                "-DICE_SWIFT",
                "--include-dir", component,
                "--output-dir", component_generated,
                ice_file
            ])

    info("Copying sources")

    # Copy IceUtil sources
    copy_files(os.path.join(cpp_dir, "src", "IceUtil", "*.cpp"),
               os.path.join(sources_dir, "IceCpp"))
    copy_files(os.path.join(cpp_dir, "src", "IceUtil", "*.h"),
               os.path.join(sources_dir, "IceCpp", "include", "IceUtil"))
    copy_files(os.path.join(cpp_dir, "include", "IceUtil", "*.h"),
               os.path.join(sources_dir, "IceCpp", "include", "IceUtil"))

    excludes = ["SChannel", "OpenSSL", "DLLMain.cpp"]

    # Copy Ice and IceSSL sources
    for d in ["Ice", "IceSSL"]:
        target_cpp = f"{d}Cpp"

        copy_files(os.path.join(cpp_dir, "src", d, "*.cpp"),
                   os.path.join(sources_dir, target_cpp), excludes)
        copy_files(os.path.join(cpp_dir, "src", d, "*.h"),
                   os.path.join(sources_dir, target_cpp, "include", d), excludes)
        copy_files(os.path.join(cpp_dir, "include", d, "*.h"),
                   os.path.join(sources_dir, target_cpp, "include", d), excludes)
        # Copy generated sources
        copy_files(os.path.join(generated_cpp_dir, d, "*.cpp"),
                   os.path.join(sources_dir, target_cpp))
        copy_files(os.path.join(generated_cpp_dir, d, "*.h"),
                   os.path.join(sources_dir, target_cpp, "include", d))

    # Copy iOS-specific Ice sources
    copy_files(os.path.join(cpp_dir, "src", "Ice", "ios", "*.cpp"),
               os.path.join(sources_dir, "IceCpp"))
    copy_files(os.path.join(cpp_dir, "src", "Ice", "ios", "*.h"),
               os.path.join(sources_dir, "IceCpp"))

    # Copy IceDiscovery and IceLocatorDiscovery sources
    for d in ["IceDiscovery", "IceLocatorDiscovery"]:
        target_cpp = f"{d}Cpp"

        copy_files(os.path.join(cpp_dir, "src", d, "*.h"),
                   os.path.join(sources_dir, target_cpp, "include", d))
        copy_files(os.path.join(cpp_dir, "src", d, "*.cpp"),
                   os.path.join(sources_dir, target_cpp))
        # Copy generated sources
        copy_files(os.path.join(generated_cpp_dir, d, "*.cpp"),
                   os.path.join(sources_dir, target_cpp))
        copy_files(os.path.join(generated_cpp_dir, d, "*.h"),
                   os.path.join(sources_dir, target_cpp, "include", d))

    # Copy IceIAP sources
    copy_files(os.path.join(cpp_dir, "include", "IceIAP", "*.h"),
               os.path.join(sources_dir, "IceCpp", "include", "IceIAP"))
    copy_files(os.path.join(generated_cpp_dir, "IceIAP", "*.cpp"),
               os.path.join(sources_dir, "IceCpp"))
    copy_files(os.path.join(generated_cpp_dir, "IceIAP", "*.h"),
               os.path.join(sources_dir, "IceCpp", "include", "IceIAP"))

    # Copy IceImpl sources (Swift/ObjC++ bridge)
    copy_files(os.path.join(swift_dir, "src", "IceImpl", "*.mm"),
               os.path.join(sources_dir, "IceImpl"))
    copy_files(os.path.join(swift_dir, "src", "IceImpl", "Convert.h"),
               os.path.join(sources_dir, "IceImpl"))
    copy_files(os.path.join(swift_dir, "src", "IceImpl", "LoggerWrapperI.h"),
               os.path.join(sources_dir, "IceImpl"))

    # Copy other IceImpl headers to include directory (excluding Convert.h and LoggerWrapperI.h)
    for h in glob.glob(os.path.join(swift_dir, "src", "IceImpl", "*.h")):
        basename = os.path.basename(h)
        if basename not in ["Convert.h", "LoggerWrapperI.h"]:
            shutil.copy2(h, os.path.join(sources_dir, "IceImpl", "include"))

    # Copy iOS-specific Objective-C++ sources
    copy_files(os.path.join(cpp_dir, "src", "Ice", "ios", "*.mm"),
               os.path.join(sources_dir, "IceImpl"))
    copy_files(os.path.join(cpp_dir, "src", "IceIAP", "*.mm"),
               os.path.join(sources_dir, "IceImpl"))
    copy_files(os.path.join(cpp_dir, "src", "IceIAP", "*.h"),
               os.path.join(sources_dir, "IceImpl"))

    # Copy Swift sources
    copy_files(os.path.join(swift_dir, "src", "Ice", "*.swift"),
               os.path.join(sources_dir, "Ice"))

    # Generate Swift sources from Slice files for Ice, IceSSL, IceIAP
    info("Generating Swift sources from Slice files")
    for d in ["Ice", "IceSSL", "IceIAP"]:
        swift_generated = os.path.join(src_dir, ".generated", "swift", d)
        os.makedirs(swift_generated, exist_ok=True)

        slice_subdir = os.path.join(slice_dir, d)
        for ice_file in glob.glob(os.path.join(slice_subdir, "*.ice")):
            run_command([
                slice2swift,
                f"-I{slice_dir}",
                "-DICE_SWIFT",
                "--output-dir", swift_generated,
                ice_file
            ])

        # Rename generated Swift files with prefix
        for swift_file in glob.glob(os.path.join(swift_generated, "*.swift")):
            basename = os.path.basename(swift_file)
            new_name = f"{d}_{basename}"
            shutil.copy2(swift_file, os.path.join(sources_dir, "Ice", new_name))

    # Generate Swift sources for Glacier2, IceGrid, IceStorm
    for d in ["Glacier2", "IceGrid", "IceStorm"]:
        output = os.path.join(sources_dir, d)
        slice_subdir = os.path.join(slice_dir, d)

        for ice_file in glob.glob(os.path.join(slice_subdir, "*.ice")):
            run_command([
                slice2swift,
                f"-I{slice_dir}",
                "-DICE_SWIFT",
                "--output-dir", output,
                ice_file
            ])

    # Copy Package.swift, README.md and license files
    info("Copying package files")
    shutil.copy2(os.path.join(packaging_dir, "Package.swift"), src_dir)
    shutil.copy2(os.path.join(packaging_dir, "README.md"), src_dir)
    shutil.copy2(os.path.join(ice_dir, "LICENSE"), src_dir)
    shutil.copy2(os.path.join(ice_dir, "ICE_LICENSE"), src_dir)

    # Clean up generated temp directories
    generated_temp = os.path.join(src_dir, ".generated")
    if os.path.exists(generated_temp):
        shutil.rmtree(generated_temp)

    info(f"SPM source distribution created successfully at: {output_dir}")
    info("You can now use 'swift build' in the output directory to build the package.")


if __name__ == "__main__":
    main()
