```markdown
# v10S — Release Notes

**Release date:** September 2026
**Base:** v10S
**License:** GPL-3.0

---

## What is this?

v10S is a **Minecraft-style voxel game layer** built on top of v10S, the hardened stable release of the Miniature Physics Engine. The game layer is fully additive — no v10S engine physics code is modified. All new code lives in `src/game/`.

---

## What's Included

### Game Layer (`src/game/`)
- **Block registry** (`block.h/.c`) — 15 block types with per-type physics properties (friction, restitution, solidity, transparency)
- **Chunk system** (`chunk.h/.c`) — 16x16x16 chunks with dirty-flag rebuild, GPU mesh handles
- **World grid** (`world.h/.c`) — 8x4x8 chunk grid (128x64x128 blocks), Perlin noise terrain generation
- **DDA voxel raycast** (`raycast.h/.c`) — fast block-type-aware ray intersection, returns hit position + face normal
- **Chunk meshing** (`block_render.h/.c`) — GPU-instanced face culling, own GLSL Phong shader (ambient + diffuse + sun-position)
- **Game init/glue** (`game_init.h/.c`) — world generation, block placement/destruction, player-voxel collision, rigidbody-voxel collision

### Engine Hooks (minimal, additive only)
- `simulation.c` — left/right click block interaction, player collision per tick, rigidbody-voxel collision per substep
- `root_gtk.c` — `game_init()` call at startup, scroll signal for block cycling
- `new_render.c` — `block_render_world()` call after scene render, sky-blue clear, sun-position light
- `input_control.c` — scroll handler, `on_scroll()` declaration

### Validation
- V01: ASan+UBSan build — **PASS** (0 warnings, 0 errors)
- V02: Clean release build — **PASS** (0 warnings, 221KB binary)
- V03: Headless physics stress (50 spheres, 600 ticks) — **PASS** (0 NaN, 0 fallen, terrain intact)

---

## Controls (New in v10S)

| Action | Input |
|---|---|
| Destroy block | Left-click |
| Place block | Right-click |
| Cycle block type | Scroll wheel |

---

## Known Limitations

- Block inventory menu not yet ported from old v10S
- Spawn gun mechanics pending
- Water and Glass blocks do not yet have transparency rendering
- Block placement uses default block type only

---

*v10S is free software, licensed under the GNU GPL v3. See [LICENSE](LICENSE).*
```
