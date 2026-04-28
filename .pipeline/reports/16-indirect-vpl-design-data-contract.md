# M17 IndirectVPL Design and Data Contract

完成时间：2026-04-28 14:36（上海时间）

## Summary

Defined the IndirectVPL configuration, sample, and float-buffer output contract
without changing default render behavior.

## Changes

- Added shared `IndirectVPLConfig`.
- Added V2 `lights[].indirectVPL` parsing and legacy `Lights[].IndirectVPL`
  parsing.
- Added V2-to-legacy adapter preservation for `IndirectVPL`.
- Expanded `IndirectVPL` sample data to position, normal, flux, radius, bounce,
  source light, and flags.
- Added `vpl_debug.hpp` helper for `light_<index>_indirect_vpls.f32`.
- Added `IndirectVPLTests`.
- Updated scene config docs and architecture notes.

## Validation

```sh
xmake build -y IndirectVPLTests
xmake run IndirectVPLTests
xmake run VPLAreaTests
xmake run LightShadowTests
```

Result: `IndirectVPLTests` 4/4 passed; VPL and light/shadow regressions passed.
