# Scene Config V2

`configs/SmokeTest.v2.json` is the first V2 scene-config example. V2 is the
long-term schema direction; existing legacy configs remain usable during the
migration.

## Top-Level Shape

```json
{
  "scene": {"name": "smoke_v2"},
  "camera": {},
  "materials": [],
  "objects": [],
  "lights": [],
  "outputs": {}
}
```

## Required Sections

- `scene.name`: output scene name, used by `outputs/<scene-name>/`.
- `camera`: `resolution`, `position`, `target`, `fov`, optional `focalLength`.
- `materials`: material records with `id`, `type`, and material parameters.
- `objects`: mesh instances with `mesh`, `material`, and transform fields.
- `lights`: light records. The temporary V2 render adapter currently supports
  `point`, `directional`, and `area`.
- `outputs`: `root` and selected `buffers`.

## CLI Examples

Render a legacy config:

```sh
xmake run HypoxRasterizer --config configs/SmokeTest.json --output-root outputs
```

Render the V2 smoke config through the temporary legacy adapter:

```sh
xmake run HypoxRasterizer --config configs/SmokeTest.v2.json --output-root outputs --buffers color,gbuffer,shadow
```

Render the directional-light smoke config:

```sh
xmake run HypoxRasterizer --config configs/DirectionalSmoke.v2.json --output-root outputs --buffers color,gbuffer,shadow
```

Render the Area VPL smoke config:

```sh
xmake run HypoxRasterizer --config configs/AreaVPLSmoke.v2.json --output-root outputs --buffers color,gbuffer,shadow
```

Override resolution:

```sh
xmake run HypoxRasterizer --config configs/SmokeTest.json --resolution 64x32
```

## Migration Notes

- Legacy `Camera` maps to V2 `camera`.
- Legacy `Materials[].Name` maps to V2 `materials[].id`.
- Legacy `Objects[].SourceFile` maps to V2 `objects[].mesh`.
- Legacy point lights map to V2 `lights[].type = "point"`.
- Legacy directional lights map to V2 `lights[].type = "directional"` with a
  `direction` vector and `intensity`.
- Legacy area lights map to V2 `lights[].type = "area"` with `position`,
  `normal`, `size`, `intensity`, and optional `vpl`.
- `lights[].vpl.samples` controls the number of direct VPL samples.
- `lights[].vpl.seed` and `lights[].vpl.stochastic` enable reproducible jittered
  VPL placement; deterministic grid sampling is used when `stochastic` is false.
- `lights[].indirectVPL` reserves the first-bounce indirect-light sampling
  contract. It is disabled by default and supports `enabled`, `samples`, `seed`,
  `bounces`, `visibility`, `clamp`, `radius`, and `sourceLights`.

The V2-to-render path is intentionally temporary in M2. Later milestones should
move rendering code to consume the new scene model directly.

## Output Notes

- `gbuffer/*.f32` files are authoritative float buffers; PNG files are previews.
- `shadow/PointlightShadowMap_<index>_face_<axis>.png` stores point cube shadow
  faces.
- `shadow/DirectionalLightShadowMap_<index>.png` stores directional shadow maps.
- `shadow/light_<index>_direct_vpls.f32` stores VPL debug data as
  `[position.xyz, intensity.rgb]` records.
- `shadow/light_<index>_indirect_vpls.f32` is reserved for IndirectVPL debug
  data as `[position.xyz, normal.xyz, flux.rgb, radius, bounce, sourceLight,
  flags]` records.
