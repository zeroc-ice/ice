#!/usr/bin/env python3
# Copyright (c) ZeroC, Inc.

"""
Common utilities shared between DEB and RPM packaging scripts.
"""

import platform
import subprocess
from datetime import datetime
from pathlib import Path
from typing import Optional


def get_host_arch() -> str:
    """
    Detect the host architecture and map to package manager arch names.
    
    Returns:
        str: Architecture name (amd64/arm64 for DEB, x86_64/aarch64 for RPM)
    """
    machine = platform.machine()
    if machine == 'x86_64':
        return 'x86_64'  # Works for both DEB (as amd64) and RPM
    elif machine in ('aarch64', 'arm64'):
        return 'aarch64'  # Works for both DEB (as arm64) and RPM
    else:
        raise ValueError(f"Unsupported architecture: {machine}")


def get_deb_arch() -> str:
    """
    Get Debian-specific architecture name.
    
    Returns:
        str: Debian architecture (amd64 or arm64)
    """
    machine = platform.machine()
    if machine == 'x86_64':
        return 'amd64'
    elif machine in ('aarch64', 'arm64'):
        return 'arm64'
    else:
        raise ValueError(f"Unsupported architecture: {machine}")


def get_rpm_arch() -> str:
    """
    Get RPM-specific architecture name.
    
    Returns:
        str: RPM architecture (x86_64 or aarch64)
    """
    machine = platform.machine()
    if machine == 'x86_64':
        return 'x86_64'
    elif machine in ('aarch64', 'arm64'):
        return 'aarch64'
    else:
        raise ValueError(f"Unsupported architecture: {machine}")


def load_channel_from_version_env(ice_repo: Path) -> str:
    """
    Load CHANNEL from config/version.env.
    
    Args:
        ice_repo: Path to the Ice repository root
        
    Returns:
        str: Channel version (e.g., '3.7', '3.8')
        
    Raises:
        FileNotFoundError: If version.env file doesn't exist
        ValueError: If CHANNEL is not found in the file
    """
    version_env = ice_repo / 'config' / 'version.env'
    if not version_env.exists():
        raise FileNotFoundError(f"Version file not found: {version_env}")
    
    channel = None
    with open(version_env) as f:
        for line in f:
            line = line.strip()
            if line.startswith('CHANNEL='):
                channel = line.split('=', 1)[1].strip()
                break
    
    if not channel:
        raise ValueError(f"CHANNEL not found in {version_env}")
    
    return channel


def find_ice_repo() -> Optional[Path]:
    """
    Find the ice repository root by looking for packaging/deb/build-package.sh.
    
    Returns:
        Optional[Path]: Path to ice repository root, or None if not found
    """
    current = Path.cwd()
    
    # Check current directory
    if (current / 'packaging' / 'deb' / 'build-package.sh').exists():
        return current
    
    # Check parent directories
    for parent in current.parents:
        if (parent / 'packaging' / 'deb' / 'build-package.sh').exists():
            return parent
    
    return None


def validate_ice_repo(ice_repo_path: Path) -> bool:
    """
    Validate that the ice repository path is correct.
    
    Args:
        ice_repo_path: Path to validate
        
    Returns:
        bool: True if valid
        
    Raises:
        FileNotFoundError: If repository structure is invalid
    """
    build_script = ice_repo_path / 'packaging' / 'deb' / 'build-package.sh'
    if not build_script.exists():
        raise FileNotFoundError(
            f"Ice repository validation failed: {build_script} not found"
        )
    return True


def is_image_outdated(image_name: str, dockerfile_path: Path) -> bool:
    """
    Check if Docker image is outdated compared to Dockerfile.
    
    Args:
        image_name: Full Docker image name (including tag)
        dockerfile_path: Path to the Dockerfile
        
    Returns:
        bool: True if image needs rebuild, False if up to date
    """
    # Get image creation time
    result = subprocess.run(
        ['docker', 'inspect', '--format={{.Created}}', image_name],
        capture_output=True,
        text=True
    )
    
    if result.returncode != 0:
        return True  # Image doesn't exist, needs build
    
    image_created = result.stdout.strip()
    
    # Get Dockerfile modification time
    dockerfile_mtime = dockerfile_path.stat().st_mtime
    
    # Parse image creation time and compare
    try:
        image_time = datetime.fromisoformat(image_created.replace('Z', '+00:00'))
        dockerfile_time = datetime.fromtimestamp(dockerfile_mtime)
        
        return dockerfile_time > image_time.replace(tzinfo=None)
    except (ValueError, AttributeError):
        # If parsing fails, assume outdated
        return True
