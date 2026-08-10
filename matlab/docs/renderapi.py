#!/usr/bin/env python3
# Copyright (c) ZeroC, Inc.

"""Render matlab-api.json (produced by extractapi.m) into a static HTML API reference.

The help text captured by extractapi.m is the raw MATLAB help comment block: a summary line followed by
3-space-indented body text, with the section conventions used by both the hand-written classes and the code that
slice2matlab generates (Input Arguments, Output Arguments, Exceptions, Examples, See also, ...). This script parses
those conventions and emits one page per class, enumeration, and function, plus an index and a sitemap. It only uses
the Python standard library.
"""

import argparse
import html
import json
import re
import shutil
import sys
from pathlib import Path

BODY_INDENT = 3

# Sections recognized in a help block, at the base indentation of the body text.
HEADER_RE = re.compile(
    r"(?P<memberlist>\S+ (?:Static )?(?:Methods|Properties):)$"
    r"|(?P<args>Input Arguments|Input Name-Value Arguments|Output Arguments)$"
    r"|(?P<exceptions>Exceptions)$"
    r"|(?P<remarks>Remarks)$"
    r"|(?P<examples>Examples?)$"
    r"|(?P<creation>Creation)$"
    r"|(?P<syntax>Syntax)$"
    r"|(?P<seealso>See also\s.*)$"
    r"|(?P<deprecated>Deprecated\b.*)$"
)

# <a href="matlab:help Ice.Future -displayBanner">Ice.Future</a> anchors emitted by slice2matlab.
MATLAB_ANCHOR_RE = re.compile(r'<a\s+href="matlab:([^"]*)"[^>]*>(.*?)</a>')

# Placeholders that carry the anchors through HTML escaping.
PLACEHOLDER_RE = re.compile("\x01(\\d+)\x02")

# A qualified name such as Ice.Communicator or Glacier2.SessionPrx, as it appears in type lines.
QUALIFIED_NAME_RE = re.compile(r"\b[A-Z]\w*(?:\.[A-Z]\w*)+\b")

PAGE_TEMPLATE = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{title}</title>
{canonical}<link rel="stylesheet" href="css/style.css">
</head>
<body>
<header><a href="index.html">Ice for MATLAB {version} API Reference</a></header>
<main>
{main}
</main>
<footer>Copyright &copy; ZeroC, Inc.</footer>
</body>
</html>
"""


def esc(text):
    return html.escape(text, quote=False)


def indent_of(line):
    return len(line) - len(line.lstrip(" "))


class Model:
    """The API model: symbol tables built from matlab-api.json."""

    def __init__(self, api):
        self.api = api
        self.classes = {}  # fully qualified name -> class dict
        self.functions = {}  # fully qualified name -> function dict
        for function in api["functions"]:
            self.functions[function["name"]] = function
        for package in api["packages"]:
            for cls in package["classes"]:
                self.classes[cls["name"]] = cls
            for function in package["functions"]:
                self.functions[function["name"]] = function

    def page(self, name):
        if name in self.classes or name in self.functions:
            return name + ".html"
        return None

    def member_url(self, class_name, member_name):
        """URL of a class member, following inheritance to the page that documents it."""
        cls = self.classes.get(class_name)
        if cls is None:
            return None
        for group in ("methods", "properties"):
            for member in cls[group]:
                if member["name"] == member_name:
                    defining = member["definingClass"]
                    if defining == class_name:
                        return f"{class_name}.html#{member_name}"
                    if defining in self.classes:
                        return f"{defining}.html#{member_name}"
                    return None
        for member in cls["enumerationMembers"]:
            if member["name"] == member_name:
                return f"{class_name}.html#{member_name}"
        constructor = cls.get("constructor")
        if constructor and constructor["name"] == member_name:
            return f"{class_name}.html#{member_name}"
        return None

    def resolve(self, name, context_class=None):
        """Resolve a symbol reference from a See also list or a matlab:help anchor to a URL."""
        name = name.strip().rstrip(".,")
        if not name:
            return None
        if "/" in name:
            class_name, _, member_name = name.partition("/")
            return self.member_url(class_name, member_name)
        page = self.page(name)
        if page:
            return page
        # Class.Member (property, method, or enumeration member).
        parent, _, member_name = name.rpartition(".")
        if parent in self.classes:
            return self.member_url(parent, member_name)
        if context_class:
            url = self.member_url(context_class, name)
            if url:
                return url
            package = context_class.rpartition(".")[0]
            for prefix in (package, "Ice"):
                page = self.page(f"{prefix}.{name}")
                if page:
                    return page
        return None

    def derives_from(self, cls, ancestor_name):
        seen = set()
        names = list(cls["superclasses"])
        while names:
            name = names.pop()
            if name == ancestor_name:
                return True
            if name in seen or name not in self.classes:
                continue
            seen.add(name)
            names.extend(self.classes[name]["superclasses"])
        return False

    def category(self, cls):
        if cls["kind"] == "enum":
            return "Enumerations"
        name = cls["name"]
        if name == "Ice.ObjectPrx" or self.derives_from(cls, "Ice.ObjectPrx"):
            return "Proxies"
        if name == "Ice.Exception" or self.derives_from(cls, "Ice.Exception"):
            return "Exceptions"
        return "Classes"


class HelpRenderer:
    """Renders one help comment block into HTML."""

    def __init__(self, model, context_class=None):
        self.model = model
        self.context_class = context_class
        self.links = []

    def render(self, text, symbol_name, heading_level):
        """Render the help text of symbol_name; sections become <h{heading_level}> headings."""
        if not text:
            return ""
        text = MATLAB_ANCHOR_RE.sub(self._protect_anchor, text)
        lines = text.split("\n")
        lines[0] = self._strip_name_prefix(lines[0], symbol_name)
        blocks = parse_blocks(lines, BODY_INDENT)
        return self._render_blocks(blocks, BODY_INDENT, heading_level)

    def summary(self, text, symbol_name):
        """The first sentence of the help text, as inline HTML."""
        if not text:
            return ""
        first = text.split("\n", 1)[0]
        first = MATLAB_ANCHOR_RE.sub(self._protect_anchor, first)
        first = self._strip_name_prefix(first, symbol_name)
        return self._inline(first)

    def _protect_anchor(self, match):
        self.links.append((match.group(1), match.group(2)))
        return f"\x01{len(self.links) - 1}\x02"

    @staticmethod
    def _strip_name_prefix(line, symbol_name):
        short = symbol_name.rpartition(".")[2]
        token, _, rest = line.partition(" ")
        if token == short.upper():
            return rest
        return line

    def _restore_anchor(self, match):
        target, label = self.links[int(match.group(1))]
        # The target is a MATLAB command such as "help Ice.Future -displayBanner".
        words = [word for word in target.split() if not word.startswith("-")]
        symbol = words[1] if len(words) > 1 and words[0] in ("help", "doc") else words[0] if words else ""
        url = self.model.resolve(symbol, self.context_class)
        if url:
            return f'<a href="{url}">{esc(label)}</a>'
        return esc(label)

    def _inline(self, text, type_line=False):
        text = esc(text)
        text = PLACEHOLDER_RE.sub(self._restore_anchor, text)
        if type_line:
            # In a type line, "|" separates alternatives; it never delimits a |code| span.
            text = QUALIFIED_NAME_RE.sub(self._autolink_name, text)
        else:
            text = re.sub(r"\|([^|\n]+)\|", r"<code>\1</code>", text)
        return text

    def _autolink_name(self, match):
        page = self.model.page(match.group(0))
        if page:
            return f'<a href="{page}">{match.group(0)}</a>'
        return match.group(0)

    def _render_blocks(self, blocks, base_indent, heading_level):
        parts = []
        h = f"h{heading_level}"
        for kind, header, lines in blocks:
            if kind == "para":
                parts.append(self._render_paragraphs(lines))
            elif kind == "memberlist":
                parts.append(f"<{h}>{esc(header.rstrip(':'))}</{h}>")
                parts.append(self._render_member_list(lines, base_indent + 2))
            elif kind == "args":
                parts.append(f"<{h}>{esc(header)}</{h}>")
                parts.append(self._render_arguments(lines, base_indent + 2, typed=True))
            elif kind == "exceptions":
                parts.append(f"<{h}>Exceptions</{h}>")
                parts.append(self._render_arguments(lines, base_indent + 2, typed=False))
            elif kind == "remarks":
                parts.append(f"<{h}>Remarks</{h}>")
                parts.append(self._render_paragraphs(lines))
            elif kind == "examples":
                parts.append(f"<{h}>{esc(header)}</{h}>")
                parts.append(self._render_pre(lines))
            elif kind == "syntax":
                parts.append(self._render_pre(lines, css_class="syntax"))
            elif kind == "creation":
                parts.append(f"<{h}>Creation</{h}>")
                inner = parse_blocks(lines, base_indent + 2)
                parts.append(self._render_blocks(inner, base_indent + 2, min(heading_level + 1, 6)))
            elif kind == "seealso":
                parts.append(self._render_see_also(header, lines))
            elif kind == "deprecated":
                parts.append(self._render_deprecated(header, lines))
        return "\n".join(part for part in parts if part)

    def _render_paragraphs(self, lines):
        # Group the lines into paragraphs and "- item" bullet lists.
        parts = []
        paragraph = []
        items = []

        def flush():
            if paragraph:
                parts.append(f"<p>{self._inline(' '.join(paragraph))}</p>")
                paragraph.clear()
            if items:
                rendered = "".join(f"<li>{self._inline(item)}</li>" for item in items)
                parts.append(f"<ul>{rendered}</ul>")
                items.clear()

        for line in lines:
            stripped = line.strip()
            if not stripped:
                flush()
            elif stripped.startswith("- "):
                if paragraph:
                    flush()
                items.append(stripped[2:])
            elif items:
                items[-1] += " " + stripped
            else:
                paragraph.append(stripped)
        flush()
        return "\n".join(parts)

    def _parse_entries(self, lines, entry_indent):
        """Parse "name - description" entries with indented continuation lines."""
        entries = []
        for line in lines:
            stripped = line.strip()
            if not stripped:
                continue
            match = re.match(r"([\w.]+) - (.*)$", stripped)
            if match and indent_of(line) == entry_indent:
                entries.append((match.group(1), [match.group(2)]))
            elif entries:
                entries[-1][1].append(stripped)
        return entries

    def _render_member_list(self, lines, entry_indent):
        items = []
        for name, description in self._parse_entries(lines, entry_indent):
            url = self.model.member_url(self.context_class, name) if self.context_class else None
            label = f'<a href="{url}">{esc(name)}</a>' if url else f"<code>{esc(name)}</code>"
            items.append(f"<li>{label} &mdash; {self._inline(' '.join(description))}</li>")
        return f'<ul class="members">{"".join(items)}</ul>' if items else ""

    def _render_arguments(self, lines, entry_indent, typed):
        parts = []
        for name, description in self._parse_entries(lines, entry_indent):
            if typed and len(description) > 1:
                # The last line of an argument entry is its type line.
                type_line = f'<span class="type">{self._inline(description[-1], type_line=True)}</span>'
                description = description[:-1]
            else:
                type_line = ""
            if typed:
                title = f"<code>{esc(name)}</code>"
            else:
                page = self.model.page(name)
                title = f'<a href="{page}">{esc(name)}</a>' if page else f"<code>{esc(name)}</code>"
            parts.append(f"<dt>{title}</dt>")
            parts.append(f"<dd>{self._inline(' '.join(description))}{type_line}</dd>")
        return f'<dl class="arguments">{"".join(parts)}</dl>' if parts else ""

    def _render_pre(self, lines, css_class=None):
        while lines and not lines[0].strip():
            lines = lines[1:]
        while lines and not lines[-1].strip():
            lines = lines[:-1]
        if not lines:
            return ""
        dedent = min(indent_of(line) for line in lines if line.strip())
        body = "\n".join(line[dedent:] if line.strip() else "" for line in lines)
        body = PLACEHOLDER_RE.sub(self._restore_anchor, esc(body))
        css = f' class="{css_class}"' if css_class else ""
        return f"<pre{css}>{body}</pre>"

    def _render_see_also(self, header, lines):
        text = " ".join([header[len("See also"):]] + [line.strip() for line in lines if line.strip()])
        rendered = []
        for name in text.split(","):
            name = name.strip().rstrip(".")
            if not name:
                continue
            url = self.model.resolve(name, self.context_class)
            rendered.append(f'<a href="{url}">{esc(name)}</a>' if url else esc(name))
        return f'<p class="seealso">See also: {", ".join(rendered)}</p>' if rendered else ""

    def _render_deprecated(self, header, lines):
        text = header[len("Deprecated"):].lstrip(": ")
        paragraphs = self._render_paragraphs(([text] if text else []) + lines)
        return f'<div class="deprecated"><p class="admonition-title">Deprecated</p>{paragraphs}</div>'


def parse_blocks(lines, base_indent):
    """Split help-comment lines into an ordered list of (kind, header, content lines) blocks.

    A line at the base indentation that matches HEADER_RE starts a section; its content is every following line
    indented deeper than the base. Everything else is paragraph text.
    """
    blocks = []
    paragraph = []

    def flush():
        if paragraph:
            blocks.append(("para", None, paragraph[:]))
            paragraph.clear()

    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.strip()
        if not stripped:
            paragraph.append("")
            i += 1
            continue
        match = HEADER_RE.fullmatch(stripped) if indent_of(line) <= base_indent else None
        if match is None:
            paragraph.append(line)
            i += 1
            continue
        flush()
        content = []
        i += 1
        while i < len(lines):
            next_line = lines[i]
            if next_line.strip() and indent_of(next_line) <= base_indent:
                break
            content.append(next_line)
            i += 1
        blocks.append((match.lastgroup, stripped, content))
    flush()
    return blocks


class SiteRenderer:
    def __init__(self, model, output_dir, ice_version, base_url):
        self.model = model
        self.output_dir = output_dir
        self.ice_version = ice_version
        self.base_url = base_url
        self.pages = []

    def write_page(self, file_name, title, main):
        canonical = f'<link rel="canonical" href="{self.base_url}{file_name}">\n' if self.base_url else ""
        text = PAGE_TEMPLATE.format(title=esc(title), canonical=canonical, version=esc(self.ice_version), main=main)
        (self.output_dir / file_name).write_text(text, encoding="utf-8")
        self.pages.append(file_name)

    def render_site(self):
        self.output_dir.mkdir(parents=True, exist_ok=True)
        for cls in self.model.classes.values():
            self.render_class(cls)
        for function in self.model.functions.values():
            self.render_function(function)
        self.render_index()
        self.render_sitemap()

    def render_class(self, cls):
        name = cls["name"]
        renderer = HelpRenderer(self.model, context_class=name)
        parts = [f"<h1>{esc(name)}{self._badges(cls)}</h1>"]
        if cls["superclasses"]:
            supers = ", ".join(self._class_link(super_name) for super_name in cls["superclasses"])
            parts.append(f'<p class="superclasses">Superclasses: {supers}</p>')
        parts.append(renderer.render(cls["help"], name, heading_level=2))

        if cls["enumerationMembers"]:
            rows = []
            for member in cls["enumerationMembers"]:
                description = renderer.render(member["help"], member["name"], heading_level=6)
                rows.append(
                    f'<tr id="{member["name"]}"><td><code>{esc(member["name"])}</code></td>'
                    f'<td>{member["value"]:g}</td><td>{description}</td></tr>'
                )
            parts.append("<h2>Enumeration Members</h2>")
            parts.append(
                '<table class="enum"><thead><tr><th>Member</th><th>Value</th><th>Description</th></tr></thead>'
                f'<tbody>{"".join(rows)}</tbody></table>'
            )

        constructor = cls.get("constructor")
        if constructor and constructor["help"]:
            parts.append("<h2>Constructor</h2>")
            parts.append(self._render_member(renderer, constructor, static=False))

        own_properties = [p for p in cls["properties"] if p["definingClass"] == name]
        if own_properties:
            parts.append("<h2>Property Details</h2>")
            for prop in sorted(own_properties, key=lambda p: p["name"].lower()):
                parts.append(self._render_property(renderer, prop))

        own_methods = [m for m in cls["methods"] if m["definingClass"] == name]
        if own_methods:
            parts.append("<h2>Method Details</h2>")
            for method in sorted(own_methods, key=lambda m: m["name"].lower()):
                parts.append(self._render_member(renderer, method, static=method["static"]))

        parts.append(self._render_inherited(cls, "methods", "Methods"))
        parts.append(self._render_inherited(cls, "properties", "Properties"))

        self.write_page(f"{name}.html", f"{name} - Ice for MATLAB API Reference", "\n".join(p for p in parts if p))

    def _badges(self, cls):
        badges = []
        if cls["kind"] == "enum":
            badges.append("Enumeration")
        if cls["abstract"]:
            badges.append("Abstract")
        return "".join(f' <span class="badge">{badge}</span>' for badge in badges)

    def _class_link(self, name):
        page = self.model.page(name)
        return f'<a href="{page}">{esc(name)}</a>' if page else f"<code>{esc(name)}</code>"

    def _render_member(self, renderer, member, static):
        badge = ' <span class="badge">Static</span>' if static else ""
        body = renderer.render(member["help"], member["name"], heading_level=4)
        return f'<section class="member" id="{member["name"]}"><h3>{esc(member["name"])}{badge}</h3>{body}</section>'

    def _render_property(self, renderer, prop):
        badges = []
        if prop["constant"]:
            badges.append("Constant")
        if prop["dependent"]:
            badges.append("Dependent")
        if prop["setAccess"] != "public" and not prop["constant"]:
            badges.append("Read-only")
        rendered = "".join(f' <span class="badge">{badge}</span>' for badge in badges)
        body = renderer.render(prop["help"], prop["name"], heading_level=4)
        return f'<section class="member" id="{prop["name"]}"><h3>{esc(prop["name"])}{rendered}</h3>{body}</section>'

    def _render_inherited(self, cls, group, label):
        by_class = {}
        for member in cls[group]:
            defining = member["definingClass"]
            if defining != cls["name"] and defining in self.model.classes:
                by_class.setdefault(defining, []).append(member["name"])
        if not by_class:
            return ""
        parts = [f"<h2>Inherited {label}</h2>"]
        for defining in sorted(by_class):
            links = ", ".join(
                f'<a href="{defining}.html#{member_name}">{esc(member_name)}</a>'
                for member_name in sorted(by_class[defining], key=str.lower)
            )
            parts.append(f'<p class="inherited">From {self._class_link(defining)}: {links}</p>')
        return "\n".join(parts)

    def render_function(self, function):
        name = function["name"]
        renderer = HelpRenderer(self.model, context_class=name)
        parts = [f"<h1>{esc(name)}</h1>"]
        if function["declaration"]:
            parts.append(f'<pre class="syntax">{esc(function["declaration"])}</pre>')
        parts.append(renderer.render(function["help"], name, heading_level=2))
        self.write_page(f"{name}.html", f"{name} - Ice for MATLAB API Reference", "\n".join(parts))

    def render_index(self):
        renderer = HelpRenderer(self.model)
        parts = [f"<h1>Ice for MATLAB API Reference</h1>"]
        for package in self.model.api["packages"]:
            parts.append(f'<h2>{esc(package["name"])}</h2>')
            groups = {}
            for cls in package["classes"]:
                groups.setdefault(self.model.category(cls), []).append(cls)
            for group_name in ("Classes", "Proxies", "Exceptions", "Enumerations"):
                if group_name not in groups:
                    continue
                parts.append(f"<h3>{group_name}</h3>")
                items = []
                for cls in sorted(groups[group_name], key=lambda c: c["name"]):
                    summary = renderer.summary(cls["help"], cls["name"])
                    items.append(f'<li><a href="{cls["name"]}.html">{esc(cls["name"])}</a> &mdash; {summary}</li>')
                parts.append(f'<ul class="members">{"".join(items)}</ul>')
            if package["functions"]:
                parts.append("<h3>Functions</h3>")
                items = []
                for function in sorted(package["functions"], key=lambda f: f["name"]):
                    summary = renderer.summary(function["help"], function["name"])
                    items.append(
                        f'<li><a href="{function["name"]}.html">{esc(function["name"])}</a> &mdash; {summary}</li>'
                    )
                parts.append(f'<ul class="members">{"".join(items)}</ul>')
        if self.model.api["functions"]:
            parts.append("<h2>Functions</h2>")
            items = []
            for function in sorted(self.model.api["functions"], key=lambda f: f["name"]):
                summary = renderer.summary(function["help"], function["name"])
                items.append(f'<li><a href="{function["name"]}.html">{esc(function["name"])}</a> &mdash; {summary}</li>')
            parts.append(f'<ul class="members">{"".join(items)}</ul>')
        self.write_page("index.html", "Ice for MATLAB API Reference", "\n".join(parts))

    def render_sitemap(self):
        if not self.base_url:
            return
        urls = "\n".join(f"  <url><loc>{self.base_url}{page}</loc></url>" for page in sorted(self.pages))
        text = (
            '<?xml version="1.0" encoding="UTF-8"?>\n'
            '<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">\n'
            f"{urls}\n</urlset>\n"
        )
        (self.output_dir / "sitemap.xml").write_text(text, encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    parser.add_argument("--input", required=True, type=Path, help="path to matlab-api.json")
    parser.add_argument("--output", required=True, type=Path, help="output directory for the static site")
    parser.add_argument("--ice-version", required=True, help="Ice version displayed in the page header")
    parser.add_argument("--base-url", default="", help="canonical URL prefix, for example https://.../api/matlab/")
    args = parser.parse_args()

    if args.base_url and not args.base_url.endswith("/"):
        args.base_url += "/"

    api = json.loads(args.input.read_text(encoding="utf-8"))
    if api["schemaVersion"] != 1:
        sys.exit(f"unsupported schema version {api['schemaVersion']}")

    model = Model(api)
    site = SiteRenderer(model, args.output, args.ice_version, args.base_url)
    site.render_site()

    assets_dir = Path(__file__).parent / "assets"
    (args.output / "css").mkdir(exist_ok=True)
    shutil.copyfile(assets_dir / "style.css", args.output / "css" / "style.css")
    shutil.copyfile(args.input, args.output / "matlab-api.json")

    print(f"Rendered {len(site.pages)} pages into {args.output}")


if __name__ == "__main__":
    main()
