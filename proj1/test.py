import os
import filecmp

test_dir = "./test-sample"
tmp_file = "tmp_test.out"
splc = "./bin/splc"

g = os.walk(test_dir)

for path, dir_list, file_list in g:
    for file in file_list:
        if file.split('.')[1] == 'spl':
            pix = file.split('.')[0]
            spl = os.path.join(f'{test_dir}/{pix}.spl')
            sample = os.path.join(f'{test_dir}/{pix}.out')
            os.system(f"{splc} {spl} > {tmp_file}")

            try:
                status = filecmp.cmp(tmp_file, sample)
                if status:
                    print(f'pass test: {pix}')
                else:
                    print(f'failed test: {pix}')
            except IOError:
                print('Error: {splc} output not found')

            