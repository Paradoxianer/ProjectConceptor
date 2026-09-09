# ProjectConceptor

Modular graph editor for Haiku and BeOS: view, edit and process
information that can be represented as a graph - nodes with
attributes, connections, groups, and automatic layout. MIT licensed,
plugin-based (editors, commands, translators).

![Overview](docs/help/screenshots/02-overview.png)

A group renders as one tight, connected shape around its children:

![Group](docs/help/screenshots/05-group.png)

## Getting started

See the [user guide](docs/help/user-guide.md) for a walkthrough of
nodes, attributes, connections, grouping and automatic layout.

## Building

Built with Haiku's own `make`-based build system: `cd src && make`
builds the app, its plugins, and the translators. `src/tests/` has its
own `makefile` for a CppUnit suite covering the core classes (commands,
indexing, group geometry) - `cd src/tests && make && ./ProjectConceptorTests`.
