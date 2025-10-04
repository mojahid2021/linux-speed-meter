# Contributing to Linux Speed Meter

Thank you for your interest in contributing to Linux Speed Meter! This document provides guidelines and instructions for contributing to the project.

## Code of Conduct

Be respectful, constructive, and professional in all interactions. We're all here to make this project better.

## How Can I Contribute?

### Reporting Bugs

Before creating a bug report:
1. Check if the issue already exists in [GitHub Issues](https://github.com/mojahid2021/linux-speed-meter/issues)
2. Try the latest version to see if the issue is already fixed
3. Collect relevant information (OS, version, error messages)

**When reporting a bug, include:**
- Clear, descriptive title
- Steps to reproduce the issue
- Expected behavior vs actual behavior
- Screenshots if applicable
- System information (OS, version, desktop environment)
- Application version (check Help → About or VERSION file)

**Bug Report Template:**
```markdown
**Description**
A clear description of the bug.

**To Reproduce**
1. Go to '...'
2. Click on '...'
3. See error

**Expected behavior**
What should happen.

**Screenshots**
If applicable.

**Environment:**
- OS: [e.g., Ubuntu 22.04]
- Desktop Environment: [e.g., GNOME 42]
- Application Version: [e.g., 2.0.0]

**Additional context**
Any other relevant information.
```

### Suggesting Features

Before suggesting a feature:
1. Check if it's already proposed in issues or discussions
2. Consider if it fits the project's scope
3. Think about how it would benefit other users

**Feature Request Template:**
```markdown
**Is your feature request related to a problem?**
A clear description of the problem.

**Describe the solution you'd like**
What you want to happen.

**Describe alternatives you've considered**
Other solutions or features you've considered.

**Additional context**
Mockups, examples, or other relevant information.
```

### Code Contributions

#### Getting Started

1. **Fork the repository**
   ```bash
   # Click "Fork" on GitHub, then:
   git clone https://github.com/YOUR-USERNAME/linux-speed-meter.git
   cd linux-speed-meter
   git remote add upstream https://github.com/mojahid2021/linux-speed-meter.git
   ```

2. **Set up development environment**
   ```bash
   # Linux
   ./setup_dev.sh
   
   # Or manually install dependencies (see BUILD_AND_TEST.md)
   ```

3. **Create a branch**
   ```bash
   git checkout -b feature/my-new-feature
   # or
   git checkout -b fix/bug-description
   ```

#### Development Guidelines

**Code Style:**
- Follow existing code style in the project
- Use C++14/17 standards
- Keep functions focused and small
- Add comments for complex logic
- Use meaningful variable names

**Example:**
```cpp
// Good
double calculateAverageSpeed(const std::vector<double>& speeds) {
    if (speeds.empty()) return 0.0;
    double sum = std::accumulate(speeds.begin(), speeds.end(), 0.0);
    return sum / speeds.size();
}

// Avoid
double calc(std::vector<double> s) {
    double t=0;for(auto x:s)t+=x;return t/s.size();
}
```

**Commits:**
- Write clear, descriptive commit messages
- Use present tense ("Add feature" not "Added feature")
- Reference issues when applicable
- Keep commits focused on single changes

**Commit Message Format:**
```
Short summary (50 chars or less)

More detailed explanation if needed. Wrap at 72 characters.
Explain what and why, not how (code shows how).

- Bullet points are okay
- Use present tense

Fixes #123
```

**Testing:**
- Test your changes on the target platform(s)
- Ensure existing features still work
- Add manual test cases to BUILD_AND_TEST.md if applicable
- Run builds successfully before submitting

**Documentation:**
- Update relevant documentation (README, USER_GUIDE, etc.)
- Add comments for new functions and classes
- Update CHANGELOG.md with your changes
- Add docstrings for public APIs

#### Pull Request Process

1. **Update your fork**
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Ensure quality**
   - Code compiles without errors
   - No new warnings introduced
   - Features tested manually
   - Documentation updated

3. **Submit Pull Request**
   - Push to your fork: `git push origin feature/my-new-feature`
   - Go to GitHub and create Pull Request
   - Fill in the PR template completely
   - Link related issues

**Pull Request Template:**
```markdown
## Description
Brief description of changes.

## Type of Change
- [ ] Bug fix (non-breaking change fixing an issue)
- [ ] New feature (non-breaking change adding functionality)
- [ ] Breaking change (fix or feature causing existing functionality to change)
- [ ] Documentation update

## Testing
How was this tested?
- [ ] Manual testing on [OS/version]
- [ ] Verified existing features work
- [ ] Added test documentation

## Checklist
- [ ] Code follows project style
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No new warnings
- [ ] CHANGELOG.md updated

## Screenshots
If applicable.

## Related Issues
Fixes #(issue number)
```

### Documentation Contributions

Documentation improvements are always welcome!

**Types of documentation:**
- README updates
- User guide enhancements
- Code comments
- Build instructions
- Troubleshooting guides

**How to contribute docs:**
1. Fork repository
2. Edit markdown files
3. Submit pull request
4. No build required for doc-only changes

### Translation (Future)

Currently not supported, but we welcome:
- Translation suggestions
- Internationalization proposals
- Locale-specific improvements

## Project Structure

```
linux-speed-meter/
├── src/              # Source files
│   ├── main.cpp          # Linux GTK entry
│   ├── main_qt.cpp       # Qt entry
│   ├── mainwindow.cpp    # Qt main window
│   ├── systemtray.cpp    # Qt system tray
│   ├── window.cpp        # GTK window
│   ├── data_exporter.cpp # Export functionality
│   └── ...
├── include/          # Header files
│   ├── mainwindow.h
│   ├── version.h         # Version definitions
│   └── ...
├── packaging/        # Package build scripts
├── docs/            # Documentation (if added)
├── CMakeLists.txt   # Main build config
└── README.md
```

## Development Workflow

### Typical Feature Development

1. **Plan**
   - Open issue to discuss feature
   - Get feedback from maintainers
   - Design approach

2. **Implement**
   - Create feature branch
   - Write code following guidelines
   - Test thoroughly

3. **Document**
   - Update user documentation
   - Add code comments
   - Update changelog

4. **Submit**
   - Create pull request
   - Address review feedback
   - Merge when approved

### Bug Fix Workflow

1. **Reproduce**
   - Confirm bug exists
   - Understand root cause
   - Document steps to reproduce

2. **Fix**
   - Create fix branch
   - Implement minimal fix
   - Test fix works

3. **Verify**
   - Test original case
   - Test edge cases
   - Ensure no regressions

4. **Submit**
   - Create pull request
   - Reference bug report
   - Merge when approved

## Code Review Process

### For Contributors

- Be patient - reviews take time
- Be open to feedback
- Address all comments
- Update as requested
- Be responsive

### Review Checklist

Reviewers will check:
- [ ] Code quality and style
- [ ] Functionality works as described
- [ ] No breaking changes (or properly documented)
- [ ] Documentation updated
- [ ] Appropriate testing
- [ ] Clean commit history

## Release Process

(For maintainers)

1. Update VERSION file
2. Update version.h
3. Update CHANGELOG.md with date
4. Update all version references
5. Build all packages
6. Test on target platforms
7. Create git tag
8. Create GitHub release
9. Upload packages
10. Announce release

## Getting Help

- **Questions**: Open a discussion on GitHub
- **Issues**: File a bug report
- **Stuck**: Ask in pull request comments
- **Ideas**: Start a feature discussion

## Recognition

Contributors will be:
- Listed in release notes
- Credited in commits (via Co-authored-by)
- Mentioned in project documentation
- Thanked publicly for significant contributions

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Additional Resources

- [BUILD_AND_TEST.md](BUILD_AND_TEST.md) - Build instructions
- [USER_GUIDE.md](USER_GUIDE.md) - Feature documentation
- [FEATURES.md](FEATURES.md) - Technical specifications
- [CHANGELOG.md](../CHANGELOG.md) - Version history

## Questions?

Feel free to ask questions by:
- Opening a GitHub discussion
- Commenting on relevant issues
- Reaching out to maintainers

Thank you for contributing to Linux Speed Meter! 🚀
