import os.path

from utils import cmd
from consts import project_name, print_project_label, is_windows, build_folder

print_project_label("Build")

build_params = "-s compiler.cppstd=17 -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True --build=missing"

cmd("conan install . {0}".format(build_params))

executable_file = os.path.join(
    build_folder, "bin", "Release", project_name + ".exe"
) if is_windows else os.path.join(build_folder, "bin", project_name)
if not os.path.isfile(executable_file):
    if is_windows:
        cmd("cmake --preset conan-default")
    else:
        cmd("cmake --preset conan-release")

    cmd("cmake --build --preset conan-release")

    if is_windows:
        cmd("rd /s /q {}".format(build_folder))
    else:
        cmd("rm -rf {}".format(build_folder))

    cmd("conan install . {0}".format(build_params))

cmake_policy = "-DCMAKE_POLICY_DEFAULT_CMP0091=NEW"
cmake_toolchain = "generators/conan_toolchain.cmake"

prebuild_str = "cmake -S . -B {0} -G \"{1}\" -DCMAKE_TOOLCHAIN_FILE={2}{3} {4} {5}"
build_str = "cmake --build {0} {1}"

if is_windows:
    cmd(prebuild_str.format(
        build_folder, "Visual Studio 17 2022", "./", cmake_toolchain, cmake_policy, ""
    ))
    cmd(build_str.format(build_folder, "--config Release"))
else:
    cmd(prebuild_str.format(
        build_folder, "Unix Makefiles", "Release/", cmake_toolchain, cmake_policy, "-DCMAKE_BUILD_TYPE=Release"
    ))
    cmd(build_str.format(build_folder, ""))
