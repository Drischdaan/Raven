# 🐦‍⬛ Raven

Raven is a modern C++20 game engine built with a focus on performance and fidelity. It achieves this through Data Oriented Design (DOD) patterns and modern rendering APIs.

![](./Documentation/Assets/Images/Banners/ReadMeBanner.png)

## 🤖 Usage of AI

Raven enforces a strict no-AI-code-generation policy. AI may be used as a tool for research, planning, and answering questions, but never to generate code that ends up in the codebase.

## ⬇️ Prerequisites

Software/Tools you need to install before setting up the development environment.

- [Python >=3.14.6](https://www.python.org/downloads/)
- [CMake >=4.2.3](https://cmake.org/download/)
- [Ninja](https://ninja-build.org/) (used for builds and for the clang-tidy compile database)
- On Windows [Rider](https://jetbrains.com/) [Visual Studio 2022](https://visualstudio.microsoft.com/) for the generated `.sln`

## 🪹 Nest (CLI Tool)

Nest is Ravens CLI tool, which you can use to run commands on the workspace.

### Available Commands

- `setup` - Installs git hooks and generates project files for your platform (Visual Studio `.sln` on Windows, Ninja elsewhere)
- `lint` - Runs clang-tidy on the codebase
- `format` - Runs clang-format on the codebase

## 👟 Get Started

1. Clone the repository `git clone https://github.com/Drischdaan/Raven`
2. Download and install [Prerequisites](#️-prerequisites)
3. Execute setup script `.\Nest.bat setup`(Windows) or `./Nest.sh setup`(Linux)
4. Open Project Files with the IDE of your choice
