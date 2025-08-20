#!/usr/bin/env python3
"""
Orion Operating System - Performance Optimization Script

Comprehensive optimization and cleanup script for the Orion OS project.
Includes code analysis, performance profiling, and optimization suggestions.

Developed by Jérémy Noverraz (1988-2025)
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
import re
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import tempfile

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

class OptimizationReport:
    """Performance optimization report"""
    def __init__(self):
        self.code_issues: List[Dict] = []
        self.performance_issues: List[Dict] = []
        self.security_issues: List[Dict] = []
        self.suggestions: List[Dict] = []
        self.metrics: Dict = {}

class OrionOptimizer:
    """Main optimization and profiling tool for Orion OS"""
    
    def __init__(self):
        self.project_root = Path(__file__).parent.absolute()
        self.report = OptimizationReport()
        
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
    
    def run_command(self, cmd: List[str], cwd: Optional[Path] = None) -> Tuple[bool, str, str]:
        """Run a command and return success status, stdout, stderr"""
        try:
            result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, timeout=60)
            return result.returncode == 0, result.stdout, result.stderr
        except subprocess.TimeoutExpired:
            return False, "", "Command timed out"
        except Exception as e:
            return False, "", str(e)

    def analyze_code_quality(self):
        """Analyze code quality and find optimization opportunities"""
        self.log("Analyzing code quality", "HEADER")
        
        # Find potential performance issues in C code
        c_files = list(self.project_root.glob("kernel/**/*.[ch]"))
        c_files.extend(self.project_root.glob("bootloader/**/*.[ch]"))
        
        for file_path in c_files:
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    lines = content.split('\n')
                    
                    for line_num, line in enumerate(lines, 1):
                        # Check for potential performance issues
                        self._check_performance_patterns(file_path, line_num, line)
                        
                        # Check for security issues
                        self._check_security_patterns(file_path, line_num, line)
                        
                        # Check for code quality issues
                        self._check_quality_patterns(file_path, line_num, line)
                        
            except Exception as e:
                self.log(f"Error analyzing {file_path}: {e}", "WARNING")

    def _check_performance_patterns(self, file_path: Path, line_num: int, line: str):
        """Check for performance anti-patterns"""
        line_clean = line.strip()
        
        # Inefficient patterns
        patterns = [
            (r'\bmalloc\s*\(', "Use kmalloc instead of malloc in kernel code"),
            (r'\bsprintf\s*\(', "sprintf is unsafe, use snprintf"),
            (r'\bstrcpy\s*\(', "strcpy is unsafe, use strncpy"),
            (r'\bstrcat\s*\(', "strcat is unsafe, use strncat"),
            (r'for\s*\([^;]*;[^;]*\+\+[^;]*;[^)]*\)\s*{[^}]*malloc', "Memory allocation in loop - consider pre-allocation"),
            (r'\bprintf\s*\(.*"\s*\\n"\s*\)', "Use kinfo/kwarning/kerror instead of printf"),
            (r'while\s*\(1\)', "Infinite loop detected - ensure proper exit conditions"),
            (r'\bvolatile\s+(?!atomic)', "Consider atomic operations instead of volatile for synchronization"),
        ]
        
        for pattern, suggestion in patterns:
            if re.search(pattern, line_clean):
                self.report.performance_issues.append({
                    'file': str(file_path.relative_to(self.project_root)),
                    'line': line_num,
                    'issue': line_clean,
                    'suggestion': suggestion,
                    'severity': 'medium'
                })

    def _check_security_patterns(self, file_path: Path, line_num: int, line: str):
        """Check for security issues"""
        line_clean = line.strip()
        
        # Security patterns
        patterns = [
            (r'\bgets\s*\(', "gets() is unsafe, use fgets()"),
            (r'\bscanf\s*\([^,]*,\s*"%s"', "Unbounded string input, use length-limited format"),
            (r'char\s+\w+\[\d+\].*scanf', "Fixed buffer with scanf - potential overflow"),
            (r'memcpy\s*\([^,]*,[^,]*,[^)]*\+', "Potential integer overflow in memcpy size"),
            (r'__asm__.*cli.*', "Disabling interrupts - ensure proper re-enabling"),
            (r'#define.*\(.*\).*\\\s*$', "Multi-line macro without do-while"),
        ]
        
        for pattern, suggestion in patterns:
            if re.search(pattern, line_clean):
                self.report.security_issues.append({
                    'file': str(file_path.relative_to(self.project_root)),
                    'line': line_num,
                    'issue': line_clean,
                    'suggestion': suggestion,
                    'severity': 'high'
                })

    def _check_quality_patterns(self, file_path: Path, line_num: int, line: str):
        """Check for code quality issues"""
        line_clean = line.strip()
        
        # Code quality patterns
        patterns = [
            (r'^[^/]*//.*TODO.*', "TODO comment found"),
            (r'^[^/]*//.*FIXME.*', "FIXME comment found"),
            (r'^[^/]*//.*HACK.*', "HACK comment found"),
            (r'^\s*if\s*\([^)]*\)\s*;', "Empty if statement"),
            (r'^\s*}\s*else\s*{[^}]*}$', "Single-line else block could be simplified"),
            (r'[^\w]0x[0-9a-fA-F]+[^\w]', "Magic number detected - consider named constant"),
        ]
        
        for pattern, suggestion in patterns:
            if re.search(pattern, line_clean):
                self.report.code_issues.append({
                    'file': str(file_path.relative_to(self.project_root)),
                    'line': line_num,
                    'issue': line_clean,
                    'suggestion': suggestion,
                    'severity': 'low'
                })

    def analyze_binary_size(self):
        """Analyze binary sizes and suggest optimizations"""
        self.log("Analyzing binary sizes", "HEADER")
        
        kernel_path = self.project_root / "build" / "kernel" / "orion-kernel.elf"
        if kernel_path.exists():
            kernel_size = kernel_path.stat().st_size
            self.report.metrics['kernel_size'] = kernel_size
            
            # Check if kernel size is reasonable
            if kernel_size > 10 * 1024 * 1024:  # 10MB
                self.report.suggestions.append({
                    'category': 'binary_size',
                    'issue': f'Kernel size is large: {kernel_size / 1024 / 1024:.1f}MB',
                    'suggestion': 'Consider enabling link-time optimization (-flto)',
                    'impact': 'high'
                })
            
            # Analyze sections
            success, stdout, stderr = self.run_command(['objdump', '-h', str(kernel_path)])
            if success:
                self._analyze_elf_sections(stdout)

    def _analyze_elf_sections(self, objdump_output: str):
        """Analyze ELF sections for optimization opportunities"""
        lines = objdump_output.split('\n')
        large_sections = []
        
        for line in lines:
            if 'CONTENTS' in line:
                parts = line.split()
                if len(parts) >= 3:
                    try:
                        section_name = parts[1]
                        size_hex = parts[2]
                        size = int(size_hex, 16)
                        
                        if size > 1024 * 1024:  # 1MB
                            large_sections.append((section_name, size))
                    except (ValueError, IndexError):
                        continue
        
        for section_name, size in large_sections:
            self.report.suggestions.append({
                'category': 'binary_size',
                'issue': f'Large section {section_name}: {size / 1024:.1f}KB',
                'suggestion': f'Investigate if {section_name} can be optimized or compressed',
                'impact': 'medium'
            })

    def check_performance_hotspots(self):
        """Check for potential performance hotspots"""
        self.log("Checking performance hotspots", "HEADER")
        
        # Look for patterns that might cause performance issues
        hotspot_patterns = [
            ('kernel/mm/', 'Memory management - critical path'),
            ('kernel/core/scheduler.c', 'Scheduler - runs frequently'),
            ('kernel/core/syscalls.c', 'System calls - user-kernel boundary'),
            ('kernel/core/ipc.c', 'IPC - inter-process communication'),
        ]
        
        for pattern, description in hotspot_patterns:
            files = list(self.project_root.glob(f"{pattern}*"))
            for file_path in files:
                if file_path.is_file():
                    self._analyze_hotspot_file(file_path, description)

    def _analyze_hotspot_file(self, file_path: Path, description: str):
        """Analyze a performance-critical file"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
                
                # Count potential performance concerns
                loop_count = len(re.findall(r'\b(for|while)\s*\(', content))
                malloc_count = len(re.findall(r'\b(kmalloc|malloc)\s*\(', content))
                lock_count = len(re.findall(r'\b(spin_lock|mutex_lock|lock)\s*\(', content))
                
                if loop_count > 10:
                    self.report.suggestions.append({
                        'category': 'performance',
                        'issue': f'{file_path.name}: High loop count ({loop_count})',
                        'suggestion': 'Review loops for optimization opportunities',
                        'impact': 'medium'
                    })
                
                if malloc_count > 5:
                    self.report.suggestions.append({
                        'category': 'performance',
                        'issue': f'{file_path.name}: Many allocations ({malloc_count})',
                        'suggestion': 'Consider object pooling or pre-allocation',
                        'impact': 'high'
                    })
                
                if lock_count > 15:
                    self.report.suggestions.append({
                        'category': 'performance',
                        'issue': f'{file_path.name}: Many locks ({lock_count})',
                        'suggestion': 'Review for lock contention and consider lock-free alternatives',
                        'impact': 'high'
                    })
                    
        except Exception as e:
            self.log(f"Error analyzing hotspot {file_path}: {e}", "WARNING")

    def cleanup_project(self):
        """Clean up project files and optimize structure"""
        self.log("Cleaning up project", "HEADER")
        
        # Remove common build artifacts
        cleanup_patterns = [
            "**/*.o",
            "**/*.obj", 
            "**/*.a",
            "**/*.so",
            "**/*.dll",
            "**/*.tmp",
            "**/*.bak",
            "**/*~",
            "**/core",
            "**/a.out",
            "**/.DS_Store",
            "**/Thumbs.db",
            "**/target/debug/**",
            "**/target/release/deps/**/*.d",
        ]
        
        cleaned_count = 0
        for pattern in cleanup_patterns:
            for file_path in self.project_root.glob(pattern):
                if file_path.is_file():
                    try:
                        file_path.unlink()
                        cleaned_count += 1
                    except Exception as e:
                        self.log(f"Could not remove {file_path}: {e}", "WARNING")
        
        if cleaned_count > 0:
            self.log(f"Cleaned up {cleaned_count} files", "SUCCESS")
        else:
            self.log("No cleanup needed", "SUCCESS")

    def optimize_rust_builds(self):
        """Optimize Rust build configurations"""
        self.log("Optimizing Rust builds", "HEADER")
        
        cargo_files = list(self.project_root.glob("**/Cargo.toml"))
        for cargo_file in cargo_files:
            self._optimize_cargo_toml(cargo_file)

    def _optimize_cargo_toml(self, cargo_path: Path):
        """Optimize a specific Cargo.toml file"""
        try:
            with open(cargo_path, 'r') as f:
                content = f.read()
            
            # Check for optimization opportunities
            if '[profile.release]' not in content:
                self.report.suggestions.append({
                    'category': 'rust_optimization',
                    'issue': f'{cargo_path.relative_to(self.project_root)}: Missing release profile optimization',
                    'suggestion': 'Add [profile.release] section with lto = true, codegen-units = 1',
                    'impact': 'medium'
                })
            
            if 'panic = "abort"' not in content:
                self.report.suggestions.append({
                    'category': 'rust_optimization', 
                    'issue': f'{cargo_path.relative_to(self.project_root)}: Using default panic strategy',
                    'suggestion': 'Add panic = "abort" for smaller binary size',
                    'impact': 'low'
                })
                
        except Exception as e:
            self.log(f"Error analyzing {cargo_path}: {e}", "WARNING")

    def generate_optimization_report(self):
        """Generate comprehensive optimization report"""
        self.log("Generating optimization report", "HEADER")
        
        print(f"\n{Colors.HEADER}{'='*80}{Colors.ENDC}")
        print(f"{Colors.HEADER}                    ORION OS OPTIMIZATION REPORT{Colors.ENDC}")
        print(f"{Colors.HEADER}{'='*80}{Colors.ENDC}")
        
        # Performance Issues
        if self.report.performance_issues:
            print(f"\n{Colors.WARNING}⚡ PERFORMANCE ISSUES ({len(self.report.performance_issues)}){Colors.ENDC}")
            for issue in self.report.performance_issues[:10]:  # Show top 10
                print(f"   📁 {issue['file']}:{issue['line']}")
                print(f"      ❌ {issue['issue'][:80]}...")
                print(f"      💡 {issue['suggestion']}")
                print()
        
        # Security Issues
        if self.report.security_issues:
            print(f"\n{Colors.FAIL}🔒 SECURITY ISSUES ({len(self.report.security_issues)}){Colors.ENDC}")
            for issue in self.report.security_issues:
                print(f"   📁 {issue['file']}:{issue['line']}")
                print(f"      ❌ {issue['issue'][:80]}...")
                print(f"      💡 {issue['suggestion']}")
                print()
        
        # Code Quality Issues
        if self.report.code_issues:
            print(f"\n{Colors.OKCYAN}📝 CODE QUALITY ({len(self.report.code_issues)}){Colors.ENDC}")
            todo_count = sum(1 for issue in self.report.code_issues if 'TODO' in issue['issue'])
            fixme_count = sum(1 for issue in self.report.code_issues if 'FIXME' in issue['issue'])
            print(f"   📋 TODO comments: {todo_count}")
            print(f"   🔧 FIXME comments: {fixme_count}")
            print(f"   📊 Other issues: {len(self.report.code_issues) - todo_count - fixme_count}")
        
        # Optimization Suggestions
        if self.report.suggestions:
            print(f"\n{Colors.OKGREEN}💡 OPTIMIZATION SUGGESTIONS{Colors.ENDC}")
            
            # Group by category
            by_category = {}
            for suggestion in self.report.suggestions:
                category = suggestion['category']
                if category not in by_category:
                    by_category[category] = []
                by_category[category].append(suggestion)
            
            for category, suggestions in by_category.items():
                print(f"\n   🏷️  {category.upper().replace('_', ' ')}")
                for suggestion in suggestions[:5]:  # Show top 5 per category
                    impact_color = Colors.FAIL if suggestion['impact'] == 'high' else Colors.WARNING if suggestion['impact'] == 'medium' else Colors.OKBLUE
                    print(f"      {impact_color}[{suggestion['impact'].upper()}]{Colors.ENDC} {suggestion['suggestion']}")
        
        # Metrics
        if self.report.metrics:
            print(f"\n{Colors.HEADER}📊 METRICS{Colors.ENDC}")
            for metric, value in self.report.metrics.items():
                if metric == 'kernel_size':
                    print(f"   💾 Kernel Size: {value / 1024 / 1024:.1f}MB")
        
        # Summary
        total_issues = len(self.report.performance_issues) + len(self.report.security_issues) + len(self.report.code_issues)
        print(f"\n{Colors.HEADER}📋 SUMMARY{Colors.ENDC}")
        print(f"   Total Issues Found: {total_issues}")
        print(f"   Optimization Suggestions: {len(self.report.suggestions)}")
        print(f"   Security Issues: {Colors.FAIL}{len(self.report.security_issues)}{Colors.ENDC}")
        print(f"   Performance Issues: {Colors.WARNING}{len(self.report.performance_issues)}{Colors.ENDC}")
        print(f"   Code Quality Issues: {Colors.OKCYAN}{len(self.report.code_issues)}{Colors.ENDC}")
        
        if total_issues == 0:
            print(f"\n{Colors.OKGREEN}🎉 EXCELLENT CODE QUALITY!{Colors.ENDC}")
            print(f"{Colors.OKGREEN}No major issues found. Orion OS is well-optimized.{Colors.ENDC}")
        elif len(self.report.security_issues) == 0:
            print(f"\n{Colors.OKGREEN}✅ SECURITY LOOKS GOOD{Colors.ENDC}")
            print(f"{Colors.OKBLUE}Consider addressing performance and quality issues for optimal results.{Colors.ENDC}")
        else:
            print(f"\n{Colors.FAIL}⚠️  SECURITY ISSUES NEED ATTENTION{Colors.ENDC}")
            print(f"{Colors.FAIL}Please address security issues before deployment.{Colors.ENDC}")
        
        print(f"{Colors.HEADER}{'='*80}{Colors.ENDC}")

    def run_optimization(self):
        """Run complete optimization suite"""
        start_time = time.time()
        
        self.log("Starting Orion OS optimization", "HEADER")
        
        # Run all optimization checks
        self.cleanup_project()
        self.analyze_code_quality()
        self.analyze_binary_size()
        self.check_performance_hotspots()
        self.optimize_rust_builds()
        
        # Generate report
        self.generate_optimization_report()
        
        duration = time.time() - start_time
        self.log(f"Optimization completed in {duration:.2f} seconds", "SUCCESS")

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description="Orion OS Optimization Tool")
    parser.add_argument("--action", choices=["analyze", "cleanup", "optimize", "all"],
                       default="all", help="Optimization action to perform")
    parser.add_argument("--format", choices=["text", "json"], default="text",
                       help="Output format")
    
    args = parser.parse_args()
    
    optimizer = OrionOptimizer()
    
    if args.action == "cleanup":
        optimizer.cleanup_project()
    elif args.action == "analyze":
        optimizer.analyze_code_quality()
        optimizer.analyze_binary_size()
        optimizer.check_performance_hotspots()
        optimizer.generate_optimization_report()
    elif args.action == "optimize":
        optimizer.optimize_rust_builds()
    else:  # all
        optimizer.run_optimization()

if __name__ == "__main__":
    main()
