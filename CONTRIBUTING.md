# Contributing to Orion OS

> **Join the future of operating systems!**  
> *Thank you for your interest in contributing to Orion OS*

## 🌟 Welcome Contributors

Orion OS is an open-source project that welcomes contributions from developers, researchers, and enthusiasts worldwide. Whether you're fixing bugs, adding features, improving documentation, or testing the system, your contributions are valuable!

## 🚀 Quick Start

### 📋 Prerequisites

Before contributing, ensure you have:

- **Development Environment**: Linux, macOS, or WSL on Windows
- **Build Tools**: Clang/LLVM, CMake, Python 3.8+, Rust (stable)
- **Version Control**: Git with GitHub account
- **Testing Tools**: QEMU for testing (recommended)

### 🔧 Setting Up Development Environment

```bash
# 1. Fork the repository on GitHub
# 2. Clone your fork
git clone https://github.com/YOUR_USERNAME/orion-os.git
cd orion-os

# 3. Add upstream remote
git remote add upstream https://github.com/orion-os/orion.git

# 4. Install dependencies
sudo apt update
sudo apt install build-essential clang cmake python3 xorriso qemu-system-x86

# 5. Install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
rustup target add x86_64-unknown-none

# 6. Build the project
python3 build.py build

# 7. Run tests
python3 test.py --all
```

## 🎯 How to Contribute

### 🐛 Reporting Bugs

1. **Search existing issues** to avoid duplicates
2. **Use the bug report template** when creating new issues
3. **Provide detailed information**:
   - OS and hardware information
   - Steps to reproduce
   - Expected vs actual behavior
   - Relevant logs or screenshots

### 💡 Suggesting Features

1. **Check the roadmap** to see if your feature is planned
2. **Open a feature request** with detailed description
3. **Discuss the approach** before implementing large features
4. **Consider backward compatibility** and security implications

### 🔧 Code Contributions

#### **1. Find an Issue**
- Look for issues labeled `good first issue` for beginners
- Check `help wanted` issues for more challenging tasks
- Comment on issues you want to work on

#### **2. Create a Branch**
```bash
git checkout -b feature/your-feature-name
# or
git checkout -b fix/bug-description
```

#### **3. Make Changes**
- Follow our coding standards (see below)
- Write tests for new functionality
- Update documentation as needed
- Ensure all tests pass

#### **4. Commit Changes**
```bash
git add .
git commit -m "feat: add new scheduler optimization"
# or
git commit -m "fix: resolve memory leak in VMM"
```

#### **5. Push and Create PR**
```bash
git push origin your-branch-name
```
Then create a Pull Request on GitHub.

## 📏 Coding Standards

### 🔤 C Code Standards

```c
// File header (required for all files)
/*
 * Orion Operating System - Component Name
 *
 * Brief description of the file's purpose.
 * More detailed description if needed.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

// Include guards
#ifndef ORION_COMPONENT_H
#define ORION_COMPONENT_H

// Includes (system first, then project)
#include <stdint.h>
#include <stdbool.h>
#include <orion/types.h>

// Constants and macros
#define MAX_ITEMS 256
#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))

// Type definitions
typedef struct component {
    uint64_t id;                 // Component identifier
    bool active;                 // Is component active
    spinlock_t lock;             // Synchronization lock
} component_t;

// Function declarations
void component_init(void);
int component_create(component_t* comp);
void component_destroy(component_t* comp);

#endif // ORION_COMPONENT_H
```

**C Style Guidelines:**
- **Indentation**: 4 spaces (no tabs)
- **Braces**: K&R style (opening brace on same line)
- **Naming**: `snake_case` for functions and variables
- **Comments**: Document all public APIs and complex logic
- **Error handling**: Always check return values
- **Memory safety**: No unsafe memory operations

### 🦀 Rust Code Standards

```rust
/*
 * Orion Operating System - Rust Driver Component
 *
 * Safe driver implementation for Orion OS.
 * Follows Rust best practices and #![no_std] requirements.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#![no_std]
#![forbid(unsafe_code)] // Only allow unsafe in specific HAL modules

use orion_driver::{DriverResult, DriverError, OrionDriver};

/// Component configuration structure
pub struct ComponentConfig {
    /// Maximum number of items
    pub max_items: u32,
    /// Enable debug mode
    pub debug_enabled: bool,
}

impl ComponentConfig {
    /// Create new configuration with defaults
    pub fn new() -> Self {
        Self {
            max_items: 256,
            debug_enabled: false,
        }
    }
}

/// Main component implementation
pub struct Component {
    config: ComponentConfig,
    items: Vec<Item>,
}

impl Component {
    /// Initialize component with configuration
    pub fn init(config: ComponentConfig) -> DriverResult<Self> {
        Ok(Self {
            config,
            items: Vec::new(),
        })
    }
}
```

**Rust Style Guidelines:**
- **Format**: Use `cargo fmt` for consistent formatting
- **Linting**: Use `cargo clippy` and fix all warnings
- **Documentation**: Document all public APIs with `///`
- **Error handling**: Use `Result<T, E>` for fallible operations
- **Safety**: Minimize `unsafe` code, document when necessary
- **Testing**: Write unit tests for all modules

### 📝 Documentation Standards

- **README files**: Clear, comprehensive, up-to-date
- **Code comments**: Explain why, not what
- **API documentation**: Complete parameter and return descriptions
- **Architecture docs**: High-level design decisions
- **User guides**: Step-by-step instructions

## 🧪 Testing Requirements

### ✅ Required Tests

1. **Unit Tests**: Test individual functions and modules
2. **Integration Tests**: Test component interactions
3. **Security Tests**: Verify security features work correctly
4. **Performance Tests**: Ensure no regressions
5. **Boot Tests**: Verify system boots correctly

### 🏃 Running Tests

```bash
# Run all tests
python3 test.py --all

# Run specific test suites
python3 test.py --suite build
python3 test.py --suite kernel
python3 test.py --suite drivers
python3 test.py --suite security

# Run with QEMU boot test
python3 test.py --suite qemu
```

### 📊 Test Coverage

- **Kernel code**: Minimum 80% coverage
- **Driver code**: Minimum 90% coverage
- **Security code**: Minimum 95% coverage
- **Critical paths**: 100% coverage required

## 🔒 Security Guidelines

### 🛡️ Security-First Development

1. **Threat Modeling**: Consider security implications of all changes
2. **Input Validation**: Validate all inputs, especially from hardware/userland
3. **Privilege Minimization**: Request only necessary capabilities
4. **Error Handling**: Never expose sensitive information in errors
5. **Code Review**: All security-related code requires multiple reviews

### 🚨 Security Issues

- **Never commit** security vulnerabilities knowingly
- **Report security bugs** privately to security@orion-os.dev
- **Follow responsible disclosure** for security research
- **Use secure coding practices** at all times

## 📋 Pull Request Process

### 🔍 Before Submitting

- [ ] All tests pass (`python3 test.py --all`)
- [ ] Code follows style guidelines
- [ ] Documentation is updated
- [ ] Commit messages are descriptive
- [ ] No merge conflicts with main branch

### 📝 PR Template

```markdown
## Description
Brief description of changes made.

## Type of Change
- [ ] Bug fix (non-breaking change that fixes an issue)
- [ ] New feature (non-breaking change that adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update

## Testing
- [ ] Unit tests added/updated
- [ ] Integration tests pass
- [ ] Manual testing completed
- [ ] QEMU boot test passes

## Security Impact
- [ ] No security impact
- [ ] Security review required
- [ ] Security documentation updated

## Checklist
- [ ] Self-review completed
- [ ] Code follows style guidelines
- [ ] Documentation updated
- [ ] Tests added/updated
- [ ] All CI checks pass
```

### 🔄 Review Process

1. **Automated Checks**: CI must pass (build, tests, linting)
2. **Code Review**: At least one maintainer approval required
3. **Security Review**: Required for security-related changes
4. **Testing**: Manual testing for significant changes
5. **Documentation**: Technical writing review if needed

## 🏆 Recognition

### 🌟 Contributors

All contributors are recognized in:
- **Contributors file**: Listed in project contributors
- **Release notes**: Credited for their contributions
- **Hall of fame**: Outstanding contributors featured
- **Swag**: T-shirts and stickers for active contributors

### 🎖️ Contribution Types

We value all types of contributions:

- **Code**: Bug fixes, features, optimizations
- **Documentation**: User guides, technical docs, tutorials
- **Testing**: Bug reports, test cases, QA
- **Design**: UI/UX improvements, graphics, branding
- **Community**: Helping users, organizing events, advocacy
- **Translation**: Localization and internationalization

## 📞 Getting Help

### 💬 Communication Channels

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: General questions and ideas
- **Discord**: Real-time chat with developers
- **Email**: Direct contact with maintainers

### 🆘 Support

- **Documentation**: Check docs/ directory first
- **FAQ**: Common questions and answers
- **Search**: Look through existing issues and discussions
- **Ask**: Don't hesitate to ask questions

## 📚 Learning Resources

### 📖 OS Development

- **OSDev Wiki**: Comprehensive OS development resource
- **Intel Manuals**: x86_64 architecture documentation
- **UEFI Specification**: Modern boot standard documentation
- **Linux Kernel**: Reference implementation for many concepts

### 🦀 Rust Resources

- **The Rust Book**: Official Rust programming guide
- **Embedded Rust**: Guide for embedded/no_std development
- **Unsafe Rust**: Guidelines for unsafe code when necessary
- **Rust Nomicon**: Advanced Rust memory management

### 🔒 Security Resources

- **OWASP**: Web application security practices (applicable to OS)
- **CWE**: Common weakness enumeration
- **CVE**: Common vulnerabilities and exposures database
- **Security Research**: Academic papers on OS security

## 🎯 Project Roadmap

### 📅 Current Focus (v1.0)

- **Core stability**: Rock-solid kernel foundation
- **Driver ecosystem**: Complete hardware support
- **Security hardening**: Advanced threat protection
- **Performance optimization**: Maximum efficiency
- **Documentation**: Comprehensive guides

### 🚀 Future Goals (v2.0+)

- **Multi-architecture**: ARM64, RISC-V support
- **Real-time capabilities**: Hard real-time guarantees
- **Distributed computing**: Cluster and cloud features
- **Advanced graphics**: Vulkan, DirectX support
- **Commercial certification**: Industry compliance

---

Thank you for contributing to Orion OS! Together, we're building the future of operating systems. 🌟

**Questions?** Feel free to reach out:
- **Email**: dev@orion-os.dev
- **Discord**: https://discord.gg/orion-os
- **GitHub**: @orion-os

*Happy coding!* 🚀
