import os
import datetime

def test(file):
    system_files_cache = {}
    for d in os.environ["PATH"].split(";"):
        if os.path.exists(d):
            for f in os.listdir(d):
                system_files_cache[f] = os.path.join(d,f)
    if not os.path.exists(file):
        if not file in system_files_cache:
            if file.endswith('.lib'):
                file = file.replace('.lib','.dll')
            if not file in system_files_cache:
                raise Exception("File (%s) can't be found in project or system files" % (file))
        file = system_files_cache[file]
    return datetime.datetime.fromtimestamp(os.path.getmtime(file)).strftime('%Y-%m-%d %H:%M:%S')

if 0:
    system_files = {}
    for d in os.environ["PATH"].split(";"):
        if os.path.exists(d):
            for f in os.listdir(d):
                system_files[f] = os.path.join(d,f)
                if 'user32.dll' in f:
                    print(os.path.join(d,f))

    print(len(system_files))
    #print(system_files)

dt = test('user32.dll')
print(dt)
