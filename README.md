# Reinterpretation of CMS-EXO-22-024
This repository is organized to collect all data related to the reinterpretation of the analysis available at                             
hepdata link : https://www.hepdata.net/record/ins2787227                                                     
Cadi line : https://cms.cern.ch/iCMS/analysisadmin/cadilines?line=EXO-22-024

with Madanalysis: https://github.com/MadAnalysis 
# Files Structure
1) .cpp file : contains all relevent information (Definition of signal regions and respective cuts) to reproduce the cutflow from analysis. For details see section 4.3 : https://arxiv.org/pdf/1808.00480

2)  .info file : contains Data and Standard Model background information and later this is used to calculate limits

3)  .h file :  its the C++ header file

4)  delphes.tcl : Contains the configuration of Delphes associated with a analysis

# How to use 
1) Download Madnalayis version : 1.9.60 and extract into desired folder
2) Go into the madanalysis5 folder. Then run this command:
./bin/ma5 It will check if everything is okay.

3) Type these commands one by one to get the packages you need:

install zlib

install fastjet

install delphes

install PAD 

install PADForMA5tune

4) If Root is not set up on your computer, also type:
install root

5) When that’s done, close MadAnalysis and start it again in "reco mode" by typing:
./bin/ma5 -R
This will check that all parts are installed correctly.

6) To run analysis copy .cpp, .h and .info file into the tools/PAD/Build/SampleAnalyzer/User/Analyzer folder. And delphes file into tools/PAD/Input/Cards folder.
7) Now go to Build directory and type 

source setup.sh and then 

make

Now PAD executable is being created. 





