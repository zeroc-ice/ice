# Repository Guidelines for AI Coding Agents

These notes apply to all AI coding assistants (Copilot, Claude Code, etc.) working in this repository.

## Dismissed audit patterns

This section captures the reasoning behind `ai-audit` findings that have been closed as "not planned". Before opening a
new audit finding (or reporting an existing concern), check whether it falls under one of these patterns — the
rationale has already been considered and rejected. Cite the canonical issue when relevant.

For dismissals newer than this section, also consult:
`gh issue list --repo zeroc-ice/ice --label ai-audit --state closed --search "reason:not-planned"`.

### 1. Slice loader cache pins generated-code assemblies (C#)

`AssemblySliceLoaderFactory` (csharp/src/Ice/Internal/AssemblySliceLoader.cs) caches the Slice loader it builds for
each assembly marked with `SliceAttribute` — and, through its recursive merge, for every marked assembly it
references — in a process-wide cache holding strong references. Unloading such assemblies (e.g. with a collectible
`AssemblyLoadContext`) is unsupported: the cache pins them for the lifetime of the process. Assemblies without the
attribute are never cached. Don't file findings proposing weak-key caching or unload-safety for this cache. Same
disposition as icerpc/icerpc-csharp#4827. (#6658.)
