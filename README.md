# Transitive Include Detector

This C++ project leverages libclang to detect and warn about transitive includes in source files. Transitive includes occur when unnecessary headers are indirectly included through other headers, potentially bloating compilation times and increasing dependencies.

# Key Features:

  Uses libclang for efficient and accurate parsing of C++ source files.
  Identifies and reports transitive includes within the codebase.
  Integrated with CodeBlocks IDE for seamless development and testing.

# Why Use This Tool:
Transitive includes can significantly impact build times and code maintainability. This tool helps developers identify and refactor unnecessary dependencies, leading to more efficient builds and cleaner codebases.

# Usage:

  Clone the repository.
  Build the project using CodeBlocks IDE or your preferred development environment.
  Run the executable on your C++ project directory to analyze source files and receive warnings about transitive includes.

# Contributing:
Contributions are welcome! Fork the repository, make improvements, and submit a pull request. Help make C++ development faster and more efficient by tackling transitive includes head-on.

# License:
This project is licensed under the MIT License - see the LICENSE file for details.
