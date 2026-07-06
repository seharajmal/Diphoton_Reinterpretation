#include "Pythia8/Pythia.h"

// Use HepMC2 to match your desired output format
#include "Pythia8Plugins/HepMC2.h"

using namespace Pythia8;

int main() {
    // Interface for conversion from Pythia8::Event to HepMC2 event
    Pythia8ToHepMC toHepMC("ggH_5000_0p014.hepmc");

    // ============================================
    // EXACT SETTINGS FROM YOUR CMSSW FRAGMENT
    // ============================================
    Pythia pythia;
    
    // 1. Beam energy (13 TeV)
    pythia.readString("Beams:eCM = 13000.0");
    
    // 2. Higgs production
    pythia.readString("HiggsSM:gg2H = on");
    
    // 3. Higgs properties
    pythia.readString("25:m0 = 5000.0");
    pythia.readString("25:mWidth = 0.7");
    
    // For H->γγ
    pythia.readString("25:onMode = off");
    pythia.readString("25:onIfMatch = 22 22");
    
    // Force width settings
    pythia.readString("25:doForceWidth = on");
    pythia.readString("Higgs:clipWings = on");
    pythia.readString("Higgs:clipWings = 10");
    
    // ============================================
    // COMMON SETTINGS FROM CMSSW FRAGMENT
    // ============================================
    pythia.readString("Tune:preferLHAPDF = 2");
    pythia.readString("Main:timesAllowErrors = 10000");
    pythia.readString("Check:epTolErr = 0.01");
    pythia.readString("Beams:setProductionScalesFromLHEF = off");
    pythia.readString("SLHA:minMassSM = 1000.");
    pythia.readString("ParticleDecays:limitTau0 = on");
    pythia.readString("ParticleDecays:tau0Max = 10");
    pythia.readString("HadronLevel:QED = on");
    
    // ============================================
    // EXACT CP2 TUNE FROM CMSSW FRAGMENT
    // ============================================
    pythia.readString("Tune:pp = 14");
    pythia.readString("Tune:ee = 7");
    
    // Multiparton interactions
    pythia.readString("MultipartonInteractions:ecmPow = 0.1391");
    pythia.readString("MultipartonInteractions:bProfile = 2");
    pythia.readString("MultipartonInteractions:pT0Ref = 2.306");
    pythia.readString("MultipartonInteractions:coreRadius = 0.3755");
    pythia.readString("MultipartonInteractions:coreFraction = 0.3269");
    
    // Colour reconnection
    pythia.readString("ColourReconnection:range = 2.323");
    
    // Sigma total settings
    pythia.readString("SigmaTotal:zeroAXB = off");
    pythia.readString("SigmaTotal:mode = 0");
    pythia.readString("SigmaTotal:sigmaEl = 21.89");
    pythia.readString("SigmaTotal:sigmaTot = 100.309");
    
    // Shower settings
    pythia.readString("SpaceShower:rapidityOrder = off");
    pythia.readString("SpaceShower:alphaSvalue = 0.13");
    pythia.readString("TimeShower:alphaSvalue = 0.13");
    
    // ============================================
    // PDF SETTING
    // ============================================
    pythia.readString("PDF:pSet = 17"); // NNPDF31_lo_as_0130
    
    
    // ============================================
    // Initialize Pythia
    // ============================================
    std::cout << "Initializing Pythia with gg->H->γγ process..." << std::endl;
    if (!pythia.init()) {
        std::cerr << "Error: Pythia initialization failed!" << std::endl;
        return 1;
    }
    
    // ============================================
    // Generate events
    // ============================================
    int nEvents = 100000;
    int nGenerated = 0;
    int nHiggs = 0;
    int nPhotons = 0;
    
    std::cout << "\nGenerating " << nEvents << " events..." << std::endl;
    
    for (int iEvent = 0; iEvent < nEvents; ++iEvent) {
        if (!pythia.next()) {
            if (iEvent < 10) {
                std::cout << "Warning: Event " << iEvent << " generation failed!" << std::endl;
            }
            continue;
        }
        
        nGenerated++;
        
        // Count Higgs and photons for statistics
        for (int i = 0; i < pythia.event.size(); ++i) {
            const Particle& part = pythia.event[i];
            if (part.id() == 25) nHiggs++;
            if (part.id() == 22 && part.isFinal()) nPhotons++;
        }
        
        // Write event to HepMC2 file using the plugin
        toHepMC.writeNextEvent(pythia);
        
        // Progress report
        if ((iEvent+1) % 100 == 0) {
            std::cout << "Generated " << (iEvent+1) << " events"
                      << " (Higgs: " << nHiggs << ", Photons: " << nPhotons << ")" << std::endl;
        }
    }
    
    // ============================================
    // Statistics
    // ============================================
    pythia.stat();
    
    double generatedXS = pythia.info.sigmaGen() * 1e9;  // Convert to pb
    double expectedXS = 3.28e-19;  // Expected value from your analysis
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "RESULTS" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "Events generated: " << nGenerated << "/" << nEvents << std::endl;
    std::cout << "Higgs produced: " << nHiggs << " (" << (double)nHiggs/nGenerated << " per event)" << std::endl;
    std::cout << "Photons produced: " << nPhotons << " (" << (double)nPhotons/nGenerated << " per event)" << std::endl;
    std::cout << "Expected photons per event: ~2 (from H->γγ decay)" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    std::cout << "CROSS SECTION:" << std::endl;
    std::cout << "  Generated: " << generatedXS << " pb" << std::endl;
    std::cout << "  Expected: " << expectedXS << " pb (σ × BR for gg→H→γγ)" << std::endl;
    
    if (expectedXS > 0) {
        std::cout << "  Ratio: " << generatedXS / expectedXS << std::endl;
    }
    
    
    // Check if we got the expected format
    std::cout << "\nChecking HepMC file format..." << std::endl;
    std::ifstream checkFile("ggH_5000_0p014.hepmc");
    std::string firstLine;
    if (std::getline(checkFile, firstLine)) {
        if (firstLine.find("HepMC::Version") != std::string::npos) {
            std::cout << "✓ HepMC file has correct header" << std::endl;
        }
    }
    checkFile.close();
    
    return 0;
}
