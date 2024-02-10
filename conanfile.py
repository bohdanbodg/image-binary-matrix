import os

from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.files import copy

from scripts.consts import build_folder

class ImageBinaryMatrix(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("libpng/1.6.42", override=True) # opencv/4.8.1 requires libpng/1.6.40

        self.requires("glew/2.2.0")
        self.requires("glfw/3.3.8")
        self.requires("imgui/1.90.1")
        self.requires("opencv/4.8.1")

    def generate(self):
        self.custom_copy_lib_includes("glew", ["include"])
        self.custom_copy_lib_includes("glfw", ["include"])
        self.custom_copy_lib_includes("imgui", ["include"])
        self.custom_copy_lib_includes("imgui", ["res", "bindings"], "*opengl3*", ["bindings"])
        self.custom_copy_lib_includes("imgui", ["res", "bindings"], "*glfw*", ["bindings"])
        self.custom_copy_lib_includes("opencv", ["include", "opencv4"])

        copy(
            self,
            "*",
            os.path.join(self.source_folder, "res"),
            os.path.join(self.source_folder, build_folder, "bin", "input")
        )

    def custom_copy_lib_includes(self, lib, fromPath, pattern = "*", toPath = []):
        lib_folder = self.dependencies[lib].package_folder
        copy(
            self,
            pattern,
            os.path.join(lib_folder, *fromPath),
            os.path.join(self.source_folder, "libs", lib, *toPath)
        )

    def layout(self):
        cmake_layout(self)
