import os
import filecmp

test_dir = "./test"
tmp_file = "tmp_test.out"
splc = "./bin/splc"

g = os.walk(test_dir)

for path, dir_list, file_list in g:
    for file in file_list:
        if file.split('.')[1] == 'spl':
            pix = file.split('.')[0]
            spl = os.path.join(f'{test_dir}/{pix}.spl')
            sample = os.path.join(f'{test_dir}/{pix}.out')
            os.system(f"{splc} {spl} > {sample}")

            