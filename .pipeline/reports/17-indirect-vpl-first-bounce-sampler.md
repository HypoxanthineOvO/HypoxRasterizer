# M18 IndirectVPL First-Bounce Sampler

完成时间：2026-04-28 14:44（上海时间）

## Summary

Implemented a deterministic CPU first-bounce sampler for IndirectVPL generation.

## Changes

- Added `indirect_vpl_sampler.hpp/.cpp`.
- Added Moller-Trumbore ray/triangle intersection.
- Added scene first-hit traversal over existing `Object` triangle data.
- Added deterministic light-emission directions for point, area, and
  directional smoke sampling.
- Extended `IndirectVPLTests`.

## Validation

```sh
xmake build -y IndirectVPLTests
xmake run IndirectVPLTests
```

Result: `8` tests passed.
