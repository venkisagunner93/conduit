import re
from argparse import ArgumentParser, Namespace
from pathlib import Path

from .command import Command


class PkgCommand(Command):
    name = "pkg"
    help = "Create a new package"

    def add_arguments(self, parser: ArgumentParser) -> None:
        parser.add_argument("name", help="Package name")
        parser.add_argument("--deps", help="Comma-separated dependencies")
        parser.add_argument("--no-tests", action="store_true", help="Skip test scaffolding")

    def run(self, args: Namespace) -> None:
        name = args.name
        deps = [d.strip() for d in args.deps.split(",")] if args.deps else []
        no_tests = args.no_tests

        # Validate name
        if not re.match(r"^[a-zA-Z][a-zA-Z0-9_]*$", name):
            raise RuntimeError(f"Invalid package name: {name} (must be alphanumeric with underscores)")

        # Check if exists
        pkg_dir = self.root / "packages" / name
        if pkg_dir.exists():
            raise RuntimeError(f"Package already exists: {pkg_dir}")

        # Warn if deps don't exist
        packages_dir = self.root / "packages"
        for dep in deps:
            dep_dir = packages_dir / dep
            if not dep_dir.exists():
                print(f"Warning: dependency '{dep}' not found")

        # Create structure
        pkg_dir.mkdir(parents=True)
        (pkg_dir / "include" / name).mkdir(parents=True)
        (pkg_dir / "src").mkdir()
        if not no_tests:
            (pkg_dir / "tests").mkdir()

        # Generate files
        self._write_conduit_toml(pkg_dir, name, deps)
        self._write_cmake(pkg_dir, name, deps, no_tests)
        self._write_header(pkg_dir, name)
        self._write_source(pkg_dir, name)
        if not no_tests:
            self._write_test(pkg_dir, name)

        print(f"Created package: {pkg_dir}")

    def _write_conduit_toml(self, pkg_dir: Path, name: str, deps: list[str]) -> None:
        lines = [
            "[package]",
            f'name = "{name}"',
            'version = "0.1.0"',
            "",
        ]

        if deps:
            lines.append("[dependencies]")
            for dep in deps:
                lines.append(f'{dep} = "*"')
            lines.append("")

        lines.extend([
            "[build]",
            'type = "cmake"',
        ])

        (pkg_dir / "conduit.toml").write_text("\n".join(lines) + "\n")

    def _write_cmake(self, pkg_dir: Path, name: str, deps: list[str], no_tests: bool) -> None:
        lines = [
            "cmake_minimum_required(VERSION 3.16)",
            f"project({name} LANGUAGES CXX)",
            "",
            "set(CMAKE_CXX_STANDARD 17)",
            "set(CMAKE_CXX_STANDARD_REQUIRED ON)",
            "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)",
            "",
        ]

        # Find dependencies
        for dep in deps:
            lines.append(f"find_package({dep} REQUIRED)")
        if deps:
            lines.append("")

        # Library
        lines.extend([
            f"add_library({name}",
            f"    src/{name}.cpp",
            ")",
            "",
            f"target_include_directories({name}",
            "    PUBLIC",
            "        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>",
            "        $<INSTALL_INTERFACE:include>",
            ")",
            "",
        ])

        # Link dependencies
        if deps:
            lines.append(f"target_link_libraries({name}")
            lines.append("    PUBLIC")
            for dep in deps:
                lines.append(f"        {dep}::{dep}")
            lines.append(")")
            lines.append("")

        # Install
        lines.extend([
            "# Install",
            f"install(TARGETS {name}",
            f"    EXPORT {name}Targets",
            "    LIBRARY DESTINATION lib",
            "    ARCHIVE DESTINATION lib",
            ")",
            "",
            f"install(DIRECTORY include/{name}",
            "    DESTINATION include",
            ")",
            "",
            f"install(EXPORT {name}Targets",
            f"    FILE {name}Targets.cmake",
            f"    NAMESPACE {name}::",
            f"    DESTINATION lib/cmake/{name}",
            ")",
            "",
            "include(CMakePackageConfigHelpers)",
            "",
        ])

        # Config file with dependencies
        config_content = ""
        if deps:
            config_content += "include(CMakeFindDependencyMacro)\\n"
            for dep in deps:
                config_content += f"find_dependency({dep})\\n"
        config_content += f"include(\\${{CMAKE_CURRENT_LIST_DIR}}/{name}Targets.cmake)"

        lines.extend([
            f"file(WRITE ${{CMAKE_CURRENT_BINARY_DIR}}/{name}Config.cmake",
            f'"{config_content}',
            '")',
            "",
            "install(FILES",
            f"    ${{CMAKE_CURRENT_BINARY_DIR}}/{name}Config.cmake",
            f"    DESTINATION lib/cmake/{name}",
            ")",
        ])

        # Tests
        if not no_tests:
            lines.extend([
                "",
                "# Tests",
                'option(BUILD_TESTING "Build tests" ON)',
                "if(BUILD_TESTING)",
                "    enable_testing()",
                "    find_package(GTest REQUIRED)",
                "",
                f"    add_executable({name}_test tests/{name}_test.cpp)",
                f"    target_link_libraries({name}_test {name} GTest::gtest_main)",
                f"    add_test(NAME {name}_test COMMAND {name}_test)",
                "endif()",
            ])

        lines.append("")
        (pkg_dir / "CMakeLists.txt").write_text("\n".join(lines))

    def _write_header(self, pkg_dir: Path, name: str) -> None:
        guard = name.upper() + "_HPP"
        ns = name.replace("_", "::")

        content = f"""\
#pragma once

namespace {ns} {{

// TODO: Add declarations

}}  // namespace {ns}
"""
        (pkg_dir / "include" / name / f"{name}.hpp").write_text(content)

    def _write_source(self, pkg_dir: Path, name: str) -> None:
        content = f"""\
#include "{name}/{name}.hpp"

namespace {name.replace("_", "::")} {{

// TODO: Add implementations

}}  // namespace {name.replace("_", "::")}
"""
        (pkg_dir / "src" / f"{name}.cpp").write_text(content)

    def _write_test(self, pkg_dir: Path, name: str) -> None:
        content = f"""\
#include <gtest/gtest.h>
#include "{name}/{name}.hpp"

TEST({name.title().replace("_", "")}Test, Placeholder) {{
    EXPECT_TRUE(true);
}}
"""
        (pkg_dir / "tests" / f"{name}_test.cpp").write_text(content)
