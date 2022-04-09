import os
import sys
import subprocess as sb
from contextlib import contextmanager

exec_glslc = r'C:\VulkanSDK\1.3.204.1\Bin\glslc.exe'
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

def GenerateShaders(output_mode='release'):
	folder_shader = os.path.join(this_dir,'src','shaders')
	folder_output = os.path.join(this_dir,'src',output_mode,'shaders')
	with cwd(folder_output):
		for file in os.listdir(folder_shader):
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
