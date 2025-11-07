# Transitive Include Detector

**Transitive Include Detector** is a C++ tool that leverages **libclang** to detect and warn about transitive includes in C/C++ projects. Transitive includes occur when headers are indirectly included through other headers, often leading to compatibility issues when building projects in different environment.

---

## 📸 Screenshots

_Sample output for some cases_

![Screenshot Placeholder](./screenshots/many_issues.png)
![Screenshot Placeholder](./screenshots/one_issue.png)
![Screenshot Placeholder](./screenshots/no_issue.png)

---

## 🚀 Key Features

- Utilizes **libclang** for efficient and accurate parsing of C++ source files  
- Detects and reports transitive includes within the codebase  
- Designed for integration with **Code::Blocks IDE** for seamless development and testing  

---

## 💡 Why Use This Tool?

Transitive includes can significantly impact build times and code maintainability. This tool helps developers:

- Identify unnecessary indirect dependencies  
- Improve compilation performance  
- Simplify and clean up header usage across large codebases  

---

## 🔧 Usage

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/transitive-include-detector.git
2. Open .cbp file using Codeblocks
3. Build the project
4. ```cd bin/debug```
5. ```./transitive_include_detectos <path to project>```
6. ```./transitive_include_detector --help for all configurtion options```
