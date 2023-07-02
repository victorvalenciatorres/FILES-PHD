import sys
import os

pathTo = '/alice/cern.ch/user/a/alihyperloop/jobs/0016/hy_166746' # hyperloop path for the submitted jobs
datarun = '528021' # put run number
apass = 'apass3' # pass
Nfiles = 48

for i in range(1, Nfiles):
    if (i < 10):
        i = str(i)
        print(i)
        command = "alien.py cp " + pathTo + "/AOD/00"+i + "/AO2D.root file://"+datarun+"/"+apass+"/AOD/00"+i+"/AO2D.root"
        os.system(command)
        i = int(i)
    elif (i >= 10 and i < 100):
        i = str(i)
        print(i)
        command = "alien.py cp " + pathTo + "/AOD/0"+i + "/AO2D.root file://"+datarun+"/"+apass+"/AOD/0"+i+"/AO2D.root"
        os.system(command)
        i = int(i)
    elif (i >= 100):
        i = str(i)
        print(i)
        command = "alien.py cp " + pathTo + "/AOD/"+i + "/AO2D.root file://"+datarun+"/"+apass+"/AOD/"+i+"/AO2D.root"
        os.system(command)
        i = int(i)