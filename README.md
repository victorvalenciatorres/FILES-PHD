# FILES-PHD

O2 TUTO:
https://indico.cern.ch/event/1267433/timetable/#20230417.detailed

//////////////////////////////   Compile O2Physics. //////////////////////////////////////////////////////////////
At: 
   /Users/valencia/alice
do: 
    aliBuild build O2Physics

///////////////////////////////////// EXECUTE ///////////////////////////////////////////////////

At:       
cd   /Users/valencia/alice/O2Physics/Tutorials/PWGMM
Do:
o2-analysistutorial-mm-my-example-task --aod-file AO2D.root



///////////////////////// WITH JSON FILE ///////////////////////////////////////////////////////
do:
   o2-analysistutorial-mm-my-example-task --configuration json://myConfig.json


////////////////////////////////////////////////////////////
Do: 

o2-analysistutorial-mm-my-example-task2 --configuration json://myConfig.json | o2-analysis-track-propagation --configuration json://myConfig.json | o2-analysis-timestamp --configuration json://myConfig.json

o2-analysistutorial-mm-my-example-task3 --configuration json://myConfig.json | o2-analysis-track-propagation --configuration json://myConfig.json | o2-analysis-timestamp --configuration json://myConfig.json

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


List of tables defined in the AO2D data files:
https://aliceo2group.github.io/analysis-framework/docs/datamodel/ao2dTables.html

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
