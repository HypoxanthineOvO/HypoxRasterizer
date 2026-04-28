# M19 IndirectVPL Shading and Visibility Integration

完成时间：2026-04-28 14:46（上海时间）

## Summary

Integrated IndirectVPL sample generation and gather into the renderer, with
debug outputs for indirect samples and per-pixel indirect radiance.

## Changes

- Light objects now store IndirectVPL samples plus visibility/clamp settings.
- Rasterizer initialization generates first-bounce samples when config enables
  `IndirectVPL`.
- Fragment shading gathers indirect radiance and honors visibility/clamp.
- G-Buffer export now includes `indirect_vpl_radiance.f32`.
- Shadow/debug export now writes `light_<index>_indirect_vpls.f32`.
- Extended `IndirectVPLTests`.

## Validation

```sh
xmake build -y IndirectVPLTests
xmake run IndirectVPLTests
```

Result: `10` tests passed.
