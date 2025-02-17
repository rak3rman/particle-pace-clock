# particle-pace-clock

A custom pace clock designed for swim practices

![Pace Clock](images/demo1.jpeg)

## Background

Have you ever had to deal with 2 overpriced and outdated pace clocks breaking for seemingly no reason during swim practice in high school? Well, I did. (drumroll sounds) Introducing the Particle Pace Clock, a state-of-the-art, multi-color, over-engineered digital clock system designed for pacing swim practices. This clock is essentially a strip of RGB leds (Neopixels) connected to a microcontroller (Particle Argon) that displays the time from 0:00 to 59:59 (mm:ss) over and over (in any color you'd like). It probably doesn't sound that cool, but trust me, it is a far cry from what it used to be.

2 of these clocks were installed in 2019 at Auburn High School in Rockford, IL. They are still in use today and performing exceptionally well.

# Setting up clang-format

## Installation

### Windows

```bash
winget install LLVM.LLVM
```

Or download installer from: https://llvm.org/builds/

### Mac

```bash
brew install clang-format
```

### Linux (Ubuntu/Debian)

```bash
sudo apt-get install clang-format
```

## Running clang-format

### Format a single file

```bash
clang-format -i file.cpp
```

### Format all C++ files in a directory (recursively)

**Mac/Linux:**

```bash
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

**Windows PowerShell:**

```powershell
Get-ChildItem -Recurse -Include *.cpp,*.h | ForEach-Object { clang-format -i $_.FullName }
```

## VSCode Integration

1. Install the "C/C++" extension
2. Install the "Clang-Format" extension
3. Add to settings.json:

```json
{
  "editor.defaultFormatter": "xaver.clang-format",
  "[cpp]": {
    "editor.defaultFormatter": "xaver.clang-format"
  },
  "editor.formatOnSave": true
}
```

Now you can format using:

- Shift+Alt+F for manual formatting
- Automatic formatting on save if enabled
