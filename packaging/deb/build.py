#!/usr/bin/env python3
# Copyright (c) ZeroC, Inc.

"""
Local DEB package builder that emulates the GitHub Actions workflow.
Builds Debian/Ubuntu packages using Docker containers.
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path
from typing import Tuple

# Add parent directory to path to import common module
sys.path.insert(0, str(Path(__file__).parent.parent / 'scripts'))
from common import (
    get_deb_arch,
    load_channel_from_version_env,
    find_ice_repo,
    validate_ice_repo,
    is_image_outdated,
)


# Distribution to build profile mapping
DISTRIBUTION_PROFILES = {
    'debian12': 'no-python312',
    'debian13': '',
    'ubuntu24.04': '',
}

# Distribution to Dockerfile path mapping
DOCKERFILE_PATHS = {
    'debian12': 'packaging/deb/docker/debian12/Dockerfile',
    'debian13': 'packaging/deb/docker/debian13/Dockerfile',
    'ubuntu24.04': 'packaging/deb/docker/ubuntu24.04/Dockerfile',
}


def get_dockerfile_path(distribution: str, ice_repo: Path) -> Path:
    """Get the Dockerfile path for a distribution."""
    if distribution not in DOCKERFILE_PATHS:
        raise ValueError(f"Unknown distribution: {distribution}")
    
    dockerfile = ice_repo / DOCKERFILE_PATHS[distribution]
    if not dockerfile.exists():
        raise FileNotFoundError(f"Dockerfile not found: {dockerfile}")
    
    return dockerfile


def build_docker_image(distribution: str, ice_repo: Path, channel: str, force: bool = False) -> str:
    """Build Docker image from Dockerfile."""
    dockerfile_path = get_dockerfile_path(distribution, ice_repo)
    image_name = f"ice-deb-builder-{distribution}:{channel}"
    
    # Check if build is needed
    if not force and not is_image_outdated(image_name, dockerfile_path):
        print(f"✓ Docker image '{image_name}' is up to date")
        return image_name
    
    print(f"Building Docker image '{image_name}' from {dockerfile_path.relative_to(ice_repo)}")
    
    # Build context is the ice repository root (like GitHub Actions)
    build_context = ice_repo
    
    # Build the image
    build_cmd = [
        'docker', 'build',
        '-t', image_name,
        '-f', str(dockerfile_path),
        str(build_context)
    ]
    
    print(f"Running: {' '.join(build_cmd)}")
    result = subprocess.run(build_cmd)
    
    if result.returncode != 0:
        raise RuntimeError(f"Failed to build Docker image '{image_name}'")
    
    print(f"✓ Successfully built '{image_name}'")
    return image_name


def resolve_image(distribution: str, ice_repo: Path, channel: str, pull: bool = False, build: bool = False, force_build: bool = False) -> Tuple[str, bool]:
    """Resolve which Docker image to use based on flags and availability."""
    local_image = f"ice-deb-builder-{distribution}:{channel}"
    remote_image = f"ghcr.io/zeroc-ice/ice-deb-builder-{distribution}:{channel}"
    
    # Priority: force_build > build > pull > local > remote
    if force_build or build:
        return build_docker_image(distribution, ice_repo, channel, force=force_build), False
    
    if pull:
        print(f"Will pull image from registry: {remote_image}")
        return remote_image, True
    
    # Check if local image exists
    result = subprocess.run(
        ['docker', 'images', '-q', local_image],
        capture_output=True,
        text=True
    )
    
    if result.stdout.strip():
        print(f"Using local image: {local_image}")
        return local_image, False
    else:
        print(f"Local image not found, will pull: {remote_image}")
        return remote_image, True


def run_interactive_session(args: argparse.Namespace) -> int:
    """Start an interactive bash session in the Docker container."""
    
    # Resolve ice repository path
    if args.ice_repo:
        ice_repo = Path(args.ice_repo).resolve()
    else:
        ice_repo = find_ice_repo()
        if not ice_repo:
            print("Error: Could not find ice repository.", file=sys.stderr)
            print("Please specify --ice-repo or run from within the repository.", file=sys.stderr)
            return 1
    
    # Validate ice repository
    try:
        validate_ice_repo(ice_repo)
    except FileNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    
    # Load channel from config/version.env
    try:
        channel = load_channel_from_version_env(ice_repo)
    except (FileNotFoundError, ValueError) as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    
    # Validate distribution
    if args.distribution not in DISTRIBUTION_PROFILES:
        print(f"Error: Unknown distribution '{args.distribution}'", file=sys.stderr)
        print(f"Supported distributions: {', '.join(DISTRIBUTION_PROFILES.keys())}", file=sys.stderr)
        return 1
    
    # Resolve Docker image
    try:
        image_name, need_pull = resolve_image(
            args.distribution,
            ice_repo,
            channel,
            pull=args.pull,
            build=args.build_image,
            force_build=args.force_build_image
        )
    except (ValueError, FileNotFoundError, RuntimeError) as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    
    # Pull image if needed
    if need_pull:
        print(f"Pulling Docker image: {image_name}")
        result = subprocess.run(['docker', 'pull', image_name])
        if result.returncode != 0:
            print(f"Error: Failed to pull image '{image_name}'", file=sys.stderr)
            return 1
    
    # Setup output directory (for convenience)
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Build Docker command for interactive session
    docker_cmd = [
        'docker', 'run', '--rm', '-it',
        '--user', f'{os.getuid()}:{os.getgid()}',
        '-e', 'HOME=/workspace',
        '-v', f'{output_dir}:/workspace',
        '-v', f'{ice_repo}:/workspace/ice:ro',
        '-w', '/workspace',
        image_name,
        '/bin/bash'
    ]
    
    # Print configuration summary
    print("\n" + "=" * 70)
    print("Interactive Session")
    print("=" * 70)
    print(f"Distribution:        {args.distribution}")
    print(f"Channel:             {channel}")
    print(f"Docker Image:        {image_name}")
    print(f"Ice Repository:      {ice_repo} (mounted at /workspace/ice)")
    print(f"Working Directory:   {output_dir} (mounted at /workspace)")
    print("=" * 70)
    print("\nStarting interactive bash session...")
    print("Type 'exit' to leave the container.\n")
    
    # Run the interactive session
    result = subprocess.run(docker_cmd)
    
    return result.returncode


def run_package_build(args: argparse.Namespace) -> int:
    """Run the DEB package build in Docker container."""
    
    # Resolve ice repository path
    if args.ice_repo:
        ice_repo = Path(args.ice_repo).resolve()
    else:
        ice_repo = find_ice_repo()
        if not ice_repo:
            print("Error: Could not find ice repository.", file=sys.stderr)
            print("Please specify --ice-repo or run from within the repository.", file=sys.stderr)
            return 1
    
    # Validate ice repository
    try:
        validate_ice_repo(ice_repo)
    except FileNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    
    # Load channel from config/version.env
    try:
        channel = load_channel_from_version_env(ice_repo)
    except (FileNotFoundError, ValueError) as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    
    # Validate distribution
    if args.distribution not in DISTRIBUTION_PROFILES:
        print(f"Error: Unknown distribution '{args.distribution}'", file=sys.stderr)
        print(f"Supported distributions: {', '.join(DISTRIBUTION_PROFILES.keys())}", file=sys.stderr)
        return 1
    
    # Get host architecture
    try:
        arch = get_deb_arch()
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    
    # Setup output directory
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Resolve Docker image
    try:
        image_name, need_pull = resolve_image(
            args.distribution,
            ice_repo,
            channel,
            pull=args.pull,
            build=args.build_image,
            force_build=args.force_build_image
        )
    except (ValueError, FileNotFoundError, RuntimeError) as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    
    # Pull image if needed
    if need_pull:
        print(f"Pulling Docker image: {image_name}")
        result = subprocess.run(['docker', 'pull', image_name])
        if result.returncode != 0:
            print(f"Error: Failed to pull image '{image_name}'", file=sys.stderr)
            return 1
    
    # Get environment variables
    dh_build_ddebs = os.environ.get('DH_BUILD_DDEBS', '0')
    deb_build_options = os.environ.get('DEB_BUILD_OPTIONS', 'nocheck parallel=4')
    deb_build_profiles = os.environ.get('DEB_BUILD_PROFILES', DISTRIBUTION_PROFILES[args.distribution])
    
    # Build Docker command
    docker_cmd = [
        'docker', 'run', '--rm',
        '--user', f'{os.getuid()}:{os.getgid()}',
        '-e', 'HOME=/workspace',
        '-v', f'{output_dir}:/workspace',
        '-v', f'{ice_repo}:/workspace/ice:ro',
        '-e', f'DEBEMAIL={args.email}',
        '-e', f'DEBFULLNAME={args.fullname}',
        '-e', f'DH_BUILD_DDEBS={dh_build_ddebs}',
        '-e', f'DEB_BUILD_OPTIONS={deb_build_options}',
    ]
    
    if args.ice_version:
        docker_cmd.extend(['-e', f'ICE_VERSION={args.ice_version}'])
    
    if deb_build_profiles:
        docker_cmd.extend(['-e', f'DEB_BUILD_PROFILES={deb_build_profiles}'])
    
    docker_cmd.extend([
        image_name,
        '/workspace/ice/packaging/deb/build-package.sh'
    ])
    
    # Print configuration summary
    print("\n" + "=" * 70)
    print("Build Configuration")
    print("=" * 70)
    print(f"Distribution:        {args.distribution}")
    print(f"Architecture:        {arch}")
    print(f"Channel:             {channel}")
    print(f"Docker Image:        {image_name}")
    print(f"Ice Repository:      {ice_repo}")
    print(f"Output Directory:    {output_dir}")
    print(f"Ice Version:         {args.ice_version or '(from changelog)'}")
    print(f"DEBEMAIL:            {args.email}")
    print(f"DEBFULLNAME:         {args.fullname}")
    print(f"DH_BUILD_DDEBS:      {dh_build_ddebs}")
    print(f"DEB_BUILD_OPTIONS:   {deb_build_options}")
    print(f"DEB_BUILD_PROFILES:  {deb_build_profiles or '(none)'}")
    print("=" * 70 + "\n")
    
    # Run the build
    print("Starting package build...")
    print(f"Command: {' '.join(docker_cmd)}\n")
    
    result = subprocess.run(docker_cmd)
    
    if result.returncode == 0:
        print("\n" + "=" * 70)
        print("✓ Build completed successfully!")
        print("=" * 70)
        print(f"Build artifacts are in: {output_dir}")
        
        # List build artifacts
        artifacts = list(output_dir.glob('*.deb')) + \
                   list(output_dir.glob('*.ddeb')) + \
                   list(output_dir.glob('*.dsc')) + \
                   list(output_dir.glob('*.tar.*')) + \
                   list(output_dir.glob('*.changes')) + \
                   list(output_dir.glob('*.buildinfo'))
        
        if artifacts:
            print("\nArtifacts:")
            for artifact in sorted(artifacts):
                print(f"  - {artifact.name}")
        print()
    else:
        print("\n" + "=" * 70, file=sys.stderr)
        print("✗ Build failed!", file=sys.stderr)
        print("=" * 70, file=sys.stderr)
    
    return result.returncode


def main() -> int:
    parser = argparse.ArgumentParser(
        description='Build DEB packages locally using Docker containers',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Basic build for Ubuntu 24.04
  %(prog)s -d ubuntu24.04

  # Build with custom output directory
  %(prog)s -d debian12 -o ~/deb-packages

  # Start an interactive bash session in the container
  %(prog)s -d ubuntu24.04 --interactive

  # Build local Docker image if outdated
  %(prog)s -d ubuntu24.04 --build-image

  # Force rebuild of Docker image (for testing Dockerfile changes)
  %(prog)s -d debian12 --force-build-image

  # Use environment variables for build options
  DEB_BUILD_OPTIONS="parallel=8" %(prog)s -d ubuntu24.04

Supported distributions: """ + ', '.join(DISTRIBUTION_PROFILES.keys())
    )
    
    parser.add_argument(
        '-d', '--distribution',
        required=True,
        choices=list(DISTRIBUTION_PROFILES.keys()),
        help='Distribution to build for'
    )
    
    parser.add_argument(
        '-o', '--output-dir',
        default='./build-output',
        help='Output directory for build artifacts (default: ./build-output)'
    )
    
    parser.add_argument(
        '-r', '--ice-repo',
        help='Path to ice repository (default: auto-detect)'
    )
    
    parser.add_argument(
        '-v', '--ice-version',
        help='Ice version for build (default: from changelog)'
    )
    
    parser.add_argument(
        '-i', '--image',
        help='Docker image name override'
    )
    
    parser.add_argument(
        '--pull',
        action='store_true',
        help='Pull image from ghcr.io even if local exists'
    )
    
    parser.add_argument(
        '--build-image',
        action='store_true',
        help='Build Docker image locally if outdated or missing'
    )
    
    parser.add_argument(
        '--force-build-image',
        action='store_true',
        help='Force rebuild of Docker image even if up to date'
    )
    
    parser.add_argument(
        '--email',
        default='local@localhost',
        help='DEBEMAIL value (default: local@localhost)'
    )
    
    parser.add_argument(
        '--fullname',
        default='Local Build',
        help='DEBFULLNAME value (default: Local Build)'
    )
    
    parser.add_argument(
        '--interactive',
        action='store_true',
        help='Start an interactive bash session in the Docker image instead of building'
    )
    
    args = parser.parse_args()
    
    # Check for conflicting options
    if args.pull and (args.build_image or args.force_build_image):
        parser.error("Cannot use --pull with --build-image or --force-build-image")
    
    try:
        if args.interactive:
            return run_interactive_session(args)
        else:
            return run_package_build(args)
    except KeyboardInterrupt:
        print("\n\nBuild interrupted by user", file=sys.stderr)
        return 130


if __name__ == '__main__':
    sys.exit(main())
