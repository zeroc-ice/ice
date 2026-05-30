# Repository Guidelines for AI Coding Agents — Security Audits

These conventions apply to all AI coding assistants (Claude Code, Codex, Copilot, etc.) performing
**security audits** of this repository. They cover how an audit is run, how findings are verified,
where issues are filed, and which findings have already been considered and dismissed.

## The two repositories

| Repo | Visibility | Role |
|------|-----------|------|
| [`zeroc-ice/ice`](https://github.com/zeroc-ice/ice) | **public** | The Ice source. Fixes land here as normal PRs. |
| [`zeroc-ice/ice-audit`](https://github.com/zeroc-ice/ice-audit) | **private** | Audit reports and the finding tracker. |

Prior audit reports live at the root of `ice-audit`, e.g.
`SECURITY_AUDIT-CODEX-2026-05-20.md` (the initial pass) and `SECURITY_AUDIT_VERIFIED-2026-05-21.md`
(the verification pass). Read the most recent pair before starting a new audit — they establish the
finding numbering, severity calibration, and citation style.

## Audit workflow — always two independent passes

Every audit is double-checked: one tool produces an initial report, a second independently verifies
it. The default ordering (and the one the existing reports follow) is **Codex first, Claude
verifies**:

1. **Initial report (Codex).** Run Codex over the audit scope (typically `cpp/` — Ice core,
   SSL/transports, Glacier2, IceGrid, IceBox, IceStorm, DataStorm, the Slice compilers, IceUtil,
   IceDB) and have it produce a static-review report. Save it to `ice-audit` as
   `SECURITY_AUDIT-<TOOL>-<YYYY-MM-DD>.md`, where `<TOOL>` names the tool that produced the initial
   pass (`CODEX` for the default ordering, `CLAUDE` for a Claude-first run). Findings are deduplicated
   and ordered by severity, each with file/line citations, impact, and a proposed fix.

2. **Verification pass (Claude).** Independently re-derive **every** finding against the current
   source. For each one record a verdict — **CONFIRMED**, **PARTIAL**, **FALSE POSITIVE**, or
   **REGRADE** (confirmed but severity changed) — re-locate stale file/line citations, recalibrate
   severity, and add any **new findings** the first pass missed (number them `NF-1`, `NF-2`, …).
   A new finding is single-pass by definition: re-derive it from scratch against the source a second
   time before treating it as verified — a fresh independent derivation by the same verifier is
   acceptable here, since routing it back through the other tool for a full third pass is usually
   impractical mid-run. Until it clears that second derivation, tag it `UNVERIFIED` (a status that
   applies only to verifier-discovered `NF-*` findings, not to the four verdicts above) and do not
   file it. Save as `SECURITY_AUDIT_VERIFIED-<YYYY-MM-DD>.md`. Where the two
   reports disagree, the verification pass supersedes the initial one — state this in both documents'
   headers and cross-link them.

The reverse ordering (Claude first, Codex verifies) is acceptable when it fits the run, but the two
independent passes are non-negotiable: **never file issues from a single unverified pass.**

Pin every citation to a specific commit so line numbers stay stable, e.g.
`https://github.com/zeroc-ice/ice/blob/<full-sha>/cpp/src/Ice/InputStream.cpp#L431-L449`. Pin to a
**pushed** commit — links to a local/unpushed commit 404. Record the audited branch and commit in
the report header, along with the tool and the exact model name and version that produced each pass
(e.g. "Initial: Codex / GPT-5.x"; "Verification: Claude / Opus 4.x"), so it is unambiguous which
model version performed the audit.

## Filing issues — one issue per confirmed finding

Only after the report is verified, open one issue per finding that survived verification: every
finding the verification pass marked **CONFIRMED** or **REGRADE** (at its recalibrated severity), and
the confirmed portion of every **PARTIAL** finding. Skip findings marked **FALSE POSITIVE** or
**UNVERIFIED**. For a **PARTIAL** finding, file only the portion that was confirmed, scoped and worded
to that portion, and note the unconfirmed remainder in the body rather than filing it as fact.

### Where to file (routing)

- **Security-sensitive `Critical`/`High`/`Medium` findings → `zeroc-ice/ice-audit` (private).** A
  genuinely exploitable or otherwise non-public vulnerability rated **Medium or above** — remote
  memory-safety bugs, auth bypasses, pre-auth DoS, anything you would not want disclosed before a fix
  ships — goes in the **private** audit repo so it is not publicly disclosed prematurely.
- **Everything else → `zeroc-ice/ice` (public).** Lower-severity findings (`Low`/`Info`), benign
  hardening, and anything with no disclosure risk go to the public repo where they are fixed in the
  open.

When in doubt, treat a finding as security-sensitive and file it privately in `ice-audit`: under-
filing a real vulnerability into the public repo is the costly mistake, since an early public
disclosure cannot be undone, whereas a finding filed privately can later be re-filed or transferred
to the public repo once it is cleared.

### Before filing — avoid recreating dismissed findings

Check both repos for an existing or already-dismissed issue covering the same thing. **Do not
re-file findings that have been closed as "Not planned" unless you have new evidence** — their
rationale has already been considered and rejected (see "Dismissed audit patterns" below), so by
default leave them closed; the narrow exception for reopening with new evidence is described at the
end of this section.

```bash
# Already-dismissed findings — the "Not planned" milestone (ice-audit only) plus not-planned closures in both repos:
gh issue list --repo zeroc-ice/ice-audit --state closed --milestone "Not planned"
gh issue list --repo zeroc-ice/ice-audit --label ai-audit --state all --search "is:closed reason:not-planned"
gh issue list --repo zeroc-ice/ice       --label ai-audit --state all --search "is:closed reason:not-planned"
# Open/duplicate check in both repos:
gh issue list --repo zeroc-ice/ice-audit --label ai-audit --state all
gh issue list --repo zeroc-ice/ice       --label ai-audit --state all
```

If a new finding genuinely differs from a dismissed one, say how in the body and cite the prior
issue. Reopen a dismissed finding only with new evidence (an actual victim, a measurable impact, a
broken assumption in the original rationale).

### Issue conventions

- **Title:** `[Audit-<Severity>] <concise description>` — severity is one of `Critical`, `High`,
  `Medium`, `Low`, `Info` (the verified severity, not the initial one).
- **Labels:** `ai-audit` on **every** audit issue (this is what the dedup/dismissal queries above
  rely on), plus the labels appropriate to the finding — at minimum the ones it carries in the report
  (e.g. `security` for a security-sensitive finding, plus any component/area labels). Apply
  `security` to security-sensitive findings, not to benign hardening or informational ones.
- **Milestone:** assign an **existing** milestone from the target repo (list them with
  `gh api repos/zeroc-ice/<repo>/milestones --jq '.[].title'`; do not invent a version) for the
  release the fix should land in; use `Not planned` only when closing without action.
- **Body** opens with the AI-generated banner, then the standard sections:

  ```markdown
  <!-- Fill in the actual tools and the exact model names/versions used for each pass before filing. -->
  > **AI-generated audit finding** — this issue was opened from an automated two-pass security audit.
  > Initial pass: **<tool>** / **<model name>** (`<model-id>`); verification pass: **<tool>** /
  > **<model name>** (`<model-id>`). It **has not been triaged by a human yet**; verify the reasoning,
  > reproducibility, and severity before acting on it.

  ## Summary
  ## The bug
  ## Impact
  ## Source references   <!-- commit-pinned blob links into zeroc-ice/ice -->
  ## Recommendation
  ## Severity
  ```

## Dismissed audit patterns

The reasoning behind `ai-audit` findings closed as "Not planned" is captured here. Before opening a
new finding, check whether it falls under one of these patterns — the rationale has already been
considered and rejected. Cite the canonical issue when relevant. For dismissals newer than this
section, also consult the `gh` commands above.

### 1. False positives — the code is already correct

Findings where the cited code already does the right thing on closer reading:

- **Flex scanner buffer arithmetic** — the `YY_INPUT` buffer is allocated with 2 bytes of slack, so
  the `strcat`/length writes fit; not an off-by-one. (`ice-audit#48`)
- **bzip2 / decompression amplification** — `uncompressedSize` *is* validated against
  `Ice.MessageSizeMax` before `resize()`. "N connections × per-conn limit" is inherent to any
  per-connection limit, not a defect. (`ice-audit#47`)
- **Request context-map amplification** — bounded by `Ice.MessageSizeMax`; the read is incremental
  and non-preallocating, so `readAndCheckSeqSize` does not meaningfully apply. Special-casing the
  context map would be inconsistent with every other dictionary in Ice. (`ice-audit#8`)
- **Glacier2 context stripping** — the reserved `_con.*` connection keys *are* stripped before any
  `PermissionsVerifier` call; only generic application context is forwarded, which is intentional
  (`Glacier2.AddConnectionContext`). (`ice-audit#46`)

### 2. Opt-in trace/log verbosity is the operator's responsibility

Ice's tracing is opt-in, and an operator who raises a trace level does so precisely to see internal
detail. Findings asking Ice to suppress or mask data already gated behind a non-default trace level
are declined: the disclosure reaches only whoever already has access to the server's logs, clients
always receive a generic error, and pattern-based masking gives false confidence (real secrets live
in fields that don't match the pattern). Putting secrets in custom-verifier exception text or
relying on env dumps is the operator's/author's responsibility, not Ice's.

- Activator trace level 2 logging full process environment / args. (`ice-audit#36`)
- `PermissionsVerifier` exception strings logged verbatim at non-zero trace levels. (`ice-audit#35`)

### 3. Pick the secure transport instead of bolting auth onto the insecure one

Ice pairs each plaintext transport with a secure variant (`tcp`/`ssl`, `bt`/`bts`,
`ws`/`wss`). Operators whose trust model requires authentication and confidentiality use the secure
variant and get X.509 mutual auth + TLS. Findings that propose adding link-layer auth or a new
"require security" property to the *plaintext* transport are declined — the answer is the same one we
give for plain `tcp`: use the secure endpoint.

- IceBT plain `bt` not requiring BlueZ pairing/encryption (use `bts`). (`ice-audit#22`)
- UDP server updating the peer address per datagram — UDP is documented as insecure; a
  pin-peer-address property would break multicast/many-to-one and wouldn't stop spoofing. Use SSL.
  (`ice-audit#23`)

### 4. Administrative configuration already implies code execution

An IceGrid admin who can edit a server descriptor's `envs` can also set `exe`, `args`, `pwd`, and
`user`. IceGrid's authorization model is binary admin/no-admin, so loader-injection via env vars
(`LD_PRELOAD`, `DYLD_*`, etc.) is fully dominated by simply setting `exe` to an attacker binary — it
adds no capability. Env-var allow/blocklists are also unwinnable (the sensitive-variable set grows
with every runtime and OS release) and break legitimate uses (`LD_LIBRARY_PATH`, `PYTHONPATH`,
`NODE_OPTIONS`, …). Multi-tenant isolation belongs at the registry admin boundary, not at descriptor
fields. (`ice-audit#26`)

### 5. Defaults whose change is a behavior break with little real-world benefit

`IceSSL.RevocationCheck` defaulting to off: many deployments (including Ice's own demos/tests) use a
CA that publishes no OCSP/CRL endpoints, so enabling revocation by default would break handshakes;
short-lived ACME certs largely supersede revocation; and the hard-fail behavior the finding asked
for is *already* implemented across all three engines once revocation is enabled. Operators who need
it opt in. (`ice-audit#41`)

### 6. `std::random_device` for non-secret WebSocket nonce/mask

`Sec-WebSocket-Key` is a handshake nonce (not a secret per RFC 6455) and the frame mask defends a
browser-cache-poisoning threat model that doesn't apply to Ice's native transport. On every
supported toolchain `std::random_device` resolves to the OS CSPRNG; the one historical exception
(pre-9.2 MinGW) is not a supported Ice build toolchain. Reverting to the 3.7 hand-rolled
`/dev/urandom`/`rand_s` machinery would re-introduce fork/concurrency complexity to close a gap that
manifests on no supported configuration. (`ice-audit#34`)

### 7. Slice compilers are not a deployed attack surface

`assert()` calls in the Slice parser fire on invariants already enforced by the Bison grammar; no
real-world Slice input reaches them. More fundamentally, the Slice compilers are build-time
developer tools, not Ice processes exposed to untrusted runtime input — IDL is authored as part of
the build, like the C++/C# sources next to it. Code-injection / input-validation framing does not
apply. (`ice-audit#39`)
