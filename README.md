# ProjectConceptor

A graph editor for Haiku and BeOS: capture information as nodes,
attributes and connections, arrange it into groups, and lay the whole
thing out automatically.

![Overview](docs/help/screenshots/02-overview.png)

## Features

**Nodes with custom attributes** - give any node its own boolean or
text attribute rows, each with an inline toggle or edit field.

![Node with attributes](docs/help/screenshots/04-node-with-attributes.png)

**Connections** - drag between two nodes' connection points to link
them; pick a line shape (straight, rounded, angular) and arrowhead
placement per connection from the toolbar.

**Grouping** - select several nodes and group them into one tight,
connected shape that hugs its children automatically, however they're
sized or arranged - including a real connecting corridor when two
children don't line up at all:

![Group](docs/help/screenshots/05-group.png)

**Automatic layout** - rearrange an entire graph with one click; choose
a direction (top-to-bottom, left-to-right, ...) and a layout algorithm
(hierarchical, circular, force-directed, ...).

![Auto-Layout toolbar](docs/help/screenshots/06-autolayout-toolbar.png)

**Plugin architecture** - editors, commands and file-format
translators are all plugins, so the app can be extended without
touching the core. Documents save to a native format; a FreeMind
import/export and a plain-text export are included.

**Full undo/redo** - every change, from creating a node to grouping,
moving, or drawing a connection, can be undone and redone.

## Getting started

See the [user guide](docs/help/user-guide.md) for a full walkthrough,
or the [API reference](Data/API-current/index.html) if you're building
a new plugin.

## Building

Built with Haiku's own `make`-based build system: `cd src && make`
builds the app, its plugins, and the translators. `src/tests/` has its
own `makefile` for a CppUnit suite covering the core classes (commands,
indexing, group geometry) - `cd src/tests && make && ./ProjectConceptorTests`.

## License

MIT.
