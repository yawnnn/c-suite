import os
import sys
import subprocess
import math
import json


def print_header(text: str, size: int = 60):
    corner_char = "="
    padding_char = " "
    wall_char = "|"

    text_size = len(text)
    wall_size = 2
    padding_size = (size - text_size - wall_size) / 2.0
    left_padding_size = math.floor(padding_size)
    right_padding_size = math.ceil(padding_size)

    corner = corner_char * size
    center = (
        wall_char
        + padding_char * left_padding_size
        + text
        + padding_char * right_padding_size
        + wall_char
    )

    print(corner)
    print(center)
    print(corner)


def add_lib(
    params: list[str],
    lib_name: str,
):
    lib = "-l" + lib_name
    params.append(lib)


class Builder:
    def __init__(self, src_dir: str, target_dir: str):
        self.src_dir: str = src_dir
        self.target_dir: str = target_dir

        self.compiler: str = "clang++"

        self.opt_level: str = ""
        self.defines: dict[str, str] = {}
        self.sources: list[str] = []
        self.exename: str = ""
        self.include_paths: list[str] = []
        self.link_paths: list[str] = []
        self.libraries: list[str] = []
        self.linker_options: str = ""

    def set_compiler(self, compiler: str):
        self.compiler = compiler

    def set_opt_level(self, opt_level: str):
        self.opt_level = opt_level

    def set_exename(self, exename: str):
        self.exename = exename

    def add_define(self, name: str, value: str):
        self.defines[name] = value

    def add_source(self, source: str):
        self.sources.append(source)

    def add_include_path(self, include_path: str):
        self.include_paths.append(include_path)

    def add_link_path(self, link_path: str):
        self.link_paths.append(link_path)

    def add_library(self, library: str):
        self.libraries.append(library)

    def set_linker_options(self, linker_options: str):
        self.linker_options = linker_options

    def get_executable(self) -> str:
        executable = os.path.join(self.target_dir, self.exename)

        if os.name == "nt":
            executable += ".exe"

        return executable

    def get_sources(self) -> list[str]:
        return [os.path.join(self.src_dir, src) for src in self.sources]

    def get_command(self) -> list[str]:
        command = []
        command += [self.compiler]
        command += ["-O" + self.opt_level]

        for name, value in self.defines.items():
            if value and len(value):
                command += [f"-D{name}={value}"]
            else:
                command += [f"-D{name}"]

        command += self.get_sources()
        command += [
            "-o",
            os.path.splitext(self.get_executable())[0],
        ]

        for path in self.include_paths:
            command += [f"-I{path}"]

        for path in self.link_paths:
            command += [f"-L{path}"]

        for lib in self.libraries:
            command += [f"-l{lib}"]

        command += [self.linker_options]

        return command

    def read_last_modified(self) -> dict | None:
        last_modified = None
        fname = os.path.join(self.target_dir, ".last_modified.json")

        if os.path.exists(fname):
            with open(fname, "r", encoding="utf-8") as f:
                last_modified: dict = json.load(f)

        return last_modified

    def write_last_modified(self, last_modified: dict):
        fname = os.path.join(self.target_dir, ".last_modified.json")

        with open(fname, "w", encoding="utf-8") as f:
            json.dump(last_modified, f)

    def get_current_last_modified(self) -> dict[str, float]:
        last_modified = {}

        for src in self.get_sources():
            src_name = src[len(self.src_dir) + len(os.path.sep) :]
            last_modified[src_name] = os.path.getmtime(src)

        return last_modified
    
    def did_sources_change(self) -> tuple[bool, dict[str, float]]:
        changed = True
        last_modified = self.get_current_last_modified()
        prev_last_modified = self.read_last_modified()

        if prev_last_modified:
            for src, mtime in last_modified.items():
                if src not in prev_last_modified:
                    break

                if mtime > prev_last_modified[src]:
                    break
            else:
                changed = False

        return (changed, last_modified)

    def build(self, force_build: bool) -> bool:
        last_modified = None
        run_build = force_build

        if not run_build:
            (changed, last_modified) = self.did_sources_change()
            run_build = changed

        if run_build:
            build_cmd = self.get_command()

            print_header("BUILD")
            print(" ".join(build_cmd))
            print()

            proc = subprocess.run(build_cmd)
            ok_build = proc.returncode == 0

            if ok_build and last_modified:
                self.write_last_modified(last_modified)
        else:
            ok_build = True

        return ok_build

    def run(self):
        print_header("RUN")

        run = builder.get_executable()
        subprocess.run(run)


if __name__ == "__main__":
    if os.name == "nt":
        is_windows = True
    elif os.name == "posix":
        is_linux = True
    else:
        raise AssertionError("OS not supported")

    # This file must be in the root
    os.chdir(os.path.dirname(__file__))

    # Setting up variabiles
    root_dir = "."
    src_dir = os.path.join(root_dir, "src")
    target_dir = os.path.join(root_dir, "build")

    if is_windows:
        deps_root = os.path.join(os.environ["LOCALAPPDATA"], "msys64")
    else:
        pass  # deps_root = os.path.join(root_dir, "dependencies")

    # Composing build command
    builder = Builder(src_dir, target_dir)

    builder.set_compiler("clang")
    builder.set_opt_level("2")

    builder.add_source("main.c")
    builder.add_source("vec.c")
    builder.add_source("vstr.c")
    builder.add_source("llist.c")
    builder.add_source("fixed_allocator.c")
    builder.add_source("arena_allocator.c")
    builder.add_source("tracking_allocator.c")
    builder.add_source("hashmap.c")
    
    builder.set_exename("main")

    builder.add_include_path(os.path.join(deps_root, "mingw64", "include"))
    builder.add_include_path(os.path.join(deps_root, "usr", "include"))

    builder.add_link_path(os.path.join(deps_root, "mingw64", "lib"))
    builder.add_link_path(os.path.join(deps_root, "usr", "lib"))

    builder.add_library("jansson")

    # Build and run
    os.system("cls")

    argv = sys.argv[1:]

    if argv and argv[0] == "force":
        force_build = True
        argv.pop(0)
    else:
        force_build = False

    if argv and argv[0] == "run":
        run = True
        argv.pop(0)
    else:
        run = False

    assert not argv

    ok_build = builder.build(force_build)

    if ok_build and run:
        builder.run()
