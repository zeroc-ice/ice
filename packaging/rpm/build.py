#!/usr/bin/env python3
# Copyright (c) ZeroC, Inc.

"""
Local RPM package builder that emulates the GitHub Actions workflow.
Builds RHEL/Amazon Linux packages using Docker containers.
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
    get_rpm_arch,
    load_channel_from_version_env,
    find_ice_repo,
    validate_ice_repo,
    is_image_outdated,
)


# Supported RPM distributions (el10 not yet available on this branch)
DISTRIBUTIONS = ['el9', 'amzn2023']

# Distribution to Dockerfile path mapping
DOCKERFILE_PATHS = {
    'el9': 'packaging/rpm/docker/el9/Dockerfile',
    'amzn2023': 'packaging/rpm/docker/amzn2023/Dockerfile',
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
    image_name = f"ice-rpm-builder-{distribution}:{channel}"
    
    # Check if build is needed
    if not force and not is_image_outdated(image_name, dockerfile_path):
        print(f"✓ Docker image '{image_name}' is up to date")
        return image_name
    
    print(f"Building Docker image '{image_name}' from {dockerfile_path.relative_to(ice_repo)}")
    
    # Build context is the ice repository root (like GitHub Actions)
    build_context = ice_repo
    
    # Check if RH credentials are needed (for el9, el10)
    needs_rh_credentials = distribution.startswith('el')
    rh_credentials_file = None
    
    if needs_rh_credentials:
        rh_username = os.environ.get('RH_USERNAME')
        rh_password = os.environ.get('RH_PASSWORD')
        
        if not rh_username or not rh_password:
            raise RuntimeError(
                f"Building {distribution} image requires Red Hat subscription credentials.\n"
                f"Set RH_USERNAME and RH_PASSWORD environment variables:\n"
                f"  export RH_USERNAME='your-username'\n"
                f"  export RH_PASSWORD='your-password'"
            )
        
        # Create temporary credentials file (like GitHub Actions does)
        import tempfile
        fd, rh_credentials_file = tempfile.mkstemp(prefix='rh_credentials_', text=True)
        try:
            os.chmod(rh_credentials_file, 0o600)  # Secure permissions
            with os.fdopen(fd, 'w') as f:
                f.write(f'RH_USERNAME="{rh_username}"\n')
                f.write(f'RH_PASSWORD="{rh_password}"\n')
            print(f"Created temporary RH credentials file: {rh_credentials_file}")
        except Exception:
            os.close(fd)
            if os.path.exists(rh_credentials_file):
                os.unlink(rh_credentials_file)
            raise
    
    try:
        # Build the image
        build_cmd = [
            'docker', 'build',
            '-t', image_name,
            '-f', str(dockerfile_path),
        ]
        
        # Add secret with RH credentials if needed
        if needs_rh_credentials and rh_credentials_file:
            build_cmd.extend(['--secret', f'id=rh_credentials,src={rh_credentials_file}'])
        
        build_cmd.append(str(build_context))
        
        print(f"Running: {' '.join(build_cmd)}")
        result = subprocess.run(build_cmd)
        
        if result.returncode != 0:
            raise RuntimeError(f"Failed to build Docker image '{image_name}'")
        
        print(f"✓ Successfully built '{image_name}'")
        return image_name
    
    finally:
        # Clean up credentials file
        if rh_credentials_file and os.path.exists(rh_credentials_file):
            os.unlink(rh_credentials_file)
            print(f"Cleaned up RH credentials file")


def resolve_image(distribution: str, ice_repo: Path, channel: str, pull: bool = False, build: bool = False, force_build: bool = False) -> Tuple[str, bool]:
    """Resolve which Docker image to use based on flags and availability."""
    local_image = f"ice-rpm-builder-{distribution}:{channel}"
    remote_image = f"ghcr.io/zeroc-ice/ice-rpm-builder-{distribution}:{channel}"
    
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
    if args.distribution not in DISTRIBUTIONS:
        print(f"Error: Unknown distribution '{args.distribution}'", file=sys.stderr)
        print(f"Supported distributions: {', '.join(DISTRIBUTIONS)}", file=sys.stderr)
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
    """Run the RPM package build in Docker container."""
    
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
    if args.distribution not in DISTRIBUTIONS:
        print(f"Error: Unknown distribution '{args.distribution}'", file=sys.stderr)
        print(f"Supported distributions: {', '.join(DISTRIBUTIONS)}", file=sys.stderr)
        return 1
    
    # Get host architecture
    try:
        arch = get_rpm_arch()
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
    
    # Get GPG credentials from environment
    gpg_key = os.environ.get('GPG_KEY', '')
    gpg_key_id = os.environ.get('GPG_KEY_ID', '')
    gpg_passphrase = os.environ.get('GPG_PASSPHRASE', '')
    
    # Validate GPG credentials
    if not gpg_key:
        print("Error: GPG_KEY environment variable is required", file=sys.stderr)
        print("Set it with: export GPG_KEY='<your-gpg-private-key>'", file=sys.stderr)
        return 1
    
    if not gpg_key_id:
        print("Error: GPG_KEY_ID environment variable is required", file=sys.stderr)
        print("Set it with: export GPG_KEY_ID='<your-gpg-key-id>'", file=sys.stderr)
        return 1
    
    # Build Docker command
    docker_cmd = [
        'docker', 'run', '--rm',
        '-v', f'{output_dir}:/workspace',
        '-v', f'{ice_repo}:/workspace/ice:ro',
        '-e', f'GPG_KEY={gpg_key}',
        '-e', f'GPG_KEY_ID={gpg_key_id}',
    ]
    
    # Add optional GPG passphrase if provided
    if gpg_passphrase:
        docker_cmd.extend(['-e', f'GPG_PASSPHRASE={gpg_passphrase}'])
    
    # Add optional ICE_VERSION override
    if args.ice_version:
        docker_cmd.extend(['-e', f'ICE_VERSION={args.ice_version}'])
    
    # Add optional GIT_TAG
    if args.git_tag:
        docker_cmd.extend(['-e', f'GIT_TAG={args.git_tag}'])
    
    docker_cmd.extend([
        image_name,
        '/workspace/ice/packaging/rpm/build-package.sh'
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
    print(f"Ice Version:         {args.ice_version or '(from spec file)'}")
    print(f"Git Tag:             {args.git_tag or '(not set)'}")
    print(f"GPG Key ID:          {gpg_key_id}")
    print(f"GPG Passphrase:      {'(set)' if gpg_passphrase else '(not set)'}")
    print("=" * 70 + "\n")
    
    # Run the build
    print("Starting package build...")
    print(f"Command: {' '.join([c if not c.startswith('GPG_KEY=') else 'GPG_KEY=***' for c in docker_cmd])}\n")
    
    result = subprocess.run(docker_cmd)
    
    if result.returncode == 0:
        print("\n" + "=" * 70)
        print("✓ Build completed successfully!")
        print("=" * 70)
        print(f"Build artifacts are in: {output_dir}")
        
        # List build artifacts
        build_dir = output_dir / 'build'
        if build_dir.exists():
            rpms = list(build_dir.glob('RPMS/**/*.rpm'))
            srpms = list(build_dir.glob('SRPMS/*.rpm'))
            
            artifacts = rpms + srpms
            
            if artifacts:
                print("\nArtifacts:")
                for artifact in sorted(artifacts):
                    print(f"  - {artifact.relative_to(output_dir)}")
        print()
    else:
        print("\n" + "=" * 70, file=sys.stderr)
        print("✗ Build failed!", file=sys.stderr)
        print("=" * 70, file=sys.stderr)
    
    return result.returncode


def main() -> int:
    parser = argparse.ArgumentParser(
        description='Build RPM packages locally using Docker containers',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Basic build for EL9
  %(prog)s -d el9

  # Build with custom output directory
  %(prog)s -d amzn2023 -o ~/rpm-packages

  # Start an interactive bash session in the container
  %(prog)s -d el9 --interactive

  # Build local Docker image if outdated
  %(prog)s -d el9 --build-image

  # Force rebuild of Docker image (for testing Dockerfile changes)
  %(prog)s -d amzn2023 --force-build-image

  # Set GPG credentials for signing packages
  export GPG_KEY="$(cat ~/.gnupg/private-key.asc)"
  export GPG_KEY_ID="ABCD1234"
  %(prog)s -d el9

  # Build EL9 image (requires Red Hat subscription)
  export RH_USERNAME='your-username'
  export RH_PASSWORD='your-password'
  %(prog)s -d el9 --build-image

Environment Variables:
  GPG_KEY         GPG private key for signing packages (required for builds)
  GPG_KEY_ID      GPG key ID (required for builds)
  GPG_PASSPHRASE  GPG passphrase (optional)
  RH_USERNAME     Red Hat subscription username (required for building el9 images)
  RH_PASSWORD     Red Hat subscription password (required for building el9 images)

Supported distributions: """ + ', '.join(DISTRIBUTIONS)
    )
    
    parser.add_argument(
        '-d', '--distribution',
        required=True,
        choices=DISTRIBUTIONS,
        help='Distribution to build for'
    )
    
    parser.add_argument(
        '-o', '--output-dir',
        default='./build',
        help='Output directory for build artifacts (default: ./build)'
    )
    
    parser.add_argument(
        '-r', '--ice-repo',
        help='Path to ice repository (default: auto-detect)'
    )
    
    parser.add_argument(
        '-v', '--ice-version',
        help='Ice version for build (default: from spec file)'
    )
    
    parser.add_argument(
        '-t', '--git-tag',
        help='Git tag for source download (optional)'
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
