# ckit

A collection of lightweight, header-only C libraries. Drop a `.h` file into your project, `#include` it, and go — no build system, no dependencies, no setup.

## Usage

Each library is self-contained in a single header file. To use one, just copy it into your project:

```c
#include "window.h"
```

## Examples

Build and run an example with:

```bash
make example NAME=<example>
```

**Running `examples/balls.c`:**

```bash
make example NAME=balls
```

## License

MIT
