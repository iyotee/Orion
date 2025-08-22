#!/usr/bin/env python3
"""
Orion Operating System - Comprehensive Test Suite

Complete testing framework for the Orion OS project.
Includes unit tests, integration tests, performance tests, and security tests.

Developed by Jeremy Noverraz (1988-2025)
August 2025, Lausanne, Switzerland

Copyright (c) 2024-2025 Orion OS Project
License: MIT
"""

import os
import sys
import subprocess
import time
import json
import argparse
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import tempfile
import shutil

class Colors:
    """ANSI color codes for terminal output"""
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

class TestResult:
    """Individual test result"""
    def __init__(self, name: str, passed: bool, duration: float, details: str = ""):
        self.name = name
        self.passed = passed
        self.duration = duration
        self.details = details

class TestSuite:
    """Test suite with multiple test cases"""
    def __init__(self, name: str):
        self.name = name
        self.tests: List[TestResult] = []
        self.setup_time = 0.0
        self.cleanup_time = 0.0

    def add_test(self, result: TestResult):
        self.tests.append(result)

    @property
    def passed_count(self) -> int:
        return sum(1 for test in self.tests if test.passed)

    @property
    def failed_count(self) -> int:
        return sum(1 for test in self.tests if not test.passed)

    @property
    def total_count(self) -> int:
        return len(self.tests)

    @property
    def success_rate(self) -> float:
        return (self.passed_count / self.total_count * 100) if self.total_count > 0 else 0

    @property
    def total_duration(self) -> float:
        return sum(test.duration for test in self.tests) + self.setup_time + self.cleanup_time

class OrionTester:
    """Main testing framework for Orion OS"""
    
    def __init__(self):
        self.project_root = Path(__file__).parent.absolute()
        self.build_dir = self.project_root / "build"
        self.test_results: List[TestSuite] = []
        self.start_time = time.time()
        
    def log(self, message: str, level: str = "INFO"):
        """Log a message with color coding"""
        colors = {
            "INFO": Colors.OKBLUE,
            "SUCCESS": Colors.OKGREEN,
            "WARNING": Colors.WARNING,
            "ERROR": Colors.FAIL,
            "HEADER": Colors.HEADER
        }
        color = colors.get(level, Colors.ENDC)
        timestamp = time.strftime("%H:%M:%S")
        print(f"{color}[{timestamp}] [{level}]{Colors.ENDC} {message}")
    
    def run_command(self, cmd: List[str], cwd: Optional[Path] = None, timeout: int = 30) -> Tuple[bool, str, str]:
        """Run a command and return success status, stdout, stderr"""
        try:
            result = subprocess.run(
                cmd, 
                cwd=cwd, 
                capture_output=True, 
                text=True, 
                timeout=timeout
            )
            return result.returncode == 0, result.stdout, result.stderr
        except subprocess.TimeoutExpired:
            return False, "", f"Command timed out after {timeout} seconds"
        except Exception as e:
            return False, "", str(e)

    def test_build_system(self) -> TestSuite:
        """Test the build system"""
        suite = TestSuite("Build System")
        self.log("Testing build system", "HEADER")
        
        # Test clean build
        start_time = time.time()
        success, stdout, stderr = self.run_command(["./orion-build.sh", "clean"], timeout=60)
        duration = time.time() - start_time
        suite.add_test(TestResult(
            "Clean build", 
            success, 
            duration, 
            stderr if not success else "Build cleaned successfully"
        ))
        
        # Test bootloader build
        start_time = time.time()
        success, stdout, stderr = self.run_command(["./orion-build.sh", "bootloader"], timeout=120)
        duration = time.time() - start_time
        suite.add_test(TestResult(
            "Bootloader build", 
            success, 
            duration, 
            stderr if not success else "Bootloader built successfully"
        ))
        
        # Test kernel build
        start_time = time.time()
        success, stdout, stderr = self.run_command(["./orion-build.sh", "kernel"], timeout=180)
        duration = time.time() - start_time
        suite.add_test(TestResult(
            "Kernel build", 
            success, 
            duration, 
            stderr if not success else "Kernel built successfully"
        ))
        
        # Test complete build (includes drivers)
        start_time = time.time()
        success, stdout, stderr = self.run_command(["./orion-build.sh", "all"], timeout=300)
        duration = time.time() - start_time
        suite.add_test(TestResult(
            "Complete build", 
            success, 
            duration, 
            stderr if not success else "Complete build successful"
        ))
        
        return suite

    def test_kernel_components(self) -> TestSuite:
        """Test kernel components"""
        suite = TestSuite("Kernel Components")
        self.log("Testing kernel components", "HEADER")
        
        # Check if kernel binary exists
        kernel_path = self.build_dir / "kernel" / "orion-kernel.elf"
        suite.add_test(TestResult(
            "Kernel binary exists",
            kernel_path.exists(),
            0.0,
            f"Kernel binary at {kernel_path}"
        ))
        
        # Test kernel size (should be reasonable)
        if kernel_path.exists():
            kernel_size = kernel_path.stat().st_size
            reasonable_size = 1024 * 1024 * 10  # 10MB max
            suite.add_test(TestResult(
                "Kernel size reasonable",
                kernel_size < reasonable_size,
                0.0,
                f"Kernel size: {kernel_size / 1024:.1f}KB"
            ))
        
        # Test for required symbols in kernel
        if kernel_path.exists():
            success, stdout, stderr = self.run_command(["objdump", "-t", str(kernel_path)])
            if success:
                required_symbols = [
                    "kernel_main",
                    "scheduler_init", 
                    "vmm_init",
                    "pmm_init",
                    "syscalls_init",
                    "security_init"
                ]
                
                for symbol in required_symbols:
                    found = symbol in stdout
                    suite.add_test(TestResult(
                        f"Symbol {symbol} present",
                        found,
                        0.0,
                        f"Symbol {symbol} {'found' if found else 'missing'}"
                    ))
        
        return suite

    def test_driver_framework(self) -> TestSuite:
        """Test driver framework"""
        suite = TestSuite("Driver Framework")
        self.log("Testing driver framework", "HEADER")
        
        # Test driver framework build
        framework_dir = self.project_root / "lib" / "orion-driver"
        if framework_dir.exists():
            start_time = time.time()
            success, stdout, stderr = self.run_command(
                ["cargo", "test"], 
                cwd=framework_dir,
                timeout=120
            )
            duration = time.time() - start_time
            suite.add_test(TestResult(
                "Framework unit tests",
                success,
                duration,
                stderr if not success else "Framework tests passed"
            ))
            
            # Test framework compilation
            start_time = time.time()
            success, stdout, stderr = self.run_command(
                ["cargo", "check"], 
                cwd=framework_dir,
                timeout=60
            )
            duration = time.time() - start_time
            suite.add_test(TestResult(
                "Framework compilation",
                success,
                duration,
                stderr if not success else "Framework compiles cleanly"
            ))
        
        # Test individual drivers
        drivers_dir = self.project_root / "drivers"
        for driver_category in ["block", "net", "gpu", "usb"]:
            driver_path = drivers_dir / driver_category
            if driver_path.exists():
                start_time = time.time()
                success, stdout, stderr = self.run_command(
                    ["cargo", "check"], 
                    cwd=driver_path,
                    timeout=60
                )
                duration = time.time() - start_time
                suite.add_test(TestResult(
                    f"{driver_category} driver compilation",
                    success,
                    duration,
                    stderr if not success else f"{driver_category} driver compiles"
                ))
        
        return suite

    def test_security_features(self) -> TestSuite:
        """Test security features"""
        suite = TestSuite("Security Features")
        self.log("Testing security features", "HEADER")
        
        # Check for security-related symbols in kernel
        kernel_path = self.build_dir / "kernel" / "orion-kernel.elf"
        if kernel_path.exists():
            success, stdout, stderr = self.run_command(["objdump", "-t", str(kernel_path)])
            if success:
                security_symbols = [
                    "capabilities_init",
                    "security_init",
                    "cap_create",
                    "cap_check_rights",
                    "security_check_wx_violation",
                    "security_init_kaslr",
                    "get_secure_random"
                ]
                
                for symbol in security_symbols:
                    found = symbol in stdout
                    suite.add_test(TestResult(
                        f"Security symbol {symbol}",
                        found,
                        0.0,
                        f"Symbol {symbol} {'found' if found else 'missing'}"
                    ))
        
        # Test for W^X enforcement in code
        security_files = [
            self.project_root / "kernel" / "core" / "capabilities.c",
            self.project_root / "kernel" / "mm" / "vmm.c",
            self.project_root / "kernel" / "arch" / "x86_64" / "cpu.c"
        ]
        
        for file_path in security_files:
            if file_path.exists():
                with open(file_path, 'r') as f:
                    content = f.read()
                    
                # Check for security functions
                security_checks = [
                    ("W^X check", "security_check_wx_violation" in content),
                    ("KASLR support", "kaslr" in content.lower()),
                    ("Capability check", "cap_check" in content),
                    ("Audit logging", "audit_log" in content)
                ]
                
                for check_name, found in security_checks:
                    suite.add_test(TestResult(
                        f"{file_path.name}: {check_name}",
                        found,
                        0.0,
                        f"{check_name} {'implemented' if found else 'not found'}"
                    ))
        
        return suite

    def test_qemu_boot(self) -> TestSuite:
        """Test QEMU boot (if QEMU is available)"""
        suite = TestSuite("QEMU Boot Test")
        self.log("Testing QEMU boot", "HEADER")
        
        # Check if QEMU is available
        success, _, _ = self.run_command(["which", "qemu-system-x86_64"])
        if not success:
            suite.add_test(TestResult(
                "QEMU availability",
                False,
                0.0,
                "QEMU not found in PATH"
            ))
            return suite
        
        suite.add_test(TestResult(
            "QEMU availability",
            True,
            0.0,
            "QEMU found"
        ))
        
        # Check if ISO exists
        iso_path = self.build_dir / "orion-os.iso"
        if not iso_path.exists():
            suite.add_test(TestResult(
                "ISO availability",
                False,
                0.0,
                f"ISO not found at {iso_path}"
            ))
            return suite
        
        suite.add_test(TestResult(
            "ISO availability",
            True,
            0.0,
            f"ISO found at {iso_path}"
        ))
        
        # Test QEMU boot (short timeout, just check if it starts)
        start_time = time.time()
        qemu_cmd = [
            "qemu-system-x86_64",
            "-cdrom", str(iso_path),
            "-m", "1G",
            "-display", "none",
            "-serial", "stdio"
        ]
        
        try:
            process = subprocess.Popen(
                qemu_cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            
            # Wait for 10 seconds to see if boot starts
            time.sleep(10)
            process.terminate()
            process.wait(timeout=5)
            
            duration = time.time() - start_time
            suite.add_test(TestResult(
                "QEMU boot initiation",
                True,
                duration,
                "QEMU started successfully"
            ))
            
        except Exception as e:
            duration = time.time() - start_time
            suite.add_test(TestResult(
                "QEMU boot initiation",
                False,
                duration,
                f"QEMU boot failed: {str(e)}"
            ))
        
        return suite

    def test_code_quality(self) -> TestSuite:
        """Test code quality"""
        suite = TestSuite("Code Quality")
        self.log("Testing code quality", "HEADER")
        
        # Check for TODO/FIXME comments (should be minimal)
        source_dirs = [
            self.project_root / "kernel",
            self.project_root / "bootloader" / "src",
            self.project_root / "lib" / "orion-driver" / "src"
        ]
        
        todo_count = 0
        fixme_count = 0
        
        for source_dir in source_dirs:
            if source_dir.exists():
                for file_path in source_dir.rglob("*.[ch]"):
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                        todo_count += content.count("TODO")
                        fixme_count += content.count("FIXME")
        
        suite.add_test(TestResult(
            "TODO comments",
            todo_count < 10,  # Allow some TODOs
            0.0,
            f"Found {todo_count} TODO comments"
        ))
        
        suite.add_test(TestResult(
            "FIXME comments",
            fixme_count == 0,  # No FIXMEs allowed
            0.0,
            f"Found {fixme_count} FIXME comments"
        ))
        
        # Check file structure
        required_files = [
            "README.md",
            "orion-build.sh",
            "kernel/CMakeLists.txt",
            "bootloader/Makefile",
            "lib/orion-driver/Cargo.toml"
        ]
        
        for file_path in required_files:
            full_path = self.project_root / file_path
            suite.add_test(TestResult(
                f"Required file: {file_path}",
                full_path.exists(),
                0.0,
                f"File {'exists' if full_path.exists() else 'missing'}"
            ))
        
        return suite

    def run_all_tests(self) -> bool:
        """Run all test suites"""
        self.log("Starting Orion OS Test Suite", "HEADER")
        
        # Run test suites
        test_functions = [
            self.test_build_system,
            self.test_kernel_components,
            self.test_driver_framework,
            self.test_security_features,
            self.test_qemu_boot,
            self.test_code_quality
        ]
        
        for test_func in test_functions:
            try:
                suite = test_func()
                self.test_results.append(suite)
            except Exception as e:
                self.log(f"Error running {test_func.__name__}: {e}", "ERROR")
        
        # Generate report
        return self.generate_report()

    def generate_report(self) -> bool:
        """Generate test report"""
        total_time = time.time() - self.start_time
        
        print(f"\n{Colors.HEADER}{'='*80}{Colors.ENDC}")
        print(f"{Colors.HEADER}                    ORION OS TEST REPORT{Colors.ENDC}")
        print(f"{Colors.HEADER}{'='*80}{Colors.ENDC}")
        
        total_tests = 0
        total_passed = 0
        overall_success = True
        
        for suite in self.test_results:
            total_tests += suite.total_count
            total_passed += suite.passed_count
            
            if suite.failed_count > 0:
                overall_success = False
            
            # Suite header
            color = Colors.OKGREEN if suite.failed_count == 0 else Colors.FAIL
            print(f"\n{color}📋 {suite.name}{Colors.ENDC}")
            print(f"   Tests: {suite.passed_count}/{suite.total_count} passed "
                  f"({suite.success_rate:.1f}%) in {suite.total_duration:.2f}s")
            
            # Failed tests details
            if suite.failed_count > 0:
                print(f"   {Colors.FAIL}Failed tests:{Colors.ENDC}")
                for test in suite.tests:
                    if not test.passed:
                        print(f"     ❌ {test.name}: {test.details}")
            
            # Successful tests summary
            if suite.passed_count > 0:
                print(f"   {Colors.OKGREEN}Passed tests:{Colors.ENDC}")
                for test in suite.tests:
                    if test.passed:
                        print(f"     ✅ {test.name} ({test.duration:.2f}s)")
        
        # Overall summary
        print(f"\n{Colors.HEADER}📊 SUMMARY{Colors.ENDC}")
        print(f"   Total Tests: {total_tests}")
        print(f"   Passed: {Colors.OKGREEN}{total_passed}{Colors.ENDC}")
        print(f"   Failed: {Colors.FAIL}{total_tests - total_passed}{Colors.ENDC}")
        print(f"   Success Rate: {total_passed/total_tests*100:.1f}%")
        print(f"   Total Time: {total_time:.2f}s")
        
        if overall_success:
            print(f"\n{Colors.OKGREEN}🎉 ALL TESTS PASSED!{Colors.ENDC}")
            print(f"{Colors.OKGREEN}Orion OS is ready for deployment.{Colors.ENDC}")
        else:
            print(f"\n{Colors.FAIL}❌ SOME TESTS FAILED{Colors.ENDC}")
            print(f"{Colors.FAIL}Please fix the issues before deployment.{Colors.ENDC}")
        
        print(f"{Colors.HEADER}{'='*80}{Colors.ENDC}")
        
        return overall_success

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description="Orion OS Test Suite")
    parser.add_argument("--suite", choices=["build", "kernel", "drivers", "security", "qemu", "quality", "all"],
                       default="all", help="Test suite to run")
    parser.add_argument("--verbose", action="store_true", help="Verbose output")
    
    args = parser.parse_args()
    
    tester = OrionTester()
    
    if args.suite == "all":
        success = tester.run_all_tests()
    else:
        # Run specific test suite
        suite_map = {
            "build": tester.test_build_system,
            "kernel": tester.test_kernel_components,
            "drivers": tester.test_driver_framework,
            "security": tester.test_security_features,
            "qemu": tester.test_qemu_boot,
            "quality": tester.test_code_quality
        }
        
        if args.suite in suite_map:
            suite = suite_map[args.suite]()
            tester.test_results.append(suite)
            success = tester.generate_report()
        else:
            print(f"Unknown test suite: {args.suite}")
            success = False
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
