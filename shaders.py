import os
import sys
import subprocess as sb
from contextlib import contextmanager

this_dir = os.path.dirname(__file__)

@contextmanager
def cwd(path):
    oldpwd=os.getcwd()
    if not os.path.exists(path): os.mkdir(path)
    os.chdir(path)
    try:
        yield
    finally:
        os.chdir(oldpwd)

def FindVulkanSDK():
    default_vulkan_path = 'C:\\VUlkanSDK\\'
    current_dir = None
    current_max = 0
    for d in os.listdir(default_vulkan_path):
        try:
            v = d.split('.')
            m = ((1 << 16) << int(v[0]))
            m = m + ((1 << 8) << int(v[1]))
            m = m + int(v[2])
            m = m + int(v[3])
            if max(m,current_max) == m:
                current_max = m
                current_dir = d
        except Exception as e:
            print(e)

    if (current_dir):
        p = os.path.join(default_vulkan_path,current_dir)
        print("Using vulkan %s" % (p))
        return p
    else:
        raise Exception( "Invalid VulkanSDK configuration. Expecting path in %s".format(default_vulkan_path) )




def GenerateShaders(output_mode='release'):
    folder_shader = os.path.join(this_dir,'src','shaders')
    folder_output = os.path.join(this_dir,'src',output_mode,'shaders')
    print("Output shader files to ... %s" % (folder_output))
    exec_glslc = os.path.join(FindVulkanSDK(), 'Bin','glslc.exe')
    with cwd(folder_output):
        for file in os.listdir(folder_shader):
            print("Working on file %s" % (file))
            if file[-2:] != '.h':
                abs_path_file = os.path.abspath(os.path.join(folder_shader,file))
                #abs_path_output = os.path.abspath(os.path.join(folder_output,file[0:file.index('.')] + '.spv'))
                abs_path_output = folder_output
                args = [exec_glslc,'-c',abs_path_file]#,'-o',abs_path_output]
                sb.call(args)


if __name__ == '__main__':
	args = sys.argv
	output_mode = 'release'
	if len(args) > 1:
		if (args[1] == 'd'):
			output_mode = 'debug'

	GenerateShaders(output_mode)
