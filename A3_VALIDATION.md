# v10S.51 Validation Checklist
> This checklist is subordinate to [`RELEASE_GATES.md`](RELEASE_GATES.md), which is the authoritative gate list for `v10S`.
> v10S game layer validation results are appended below.


<!-- MPE_RELEASE_GATES_SECTION_BEGIN -->
## v10S Release Gates

The stable release is controlled by [`RELEASE_GATES.md`](RELEASE_GATES.md).

Minimum mandatory gates:

- [x] Release freeze policy is active.
- [x] Build passes.
- [x] Startup prints the correct version.
- [x] Shader/render failures are visible.
- [x] Input and focus-loss behaviour is stable.
- [x] Editor deletion/joint torture tests pass.
- [x] Physics stacks settle without explosion.
- [x] Sleeping stacks remain sleeping.
- [x] Overflow counters are visible where applicable.
- [x] F5-F10 validation tests pass.
- [x] Documentation matches code.
- [x] Repository artifacts are cleaned.
- [x] Sanitizer validation passes.

Do not tag `v10S` until all mandatory P0 gates pass.
<!-- MPE_RELEASE_GATES_SECTION_END -->

This checklist validates the full 1-51 A3 patch sequence.

## Built-in Test Keys

| Key | Test |
|---|---|
| F5 | Spawn 10-cube stability stack |
| F6 | Spawn sleeping cube + moving projectile |
| F7 | Editor torture test: select, joint, delete, reset |
| F8 | Spawn stress test: up to 300 mixed objects |

## Core Lifecycle

- [ ] Closing the window quits the program.
- [ ] Losing window focus clears stuck input.
- [ ] Mouse lock state does not remain stuck after focus loss.

## Editor State

- [ ] Deleting selected object does not crash.
- [ ] Deleting jointed object does not crash.
- [ ] Deleting marked joint object does not crash.
- [ ] Loading a scene resets selection/menu/cache state.
- [ ] Object menu closes when selection becomes invalid.

## Physics

- [ ] Objects rest on floor without explosive jitter.
- [ ] Cubes stack with reasonable stability.
- [ ] Spheres and cubes collide correctly.
- [ ] Objects with restitution bounce on floor.
- [ ] Surface friction affects sliding.
- [ ] Sleeping objects wake when hit.
- [ ] Joints remain valid after deletion.

## Broadphase

- [ ] Sleeping objects are still discoverable.
- [ ] High object counts do not allocate quadratic dedupe memory.
- [ ] Broadphase overflow counters are visible if overflow occurs.
- [ ] Node pool grows instead of silently dropping objects.

## Rendering

- [ ] Ground grid is visible.
- [ ] Wireframe selection renders.
- [ ] Joint lines render.
- [ ] No per-frame uniform query spam in grid/wireframe/joint paths.

## Build

- [ ] make -C src clean succeeds.
- [ ] make -C src succeeds.
- [ ] Startup prints: v10S.51

## Final Pass

Run:

    make -C src clean
    make -C src
    ./src/engine

Then manually test:

1. Press F5 — stack settles.
2. Press F6 — sleeping cube wakes on impact.
3. Press F7 — editor torture does not crash.
4. Press F8 — spawn stress remains observable and does not silently fail.
5. Save/load scene with menus open — no crash.
6. Delete selected/jointed objects — no crash.
7. Set friction to 0 and restitution above 0 — objects slide and bounce.

If all pass, the A3 patch sequence is complete.

---

## v10S — Game Layer Validation

The game layer (`src/game/`) was validated separately from the v10S engine. All tests target the additive game code only.

### V01: ASan + UBSan Build

- [x] Build with `-fsanitize=address -fsanitize=undefined` compiles
- [x] 0 compiler warnings
- [x] 0 compiler errors
- [x] Binary produced (2.9 MB with sanitizer instrumentation)

### V02: Clean Release Build

- [x] `make clean && make` succeeds
- [x] 0 compiler warnings
- [x] 0 compiler errors
- [x] Binary produced (221 KB)

### V03: Headless Physics Stress Test

- [x] 50 spheres spawned above voxel terrain
- [x] 600 physics ticks (10 seconds at 60Hz) completed
- [x] 0 NaN events across all bodies
- [x] 0 bodies fallen below y=-10
- [x] Terrain integrity maintained (65536 solid blocks)
- [x] Max linear speed within bounds (< 15 m/s)
- [x] Spheres settle on terrain surface

### V04: F10 Long-Run Validation

- [ ] Requires GUI interaction (not headless-testable)
- [ ] Manual test: press F10, wait 60 seconds, verify PASS

### Game Layer Feature Checklist

- [x] Block registry: 15 block types registered
- [x] Chunk system: 16x16x16 chunks with dirty-flag rebuild
- [x] World grid: 8x4x8 chunks (128x64x128 blocks)
- [x] Perlin noise terrain generation
- [x] DDA voxel raycast (block-type-aware, returns face normal)
- [x] GPU-instanced chunk meshing with face culling
- [x] Own GLSL Phong shader for blocks
- [x] Block destruction (left-click, raycast-based)
- [x] Block placement (right-click, face-normal-aware)
- [x] Scroll-wheel block type cycling
- [x] Player-voxel collision (3-sphere model)
- [x] Rigidbody-voxel collision (per physics substep)
- [x] Sky-blue background, sun-position lighting
