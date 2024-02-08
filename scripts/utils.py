import subprocess
import time

def cmd(command, error=False):
    print("Running: {}".format(command))
    start_time = time.time()

    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True, text=True)

    output = ''
    
    for line in iter(process.stdout.readline, ''):
        print(line, end='', flush=True)
        output += line

    ret = process.wait()
    end_time = time.time()
    
    elapsed_time = end_time - start_time
    print(f"Elapsed time: {elapsed_time:.2f} seconds")

    if ret != 0 and not error:
        raise Exception(f"Failed command: {command}\n{output}")
    elif ret == 0 and error:
        raise Exception(f"command succeeded (failure expected): {command}\n{output}")

    return output