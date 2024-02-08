import platform

is_windows = platform.system() == "Windows"
build_folder = "build"
project_name = "image-binary-matrix"
project_label = "Image Binary Matrix"

def print_project_label(addition = ""):
    output = project_label

    if addition is not "":
        output = "{0} : {1}".format(project_label, addition)

    print(output)
