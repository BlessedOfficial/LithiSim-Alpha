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

## Current implementation snapshot

The project is still an early prototype, but the core simulation scaffolding is now in place.

- `engine-cpp/include/truck.h` and `engine-cpp/src/truck.cpp`
  - Truck state machine implemented with states: `IDLE`, `HAULING`, `DUMPING`, `BROKEN`
  - Health-aware trucks (`health` in range 0-100)
  - Dispatch gating (`dispatched` flag): only dispatched trucks advance cycle actions
  - Speed property (`speed`) used as average travel time parameter
  - State transition logging via `log_state()` and `log_transition(...)`

- `engine-cpp/include/math_models.h` and `engine-cpp/src/math_models.cpp`
  - Reusable stochastic model hub (`MathModels`)
  - Implemented distributions:
    - Poisson sampling (`sample_poisson`)
    - Exponential sampling (`sample_exponential`)
    - Uniform integer sampling (`sample_uniform_int`)

- `engine-cpp/include/simulate.h` and `engine-cpp/src/simulate.cpp`
  - System-level simulation controller (`simulate(...)`)
  - Dispatch scheduler checks every random 20-25 timesteps
  - Low-intensity stochastic dispatch trigger using Poisson (`lambda = 0.03`)
  - Dispatch call count matches sampled required events
  - Dispatch policy currently picks one eligible truck by highest health
  - Selected truck travel time is sampled from exponential distribution using:
    - `rate = 1.0 / speed`
    - sampled seconds rounded up to at least 1 timestep

- `engine-cpp/src/main.cpp`
  - Wires together truck fleet + math models + simulate loop for demo runs

- `assets/`
  - `truck-state-flow.mmd` and exported `truck-state-flow.png` documenting transition flow

## Current behavior notes

- With `lambda = 0.03` and dispatch checks every 20-25 timesteps, dispatch events are intentionally rare in short runs.
- The current model is focused on validating trigger wiring and stochastic control flow, not final operational realism yet.

## Next planned increments

- Add structured CSV telemetry output for downstream Python analysis
- Add explicit queue/server logic (crusher/pit service modeling)
- Add stochastic failure/repair calibration (MTBF/MTTR scenarios)
- Introduce simulation config object for tunable parameters without code edits
- Add seed-fixed sanity tests for repeatable transition verification

## License

No license file is attached yet. If you clone this before a license is added, treat the code as **all rights reserved** unless the author specifies otherwise.
