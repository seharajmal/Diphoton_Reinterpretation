# Reinterpretation of CMS-EXO-22-024
This repository is organized to collect all data related to the reinterpretation of the analysis available at                             
hepdata link : https://www.hepdata.net/record/ins2787227                                                     
Cadi line : https://cms.cern.ch/iCMS/analysisadmin/cadilines?line=EXO-22-024

with Madanalysis: https://github.com/MadAnalysis 
# Files Structure
- .cpp file : contains all relevent information (Definition of signal regions and respective cuts) to reproduce the cutflow from analysis. For details see section 4.3 : https://arxiv.org/pdf/1808.00480

-  .info file : contains Data and Standard Model background information and later this is used to calculate limits

-  .h file :  its the C++ header file

-  delphes.tcl : Contains the configuration of Delphes associated with a analysis

# How to use 
- Download Madnalayis version : 1.9.60 and extract into desired folder
- Go into the madanalysis5 folder. Then run this command:
./bin/ma5 It will check if everything is okay.

- Type these commands one by one to get the packages you need:
  ```bash
  install zlib
  
install fastjet
  
install delphes
  
install PAD 
  
install PADForMA5tune
  ```
- If Root is not set up on your computer, also type:
install root

- When that’s done, close MadAnalysis and start it again in "reco mode" by typing:
./bin/ma5 -R
This will check that all parts are installed correctly.

- To run analysis copy .cpp, .h and .info file into the tools/PAD/Build/SampleAnalyzer/User/Analyzer folder. And delphes file into tools/PAD/Input/Cards folder.
- Now go to Build directory and type 

source setup.sh and then 

make

Now PAD executable is being created. 

# Validation
For validation of the analysis implementation, signal samples must be produced using the identical settings as the original analysis. The generation code (which produces HepMC files) is provided in the Validation_material folder. Before running, be sure to modify the number of events, cross-section, mass, and width to match those used in the analysis. For the diphoton final state, pre-existing samples are available at the following link: https://cms-pdmv-prod.web.cern.ch/mcm/requests?tags=EXO-Camilo-0011&page=0&shown=127

## Setup

- Install [Pythia8](https://pythia.org/)
- Install [LHAPDF](https://www.lhapdf.org/) with Pythia support
- Build HepMC:
  ```bash
  cmake -DCMAKE_INSTALL_PREFIX=../ -Dmomentum:STRING=GEV -Dlength:STRING=CM ../hepmc2.06.09 && make && make install
  ```
- Configure Pythia:
  ```bash
  ./configure --with-hepmc2=../ --with-lhapdf6=../LHAPDF-6.5.5
  ```
- Install PDF:
  ```bash
  lhapdf install NNPDF23_lo_as_0119_qed
  ```
- Export paths and compile:
  ```bash
  export LD_LIBRARY_PATH=/home/debian/pythia8316/lib:/home/debian/lib:$LD_LIBRARY_PATH
  g++ -o generate_ggH_standalone generate_ggH_standalone.cc -I/home/debian/pythia8316/include -I/home/debian/include -L/home/debian/pythia8316/lib -L/home/debian/lib -lpythia8 -lHepMC -ldl -std=c++11 -O2 -Wno-deprecated-declarations -Wl,-rpath,/home/debian/pythia8316/lib:/home/debian/lib
  ```
```




