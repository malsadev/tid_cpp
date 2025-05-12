# Transitive Include Detector

**Transitive Include Detector** is a C++ tool that leverages **libclang** to detect and warn about transitive includes in C/C++ projects. Transitive includes occur when headers are indirectly included through other headers, often leading to increased compilation times and unnecessary dependencies.

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

## 📸 Screenshots

_Add screenshots here to demonstrate the tool's functionality, sample output, or Code::Blocks integration._

![Screenshot Placeholder](./screenshots/example.png)

---

## 🔧 Usage

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/transitive-include-detector.git
