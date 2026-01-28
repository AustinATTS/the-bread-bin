The Bread Bin
================

The **Bread Bin** is a **local-first, cross-platform workflow and session orchestration system**. It allows you to define, save, share, and execute repeatable working contexts called **LOAFs** (Libraries of Applications and Files).

At its core, The Bread Bin answers one simple question:

> **“What should exist for me to start working now?”**

Not which buttons to click.Not which windows to drag back into place.Not which tabs you forgot to reopen.

Instead, The Bread Bin focuses on **intentional state**, the artefacts, tools, checks, and routines that must exist for meaningful work to begin.

Think of it as a pantry for work contexts: declarative, portable, and always under your control.

What The Bread Bin _Is_
-------------------------

*   A **workflow context system**, not just a launcher

*   A **declarative description** of work states

*   A **portable abstraction** over OS, filesystem, and execution differences

*   A **developer- and power-user–friendly tool** that stays out of the way

*   Fully **local-first**. No accounts, no telemetry, no cloud dependency


What The Bread Bin _Is Not_
------------------------------

The Bread Bin is explicitly **not**:

*   A simple app launcher

*   A window tiling manager

*   A startup script runner

*   A cron / systemd replacement

*   A cloud service or SaaS


It does not monitor users, harvest telemetry, or store data remotely. All data lives where _you_ put it.

Core Concepts
----------------

### LOAFs (Libraries of Applications and Files)

A **LOAF** is a declarative description of a working context.

A LOAF may define:

*   Applications to launch

*   Files or directories to open

*   Documents or websites that must exist

*   Scripts or automations to run

*   Preconditions and validation checks

*   OS- or device-specific behaviour


LOAFs are:

*   Fault-tolerant by default

*   Partially executable

*   Shareable

*   Version-controllable


Each LOAF has:

*   A **display name** (human-readable)

*   A **canonical name** (machine-safe)


### Ingredients

**Ingredients** are the atomic units of a LOAF. Each ingredient represents one intentional requirement or action.

Examples:

*   “Ensure this directory exists and is open”

*   “Ensure this document exists”

*   “Ensure this application is running”

*   “Run this build step”


Ingredients can be:

*   Required or optional

*   Conditional

*   OS-specific

*   Dependent on other ingredients


Failure of one ingredient **does not abort execution**. Partial success is success.

### Recipe Maps

**Recipe Maps** translate logical paths and commands into environment-specific ones.

They enable:

*   Cross-OS path translation

*   Device-specific overrides

*   Logical roots independent of physical mounts


Recipe Maps are global by default, but overridable per LOAF.

Design Principles
--------------------

*   **Declarative First** - describe outcomes, not clicks

*   **Partial Success Is Success** - execution is resilient by design

*   **Local-First Trust Model** - your data, your machine

*   **Extensible Without Lock‑In** - powerful, but never boxed in


No mystery yeast, no hidden ovens.

Architecture Overview
------------------------

The Bread Bin follows a **Controller–Provider architecture**.

### Core Controller (C++)

*   LOAF parsing

*   Validation

*   Execution planning

*   Error handling


### Providers (OS-specific)

*   Filesystem operations

*   Process execution

*   Privilege handling

*   UI integration


Providers declare capabilities; the core adapts dynamically.

Execution Pipeline
---------------------

1.  Validation

2.  Translation

3.  Preflight checks

4.  Execution

5.  Verification

6.  Reporting


Each phase is observable, debuggable, and predictable.

Execution Modes
------------------

### Warm Start

Opens applications and resources **without running scripts**.

### Toasting

Runs defined automation phases such as:

*   Build

*   Sync

*   Test


Toast phases support structured output, timeouts, and OS-specific commands.

### Bread Knife (Cleanup)

Gracefully terminates all processes associated with a LOAF:

*   Best-effort grouping

*   Graceful shutdown first

*   Optional force termination

*   Never touches unrelated processes


Baking (State Capture)
-------------------------

**Baking** is an optional, best-effort feature that attempts to generate LOAFs from observed system state.

*   Captures open editors, browsers, and files

*   Annotates results with confidence metadata

*   Always requires user review


Baking is intentionally non-deterministic and never automatic.

User Interface
------------------

### The Pantry

A searchable, tag-driven gallery of LOAFs.

### The Crust

A background/system access point for fast execution.

### Editors

*   Graphical LOAF editor

*   Raw text editor

*   Theme editor

*   Diffing, validation, and reload-on-apply


Tech Stack (Evolving)
-------------------------

*   Modern C++

*   Immediate-mode tooling for editors

*   Plain-file persistence (no opaque databases)


Platforms
-------------

*   Linux (primary focus)

*   Windows

*   Future-ready for mobile and embedded UIs


Project Status
-----------------

The Bread Bin is **actively under development**.