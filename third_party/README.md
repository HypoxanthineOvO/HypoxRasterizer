# Third-Party Headers

This directory contains header-only dependencies that are used directly by the
xmake targets:

- `nlohmann/`: JSON for Modern C++.
- `stb_image.h` and `stb_image_write.h`: stb image load/write headers.

They are vendored here so the project can build without relying on an online
xmake package fetch for these header-only libraries. The remaining xmake-managed
dependencies are still declared in `xmake.lua`.
