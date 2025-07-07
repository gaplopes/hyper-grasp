# Hyper-GRASP: A Hypervolume-Based Constructive Heuristic

[![DOI](https://img.shields.io/badge/DOI-10.1145%2F3729878.3746617-blue)](https://doi.org/10.1145/3729878.3746617)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/std/the-standard)

## Table of Contents
- [About](#about)
- [Scientific Paper](#scientific-paper)
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Instance Files](#instance-files)
- [Implementation Details](#implementation-details)
- [License](#license)
- [Contributing](#contributing)
- [Contact](#contact)

## About

This project implements the Hyper-GRASP algorithm for Multiobjective Combinatorial Optimization (MOCO) problems.

Hyper-GRASP is a hypervolume-based GRASP metaheuristic that combines greedy construction with local search to find a set of non-dominated solutions and approximate the Pareto front.

In particular, this repository includes implementations for two classic MOCO problems, a maximization and a minimization problem:
- Multiobjective Knapsack Problem (MOKP) for any number of objectives
- Multiobjective Minimum Spanning Tree (MOMST) for the bi-objective case

In addition, the codebase provides a flexible template for implementing other MOCO problems (see `problem/problem_template.hpp`).

## Scientific Paper

If you use this code in your research, please cite our paper: [Hyper-GRASP: A Hypervolume-Based Constructive Heuristic](https://doi.org/10.1145/3729878.3746617)

## Features

- Command-line interface using CLI11
- Support for multiple stopping criteria (time-based and iteration-based)
- Statistics collection and analysis
- Hypervolume indicator calculation
- Local search optimization
- Support for both maximization and minimization problems
- Implementation of two classic problems:
  - Multiobjective Knapsack Problem (MOKP)
  - Multiobjective Minimum Spanning Tree (MOMST)

## Requirements

- C++17 or later
- CMake 3.10 or later
- CLI11 library
- [mooutils](https://github.com/adbjesus/mooutils) library (included as a submodule)

## Installation

```bash
# Clone the repository
git clone https://github.com/gaplopes/hyper-grasp.git
cd hyper-grasp

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .
```

## Usage

```bash
./hyper-grasp [OPTIONS]

Options:
  --local-search BOOL      Enable local search (default: true)
  --maximization BOOL      Set problem type: maximization (true) or minimization (false) (required)
  --alpha FLOAT           Set alpha value [0.0-1.0] (default: 0.05)
  --criteria STRING       Set stopping criteria: 'time' or 'iterations' (default: time)
  --criteria_limit INT    Set criteria limit: time in seconds or iterations (default: 100)
  --input-file FILE      Input file with the problem instance (required)
  --moco-problem STRING  Set the MOCO problem type: 'mokp' or 'momst' (default: mokp)
```

Use the `--help` option to display the available options.

### Usage Example

### Solving a Multiobjective Knapsack Problem

```bash
./hyper-grasp --maximization=true --local-search=true --alpha=0.05 --criteria=iterations
              --criteria_limit=100 --moco-problem=mokp --input-file=../instances/mokp/random/2D/100_1.in
```

## Project Structure

```
hyper-grasp/
├── apps/
│   └── main.cpp              # Main application
├── include/
│   ├── indicator.hpp             # Hypervolume Indicator calculation
│   ├── dominance.hpp             # Dominance relation and comparison
│   ├── statistics.hpp            # Statistics collection and analysis
│   └── stopping_criteria.hpp     # Stopping criteria implementations
├── problem/
│   ├── mokp.hpp             # Multiobjective Knapsack Problem
│   |── momst.hpp            # Multiobjective Minimum Spanning Tree
│   ├── ...                  # Other problem implementations
|   ├── problem_template.hpp # Template class for multiobjective problems
|   └── problem_base.hpp     # Base class for multiobjective problems
├── instances/
│   ├── mokp/                # MOKP instances
│   ├── momst/               # MOMST instances
│   ├── ...                  # Other instances
│   └── README.md            # Instances format and examples
└── solver/
    └── hyper_grasp.hpp      # Solver implementation
```

## Instance Files

In the `instances/` directory, you can find example instance files for the implemented problems.

These files are organized into subdirectories for each problem type, such as `mokp/` for Multiobjective Knapsack Problem instances and `momst/` for Multiobjective Minimum Spanning Tree instances.
Moreover, these files follow a specific format, as described in the `instances/README.md` file within the `instances/` directory.

## Implementation Details

### Statistics Output

The program outputs comprehensive statistics in CSV format, including:
- Seed value for reproducibility
- Solution statistics and metrics
- Number of iterations and skipped iterations
- Solutions found
- Hypervolume indicators
- Performance metrics (execution time, solution quality)

The statistics output format is as follows:
```
seed
(stat1,stat2,stat3) (stat1,stat2,stat3) ... (stat1,stat2,stat3)
iterations skipped_iterations
(sol1) (sol2) ... (solN)
nondominated_set_hv solutions_set_hv ratio_hv nadir_nset_hv nadir_set_hv ratio_nadir_hv
n_nondominated_set n_solutions n_matching elapsed_time
```

In the current implementation, the statistics pair `(stat1,stat2,stat3)` corresponds to the following metrics:
- `current stopping criteria value`, `size of the solution set` and `hypervolume of the solution set`

Note that the statistics related to the non-dominated set (Pareto front) are only available if the instance file contains this information.
Otherwise, the statistics will be calculated based on the solutions found by the algorithm and not the actual Pareto front.

### Stopping Criteria

Two types of stopping criteria are available:
1. Time-based: Stops after a specified number of seconds
2. Iteration-based: Stops after a specified number of iterations

### Local Search

The local search procedure can be enabled/disabled via command-line arguments and includes:
- Neighborhood exploration
- First-improvement strategy

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Contact

[Gonçalo Lopes](https://github.com/gaplopes) by email at galopes@dei.uc.pt, or on GitHub or LinkedIn (see profile).
