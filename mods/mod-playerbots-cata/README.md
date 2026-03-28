# mod-playerbots-cata

Firelands worldserver module (see repository `mods/CMakeLists.txt` and `mods/how-to.md`).

## Layout

- `conf/` — `*.conf.dist` merged at configure time into the worldserver config list.
- `data/sql/db-playerbots/` — SQL updates merged into the **playerbots** database (via `UpdateFetcher` + `FC_MODULES_LIST`).
- `src/` — C++ sources compiled into the `mods` static library (or a dynamic mod project if linkage is set to dynamic).

## Database

- **playerbots** (separate MySQL schema): bot metadata, profile JSON, talent drafts, templates — see `data/sql/base/db_playerbots/` in the core tree.
- Configure `PlayerbotsDatabaseInfo` in `worldserver.conf`. Base + updates require core bit `DATABASE_PLAYERBOTS` (16) in `Updates.EnableDatabases` (default 31 includes it).

## Phase 1 skeleton

Registers a `WorldScript` that reloads `Playerbots.*` keys on config load and optionally logs once on startup. Service classes are stubs for account/character/persistence/session/talent storage.

## Next steps

- SQL migrations under `data/sql/db-characters/`.
- `CommandScript` for admin/testing.
- Wire services to `LoginDatabase` / `CharacterDatabase` (or a dedicated pool if core is extended).
