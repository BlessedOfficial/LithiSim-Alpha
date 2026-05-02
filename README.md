# LithiSim-Alpha

## Introduction

I have always enjoyed **simulation modelling**—turning messy real-world systems into something you can run, measure, and reason about. **LithiSim-Alpha** is a personal side project where I apply what I have studied so far, dig deeper into transport and operations-style simulation, and keep exploring that space in a hands-on way.

Alongside the domain work, I am deliberately using this repo to **level up in C++**, get more comfortable with **Python** for analysis and experimentation, and stretch into **C#** where it fits (for example tooling, small utilities, or future integrations—details will grow as the project does). The goal is not only a working model, but a stronger toolkit as a developer.

## Portfolio purpose and how I work

The **main purpose of this project is my portfolio**: it should show how I think, structure code, and apply simulation ideas at a **depth I genuinely understand**. A reviewer should see concepts I can explain and defend—not borrowed sophistication I cannot walk through in plain language.

In line with how many people build software today, I also use **AI as a partner**: for learning, design discussion, and productivity, while I remain responsible for architecture, correctness, and comprehension. That collaboration is **educational** first; it is not a substitute for knowing what the program does and why.

## The idea

LithiSim-Alpha models **mining transport** as a discrete simulation: trucks, routes, queues, delays, and (eventually) stochastic events such as breakdowns. The **C++ engine** is intended to be the fast, deterministic core that steps time forward and writes structured logs. A **Python layer** will ingest those logs for metrics, plots, and comparisons—especially between simple dispatch rules and smarter policies.

In short: simulate the mine floor, export honest data, and let the numbers show where bottlenecks and policy choices matter.

## Repository layout

| Path | Role |
|------|------|
| `engine-cpp/` | C++ simulator (CMake project, entry in `src/main.cpp`) |
| `analysis-py/` | Python notebooks and scripts (to be filled as logs exist) |
| `data/` | Raw and processed run outputs |
| `docs/` | Abstract, figures, presentation notes |

## Building the C++ engine

Prerequisites: **CMake** and a C++17 toolchain (e.g. **MinGW-w64** or **MSVC**).

From `engine-cpp`:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
.\build\lithisim.exe
```

With Visual Studio generators, configure without `-G "MinGW Makefiles"`, then:

```powershell
cmake --build build --config Release
```

Release binaries will appear under `build/` (name: `lithisim` / `lithisim.exe`).

## Status

Early stage: project skeleton, CMake build, and a minimal executable. Simulation logic, logging, and Python analysis will land incrementally.

## License

No license file is attached yet. If you clone this before a license is added, treat the code as **all rights reserved** unless the author specifies otherwise.
