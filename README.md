# ./smokessh

>[!TIP]
>Have IPv6? Try it out at ```ssh -p 2222 smokessh.awokolaw.com```

<p align="center">
    <img src="assets/showcase_image.jpeg" alt="showcase image">
</p>

Terminal smoke simulator made in C, wrapped around a Go ssh server.
Move an object around various scenarios and switch up settings to your liking.

## Usage
>[!NOTE]
> These flags apply to the executable itself, and not to the ssh server.

```./smokessh [-cdefhlostw]```

| Flag | Argument | Description | Default |
| --- | --- | --- | -- |
| -c | float | sets the cell size | 0.1 |
| -d | string | searches for presets in defined path | ./ |
| -e | string | loads provided file as default emitter file | default |
| -f | integer | sets the target fps | 30 |
| -h | | shows usage information | |
| -l | float | decay | 1.0f (no decay) |
| -o | integer | sets the obstacle radius | 2 |
| -s | float | sets the timestep | 0.0167 |
| -t | integer | sets ambient temperature | 273 |
| -w | float | sets wind value | 0.0 |

Example:
```
./smokessh -f120 -d"../presets/" -t300
```
Sets target fps to 120, presets folder to ../presets/, and ambient temperature to 300.

> [!IMPORTANT]
> There must be a "default" properly defined emitter file either in the same directory of 
> the executable or in the -d flag defined folder.

## Emitter files

Two main unit types;
 - percent: percentage of screen; 50 50 with a terminal of size 120 x 60 would be cell 60,30
 - cells: absolute coordinates

```
units percent/cells
emitter x y r g b temp
...
emitter x y r g b temp
```

Example:

```
units percent
emitter 50 50 10 10 10 277
```
> [!IMPORTANT]
> Smoke might appear stuck when using units percent due to the percentage causing the smoke
> to spawn on the solid walls of the simulation. Use higher percentage, or switch to units

### Notes

- Max terminal size is 128x128

## Building

smokessh has two parts: the simulation (C, built with CMake)
and an SSH server wrapper (Go, 'in wrapper/').

### Requirements

- A C Compiler (GCC or Clang) with C11 support
- CMake 3.12 or newer
- Go 1.27.3+
- A POSIX system (Simulation uses terminal/PTY features).

### 1. Build the simulation

From the repository root:
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
This produces 'build/smokessh'.

You can run the simulation directly in your terminal at this point:

```
./build/smokessh -d"../presets"
```

### 2. Build the SSH wrapper

```
cd wrapper
go mod download
go build -o smokessh-server .
```

This produces `wrapper/smokessh-server`.

### 3. Run the SSH server

The wrapper launches the C executable for each SSH session, so it must be able to
find it. Run it from a directory containing the C binary and your presets.

The SSH server accepts a few flags:

| Flag | Argument | Description | Default |
| --- | --- | --- | --- |
| -ip | 4 or 6 | sets IP version | 4 |
| -presets | string | sets presets directory | ./presets |
| -exe | string | sets executable path and name | ./smokessh |
| -port | integer | sets port number | 2222 |
| -hostkey | string | sets path to ssh host key | /keys/host_ed25519 |
| -idle-timeout | duration | sets max inactivity time | 2m |
| -max-session | duration | sets max session duration | 15m |
| -max-per-ip | integer | sets max concurrent sessions per ip | 2 |
| -max-sessions | integer | sets max concurrent sessions overall | 5 |

Connect through

```ssh -p 2222 localhost```