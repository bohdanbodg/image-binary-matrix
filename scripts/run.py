from utils import cmd
from consts import print_project_label, build_folder, project_name

print_project_label("Run")

cmd("./{0}/bin/{1}".format(build_folder, project_name))
