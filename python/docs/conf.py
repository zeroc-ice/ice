# Configuration file for the Sphinx documentation builder.
#
# This file configures Sphinx to generate API documentation for Ice for Python.
# It auto-generates per-module RST files for each public symbol in the Ice packages,
# creating a navigable API reference.
#
# For the full list of built-in configuration values, see:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import ast
import importlib
import inspect
import os
import sys
from dataclasses import fields, is_dataclass
from pathlib import Path
from typing import Any

# Add the Ice Python source directory to the path so Sphinx can import the modules.
sys.path.insert(0, os.path.abspath("../python"))
if sys.platform == "win32":
    sys.path.insert(0, os.path.abspath("../python/x64/Release"))

# =============================================================================
# Project Information
# =============================================================================

project = "Ice"
copyright = "ZeroC Inc."
author = "ZeroC Inc."
release = "3.9.0a"
html_title = "Ice 3.9.0a"

# =============================================================================
# Sphinx Extensions
# =============================================================================

extensions = [
    "sphinx.ext.autodoc",  # Auto-generate docs from docstrings
    "sphinx.ext.intersphinx",  # Link to external documentation (e.g., Python stdlib)
    "sphinx.ext.napoleon",  # Support NumPy and Google style docstrings
]

# =============================================================================
# General Configuration
# =============================================================================

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store", "venv"]

# =============================================================================
# HTML Output Options
# =============================================================================

html_theme = "furo"
html_static_path = ["_static"]
html_theme_options = {
    "light_logo": "logo-light.png",
    "dark_logo": "logo-dark.png",
}
html_favicon = "../../assets/favicon-96x96.png"
html_css_files = ["custom.css"]

# =============================================================================
# Autodoc Configuration
# =============================================================================

autodoc_typehints = "both"
autodoc_typehints_description_target = "all"
add_module_names = True
python_use_unqualified_type_names = False

# =============================================================================
# Intersphinx Configuration
# =============================================================================

intersphinx_mapping = {
    "python": ("https://docs.python.org/3", None),
}

# =============================================================================
# Auto-generated API Documentation
# =============================================================================
#
# This section generates one RST file per public symbol (class, function, data)
# exported from each Ice package. The generated files are placed directly in
# the docs root and are recreated on every build.
#
# Structure:
#   Ice.Communicator.rst
#   Ice.ObjectPrx.rst
#   Glacier2.PermissionDeniedException.rst
#   ...

DOCS_ROOT = Path(__file__).parent.resolve()
PYTHON_SRC = DOCS_ROOT.parent / "python"

# Ice packages to document
PACKAGES = ("Ice", "Glacier2", "IceBox", "IceGrid", "IceMX", "IceStorm")


def _get_package_exports(package: str) -> list[tuple[str, str, list[str]]]:
    """
    Get the public exports from a package's __init__.py.

    Parses the __all__ list to find exported names, then categorizes each as
    'class', 'function', or 'data' for the appropriate autodoc directive.

    Returns a list of tuples: (name, kind, exclude_members)
    - name: The exported symbol name
    - kind: 'class', 'function', or 'data'
    - exclude_members: For dataclasses/typed classes, fields to exclude from docs
    """
    init_py = PYTHON_SRC / package / "__init__.py"
    if not init_py.exists():
        return []

    # Parse __all__ from the source file to preserve declaration order
    names = _parse_all_from_source(init_py)

    # Import the module to inspect the actual objects
    try:
        module = importlib.import_module(package)
    except Exception:
        return []

    # Fall back to runtime __all__ if AST parsing found nothing
    if not names and hasattr(module, "__all__"):
        names = list(module.__all__)

    # Categorize each export
    exports = []
    seen = set()
    for name in names:
        if name.startswith("_") or name in seen:
            continue
        if not hasattr(module, name):
            continue

        seen.add(name)
        obj = getattr(module, name)
        kind, exclude_members = _categorize_object(obj)
        exports.append((name, kind, exclude_members))

    return exports


def _parse_all_from_source(init_py: Path) -> list[str]:
    """Parse __all__ = [...] from source to preserve declaration order."""
    tree = ast.parse(init_py.read_text(encoding="utf-8"), filename=str(init_py))
    for node in tree.body:
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == "__all__":
                    if isinstance(node.value, (ast.List, ast.Tuple)):
                        return [
                            elt.value
                            for elt in node.value.elts
                            if isinstance(elt, ast.Constant) and isinstance(elt.value, str)
                        ]
    return []


def _categorize_object(obj: object) -> tuple[str, list[str]]:
    """
    Determine the autodoc directive kind and excluded members for an object.

    For dataclasses and typed classes, we exclude the field names from the
    members list since they're already documented in the class signature.
    """
    if inspect.isclass(obj):
        exclude_members = []
        if is_dataclass(obj):
            exclude_members = [f.name for f in fields(obj)]
        elif hasattr(obj, "__annotations__"):
            exclude_members = [n for n in getattr(obj, "__annotations__", {}) if not n.startswith("_")]
        return "class", exclude_members
    elif inspect.isfunction(obj):
        return "function", []
    else:
        return "data", []


def _generate_rst_page(package: str, name: str, kind: str, exclude_members: list[str]) -> str:
    """Generate RST content for a single exported symbol."""
    title = f"{package}.{name}"
    underline = "=" * len(title)

    if kind == "class":
        options = [
            "   :members:",
            "   :undoc-members:",
            "   :show-inheritance:",
        ]
        if exclude_members:
            options.append(f"   :exclude-members: {', '.join(exclude_members)}")
        return f"{title}\n{underline}\n\n.. autoclass:: {package}.{name}\n" + "\n".join(options) + "\n"
    elif kind == "function":
        return f"{title}\n{underline}\n\n.. autofunction:: {package}.{name}\n"
    else:
        return f"{title}\n{underline}\n\n.. autodata:: {package}.{name}\n"


# Track generated files so we can clean them up on rebuild
_generated_files: list[Path] = []


def _generate_module_pages(_app: Any) -> None:
    """Generate RST files for all public exports in each Ice package."""
    global _generated_files

    # Clean up previously generated files
    for f in _generated_files:
        if f.exists():
            f.unlink()
    _generated_files.clear()

    for package in PACKAGES:
        exports = _get_package_exports(package)
        if not exports:
            continue

        # Generate one RST file per export directly in the docs root
        for name, kind, exclude_members in exports:
            rst_file = DOCS_ROOT / f"{package}.{name}.rst"
            rst_file.write_text(_generate_rst_page(package, name, kind, exclude_members), encoding="utf-8")
            _generated_files.append(rst_file)


def _suppress_builtin_docstrings(
    _app: Any, what: str, _name: str, obj: object, _options: Any, lines: list[str]
) -> None:
    """
    Remove built-in docstrings for simple data constants.

    Avoids noisy documentation like "int(x=0) -> integer" for constant values.
    """
    if what == "data" and isinstance(obj, (int, float, str, bool, bytes)):
        lines.clear()


# =============================================================================
# Sphinx Setup
# =============================================================================


def setup(app: Any) -> None:
    """Register Sphinx event handlers."""
    app.connect("builder-inited", _generate_module_pages)
    app.connect("autodoc-process-docstring", _suppress_builtin_docstrings)
