import sys
import os
# for 001 002 etc

pathToBase = '/alice/cern.ch/user/a/alihyperloop/jobs/0016/'
apass = 'apass3' 
Nfiles = 48

file_run_mapping = {
    "hy_166743": "528232",
    "hy_166742": "528231",
    "hy_166741": "528110",
    "hy_166740": "528109",
    "hy_166739": "528105",
    "hy_166738": "528097",
    "hy_166737": "528094",
    "hy_166736": "528093",
    "hy_166735": "528026",
    "hy_166734": "528021",
    "hy_166733": "527979",
    "hy_166732": "527978",
}

for filename in file_run_mapping:
    pathTo = pathToBase + filename + "/AOD" # hyperloop path for the submitted jobs
    datarun = file_run_mapping[filename] # get run number from mapping

    for i in range(1, Nfiles+1):
        if i < 10:
            num_str = f"00{i}"
        elif i < 100:
            num_str = f"0{i}"
        else:
            num_str = str(i)
        print(num_str)
        command = f"alien.py cp {pathTo}/{num_str}/AO2D.root file://{datarun}/{apass}/AOD/{num_str}/AO2D.root"
        print(command)
        os.system(command)
