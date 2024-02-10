import os.path

from utils import cmd
from consts import print_project_label, build_folder, project_name, is_windows

print_project_label("Run")

if is_windows:
    cmd(os.path.join(build_folder, "bin", "Release", project_name + ".exe"))
else:
    cmd(os.path.join(build_folder, "bin", project_name))
