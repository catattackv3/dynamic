# Predictive Input Buffer — edited community recreation

This is a community recreation, not the original author's source or a verified byte-for-byte equivalent.

## Changes in this version
- F6 cycles through **1, 2, 3, ... 10 ticks, then OFF**.
- Default remains **2 ticks** to preserve the recreation's previous default.
- A top-left in-level indicator shows `INPUT BUFFER: ON | N TICKS` or `INPUT BUFFER: OFF`.
- Normal and wave inputs use the selected tick count.
- Resetting a level clears queued inputs.

A simulation tick is 1/240 second in this recreation. At 60 FPS, 4 ticks are approximately one rendered frame. So 10 ticks are approximately 2.5 frames (41.7 ms), not 10 rendered frames.

## Build a `.geode` file
The current environment does not have the Geode SDK/CLI installed, so this ZIP contains edited source rather than a compiled `.geode` package.

### Easiest route: GitHub Actions
1. Create a new GitHub repository and upload the **contents** of this ZIP.
2. Before building, edit `mod.json`: replace `tuusuario` in the `id` and `TuUsuario` in `developer` with your own Geode developer ID/name.
3. Open **Actions** and run **Build Geode Mod** (or push a commit to trigger it).
4. Download the `Build Output` artifact from the completed workflow. It should contain the `.geode` file.
5. Back up your existing mod, then install the test build into `Geometry Dash/geode/mods/`.

### Local build (Windows)
Install the Geode SDK and CLI for the target Geometry Dash version, then run `geode build` from this folder.

## Important testing note
This is a recreation of observed behavior, not the original author's source. It has not been compiled or tested in-game in this environment; Geode API/signature adjustments may be needed during the first build. Test in practice mode first and keep the original `.geode` backup.
