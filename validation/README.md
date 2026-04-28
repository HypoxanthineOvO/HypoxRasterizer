# Validation Data

`scenes.yaml` is the top-level validation manifest. It is intentionally small in
M7 and will grow through later milestones.

Tiers:

- `fast`: default daily corner-case and smoke scenes.
- `standard`: texture, classic scene, and medium mesh validation.
- `heavy`: optional benchmark or external scenes; skipped by default.

External assets belong under `assets/external/` and must carry source and
license metadata before they are marked runnable.
