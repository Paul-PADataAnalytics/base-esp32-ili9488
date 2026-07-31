# Lua Game Scripting Guide

Welcome to the **Lua 5.4 Game Scripting & UI Engine Documentation**. This guide provides a complete manual for designing, packaging, and deploying interactive games and UI applications on embedded microcontrollers (ESP32) and native desktop simulators (Linux/SDL2).

## Table of Contents

1. [Chapter 1: General Workflow](01_general_workflow.md)
   - Architecture & Core Execution Flow
   - Memory & Allocation Model
   - Event-Driven Callbacks & Band Rendering
2. [Chapter 2: Project Description](02_project_description.md)
   - File & Directory Layout
   - State Management (`game.*`)
   - Component Tree & Positioning
3. [Chapter 3: Packaging & Cross-Platform Support](03_packaging.md)
   - Embedded String Literals vs. Filesystem Loading
   - Cross-Platform & ABI Compatibility
4. [Chapter 4: Putting It All Together (Complete Project Example)](04_working_project_example.md)
   - Complete RPG Inventory & Shop Tutorial Script
   - C++ Application Integration Harness
5. [Chapter 5: Full API Reference](05_api_reference.md)
   - `ui` Namespace Specifications
   - `game` Namespace Specifications
