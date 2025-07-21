# CI Pathway: Parallel Computing Exercises

## Overview

This repository contains educational materials and programming exercises for the **2025 NCSA CI Pathway Parallel Computing Course**. The course provides hands-on experience with parallel programming paradigms including OpenMP, MPI, and OpenACC, focusing on high-performance computing concepts and practical implementation.

### Acknowledgment

This course is sponsored by **NSF Award #2417789** and led by the **Pittsburgh Supercomputing Center (PSC)** and the **National Center for Supercomputing Applications (NCSA)**.

## Course Structure

The repository is organized into comprehensive modules covering different aspects of parallel computing:

### 📚 Core Materials
- **Exercises/**: Practical programming exercises with multiple paradigms
- **HW/**: Three comprehensive homework assignments with detailed analysis
- **Lecture/**: Course lecture materials and presentations
- **Setup**: Environment configuration and setup instructions

### 🖥️ Computing Environment

**Platform**: NCSA Delta HPC Cluster
- **Login Node**: dt-login04.delta.ncsa.illinois.edu  
- **Compute Nodes**: AMD EPYC 7763 64-Core processors
- **Architecture**: x86_64 with 128 hardware threads per node
- **Interconnect**: HPE Slingshot high-speed network

**Quick Start**: [NCSA OnDemand Portal](https://openondemand.delta.ncsa.illinois.edu/)  
**Training Platform**: [HPC Moodle](https://www.hpc-training.org/moodle/)

## Programming Paradigms Covered

### 1. **OpenMP (Shared Memory Parallelism)**
- Thread-based parallelization for multicore systems
- Compiler directives for parallel loops and reductions
- Performance scaling from 1-32 threads
- **Example**: `laplace_omp.c` - 2D heat equation solver

### 2. **MPI (Distributed Memory Parallelism)** 
- Message passing for cluster computing
- Domain decomposition techniques
- Inter-process communication patterns
- **Example**: `laplace_mpi.c` - Distributed Laplace solver

### 3. **OpenACC (GPU Computing)**
- Accelerator-based parallel computing
- GPU optimization strategies
- Performance portability across architectures
- **Example**: `laplace_acc.c` - GPU-accelerated solver

## Key Learning Objectives

### 📊 Performance Analysis
Students learn to:
- Measure parallel speedup and efficiency
- Analyze scalability characteristics  
- Compare different parallelization strategies
- Optimize for specific hardware architectures

### 🔧 Practical Implementation
- Compile and run parallel applications
- Use job scheduling systems (Slurm)
- Debug parallel code issues
- Apply algorithmic optimizations (e.g., Red-Black iterative methods)

### 📈 Research Methodology
- Conduct systematic performance experiments
- Document results with statistical analysis
- Create professional technical reports
- Visualize performance data effectively

## Sample Results

### OpenMP Performance (Laplace Solver, 1000×1000 grid)
| Threads | Time (s) | Speedup | Efficiency |
|---------|----------|---------|------------|
| 1       | 21.7     | 1.00×   | 100%       |
| 8       | 4.2      | 5.21×   | 65.1%      |
| 32      | 2.0      | 10.91×  | 34.1%      |

### MPI Performance (4 Processes)
- **Execution Time**: 6.4 seconds
- **Speedup**: 7.30× (vs serial baseline)
- **Communication Overhead**: Optimized ghost cell exchanges

## Assignment Highlights

### **HW1**: OpenMP vs Serial Performance
- Parallel scaling analysis with thread count variation
- Comparison of serial, OpenMP, and enhanced parallel algorithms
- Red-Black checkerboard optimization implementation

### **HW2**: OpenMP Race Condition Analysis  
- Prime number calculation with parallel optimization
- Race condition identification and resolution
- Thread synchronization techniques

### **HW3**: Advanced MPI Optimization
- 2D domain decomposition strategies
- Communication pattern optimization
- Performance analysis with scaling studies
- Jupyter notebook visualization of results

## Technical Specifications

### Compilation Examples
```bash
# OpenMP
nvc -mp laplace_omp.c

# MPI  
mpicc laplace_mpi.c

# OpenACC
nvc -acc -Minfo=accel laplace_acc.c
```

### Job Submission (Slurm)
```bash
# Interactive OpenMP job
srun --account=becs-delta-cpu --partition=cpu-interactive \
     --nodes=1 --cpus-per-task=32 --pty bash

# MPI job submission
srun --account=becs-delta-cpu --partition=cpu-interactive \
     --nodes=1 --tasks=4 --tasks-per-node=4 --pty bash
```

## Learning Outcomes

Upon completion, students will be able to:

1. **Implement** parallel algorithms using multiple programming models
2. **Analyze** parallel performance characteristics and bottlenecks  
3. **Optimize** code for different hardware architectures
4. **Evaluate** trade-offs between programming paradigms
5. **Communicate** technical results through professional documentation

## Course Impact

This course prepares students for careers in:
- **High-Performance Computing**: National laboratories, research institutions
- **Scientific Computing**: Weather modeling, computational physics, bioinformatics
- **Industry Applications**: Financial modeling, machine learning, data analytics
- **Research Computing**: Academic research support and development

## Repository Structure

```
CI-Pathway-exercise/
├── README.md                    # This file
└── parallel_computing/
    ├── Exercises/              # Practice implementations
    │   ├── MPI/               # Message passing examples
    │   ├── OpenMP/            # Shared memory examples  
    │   ├── OpenACC/           # GPU computing examples
    │   └── Test/              # Validation programs
    ├── HW/                    # Graded assignments
    │   ├── hw1/              # OpenMP performance study
    │   ├── hw2/              # Race conditions & optimization
    │   └── hw3/              # Advanced MPI techniques
    ├── Lecture/              # Course materials
    └── Setup                 # Environment configuration
```

## Getting Started

1. **Access the computing environment**: Connect to NCSA Delta
2. **Load required modules**: Set up compilers and MPI libraries
3. **Clone this repository**: Download course materials
4. **Start with exercises**: Begin with `Exercises/` directory
5. **Progress to assignments**: Complete `HW/` in sequence

## License

This project is licensed under a modified Apache License 2.0 with academic use restrictions. All rights reserved to **Hochan Son** (ohsono@gmail.com or hochanson@g.ucla.edu). 

**Important**: This software is for academic and educational use only. Commercial use is prohibited and prior authorization from the author is required for any use. See [LICENSE.md](LICENSE.md) for complete terms.

---

*This educational program advances the national cyberinfrastructure workforce through hands-on parallel computing training, preparing the next generation of computational scientists and HPC practitioners.*