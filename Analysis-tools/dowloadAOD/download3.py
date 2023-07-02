import sys
import os

#for 0001 0001 0001 etc. LHC1Operiod

pathTo = '/alice/cern.ch/user/a/alihyperloop/jobs/0016/hy_161100' # hyperloop path for the submitted jobs
datarun = '18097' # put run number
apass = 'apass1' # pass
Nfiles = 1

for i in range(1, Nfiles+1):
    if i < 10:
        num_str = f"000{i}"
    elif i < 100:
        num_str = f"00{i}"
    else:
        num_str = f"0{i}"
    print(num_str)
    command = f"alien.py cp {pathTo}/{num_str}/AnalysisResults.root file://{datarun}/{apass}/AOD/{num_str}/AnalysisResults.root"
    os.system(command)


