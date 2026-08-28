Pikmin 2: Collect the Fruit!
============================

A WIP mod of Pikmin 2, based on the "Go Here" mod by Drought-Ender.

New Features/Enhancements:

* New "Collect the Fruit!" challenge mode
* Restore Yellow Pikmin bomb rocks
* "Go Here" mod by Drought-Ender, intns, and dannye
* Idle captains auto-pluck Pikmin
* Press A once to Auto-pluck Pikmin
* Press B to cancel Pikmin throw
* Display Pikmin throw trajectory
* Flip carrying ratio (proper fraction)
* Hold Y in Onion menu to increment/decrement by 10
* Squad count increases/decreases as soon as Pikmin is plucked or thrown

Index
-----

- [Dependencies](#dependencies)
  - [Windows](#windows)
  - [macOS](#macos)
  - [Linux](#linux)
- [Building](#building)

Dependencies
------------

### Windows

On Windows, it's **highly recommended** to use native tooling. WSL or msys2 are **not** required.

- Install [Python](https://www.python.org/downloads/) and add it to `%PATH%`.
  - Also available from the [Windows Store](https://apps.microsoft.com/store/detail/python-311/9NRWMJP3717K).
- Download [ninja](https://github.com/ninja-build/ninja/releases) and add it to `%PATH%`.
  - Quick install via pip: `pip install ninja`

### macOS

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages):

  ```sh
  brew install ninja
  ```

- Install [wine-crossover](https://github.com/Gcenx/homebrew-wine):

  ```sh
  brew install --cask --no-quarantine gcenx/wine/wine-crossover
  ```

After OS upgrades, if macOS complains about `Wine Crossover.app` being unverified, you can unquarantine it using:

```sh
sudo xattr -rd com.apple.quarantine '/Applications/Wine Crossover.app'
```

### Linux

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages).
- For non-x86(_64) platforms: Install wine from your package manager.
  - For x86(_64), [wibo](https://github.com/decompals/wibo), a minimal 32-bit Windows binary wrapper, will be automatically downloaded and used.

Building
--------

- Clone the repository:

  ```sh
  git clone https://github.com/dannye/pikmin2-ctf.git
  ```

- Using [Dolphin Emulator](https://dolphin-emu.org/), extract your game's system data to `orig/GPVE01`. (Or `orig/GPVE01_D17` for demo)
  - Right-click the game in Dolphin's game list and select `Properties`.
  - Go to the `Filesystem` tab and right-click `Disc` -> `Extract System Data`.
![Dolphin filesystem extract](assets/dolphin-extract.png)
  - After extraction, the following file should exist: `orig/GPVE01/sys/main.dol`.
  - Copy the contents of `assets/Drought` to your games `files/user/` folder, so the directory: `files/user/Drought` should exist.

- Configure:

  ```sh
  python configure.py --non-matching
  ```

- Build:

  ```sh
  ninja
  ```

Once built with `ninja`, the new DOL will exist at `build/GPVE01/main.dol`. Copy this to your extracted games `sys/` folder, overwriting the old one. You are now ready to play, test, and develop the mod.
