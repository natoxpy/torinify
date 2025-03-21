# 🚧 TORINIFY 🚧

A feature rich music player, metadata editor, music management system, and open protocol music distribution platform.

## Build

### Requirements

You must have `SQLITE3`, `FFMPEG`, and `TAGLIB` static libraries.

### Commands

```bash
cmake ..
make
```

This will give you a `test`, `main`, and `torinify` (static library).
Test are the unit tests, and main is a basic music player which uses torinify used for manual integration testing.
