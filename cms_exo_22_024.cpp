#include "SampleAnalyzer/User/Analyzer/cms_exo_22_024.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <random>

using namespace MA5;
using namespace std;

// Global random generator for smearing
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> dis(0, 1);

bool cms_exo_22_024::Initialize(const Configuration& cfg, const std::map<std::string,std::string>& parameters)
{
    INFO << "Starting analysis cms_exo_22_024" << endmsg;
    std::cout << "BEGIN Initialization" << std::endl;

    for (int i = 1; i <= 31; i++)
    {
        Manager()->AddRegionSelection("EBEB_bin" + std::to_string(i));
        Manager()->AddRegionSelection("EBEE_bin" + std::to_string(i));
    }

    std::string EBEB_regions[] = {
        "EBEB_bin1",  "EBEB_bin2",  "EBEB_bin3",  "EBEB_bin4",  "EBEB_bin5",
        "EBEB_bin6",  "EBEB_bin7",  "EBEB_bin8",  "EBEB_bin9",  "EBEB_bin10",
        "EBEB_bin11", "EBEB_bin12", "EBEB_bin13", "EBEB_bin14", "EBEB_bin15",
        "EBEB_bin16", "EBEB_bin17", "EBEB_bin18", "EBEB_bin19", "EBEB_bin20",
        "EBEB_bin21", "EBEB_bin22", "EBEB_bin23", "EBEB_bin24", "EBEB_bin25",
        "EBEB_bin26", "EBEB_bin27", "EBEB_bin28", "EBEB_bin29", "EBEB_bin30",
        "EBEB_bin31"
    };

    std::string EBEE_regions[] = {
        "EBEE_bin1",  "EBEE_bin2",  "EBEE_bin3",  "EBEE_bin4",  "EBEE_bin5",
        "EBEE_bin6",  "EBEE_bin7",  "EBEE_bin8",  "EBEE_bin9",  "EBEE_bin10",
        "EBEE_bin11", "EBEE_bin12", "EBEE_bin13", "EBEE_bin14", "EBEE_bin15",
        "EBEE_bin16", "EBEE_bin17", "EBEE_bin18", "EBEE_bin19", "EBEE_bin20",
        "EBEE_bin21", "EBEE_bin22", "EBEE_bin23", "EBEE_bin24", "EBEE_bin25",
        "EBEE_bin26", "EBEE_bin27", "EBEE_bin28", "EBEE_bin29", "EBEE_bin30",
        "EBEE_bin31"
    };

    // General cuts
    Manager()->AddCut("At least 2 photons");
    Manager()->AddCut("Photon pT > 125 GeV & |eta| < 2.5");
    Manager()->AddCut("Photon ID selection");
    Manager()->AddCut("EBEB||EBEE");
    Manager()->AddCut("Mgg > 500GeV");
    Manager()->AddCut("DeltaR > 0.45");

    // Region-specific mass bin cuts
    for (int i = 1; i <= 31; i++)
    {
        std::string region_ebeb = "EBEB_bin" + std::to_string(i);
        string ebeb_arr[] = {region_ebeb};
        Manager()->AddCut("Mass bin " + std::to_string(i) + " EBEB", ebeb_arr);

        std::string region_ebee = "EBEE_bin" + std::to_string(i);
        string ebee_arr[] = {region_ebee};
        Manager()->AddCut("Mass bin " + std::to_string(i) + " EBEE", ebee_arr);
    }

    Manager()->AddHisto("mgg_EBEB", 31, 500, 3600, EBEB_regions);
    Manager()->AddHisto("mgg_EBEE", 31, 500, 3600, EBEE_regions);
    Manager()->AddHisto("photon_pt_all",      100, 0, 1000);
    Manager()->AddHisto("photon_eta_all",      50, -2.5, 2.5);
    Manager()->AddHisto("photon_pt_afterID",  100, 0, 1000);
    Manager()->AddHisto("photon_eta_afterID",  50, -2.5, 2.5);

    cout << "END Initialization" << endl;
    return true;
}

void cms_exo_22_024::Finalize(const SampleFormat& summary, const std::vector<SampleFormat>& files)
{
    std::cout << "BEGIN Finalization" << std::endl;
    std::cout << "END Finalization" << std::endl;
}

// -----------------------------------------------------------------------
// DeltaR helper
// -----------------------------------------------------------------------
double CalculateDeltaR(double eta1, double phi1, double eta2, double phi2)
{
    double deta = eta1 - eta2;
    double dphi = phi1 - phi2;
    while (dphi >  M_PI) dphi -= 2 * M_PI;
    while (dphi < -M_PI) dphi += 2 * M_PI;
    return std::sqrt(deta * deta + dphi * dphi);
}

// -----------------------------------------------------------------------
// Photon energy correction: 0.5% scale + Gaussian smearing
// Returns a scale factor to multiply the 4-vector by.
// -----------------------------------------------------------------------
double PhotonCorrectionFactor(const RecPhotonFormat* photon)
{
    double abseta = std::fabs(photon->eta());

    // Base scale correction from Z->ee calibration
    double scale = 1.005;

    // Additional Gaussian smearing (sigma depends on eta region)
    double smear_sigma = 0.0;
    if (abseta < 1.44) {
        smear_sigma = 0.008 + dis(gen) * 0.007;   // 0.8-1.5% EB
    } else if (abseta > 1.57 && abseta < 2.5) {
        smear_sigma = 0.02  + dis(gen) * 0.005;   // 2.0-2.5% EE
    } else {
        return scale;  // gap region: scale only, no smearing
    }

    std::normal_distribution<> gauss(1.0, smear_sigma);
    return scale * gauss(gen);
}

// -----------------------------------------------------------------------
// Per-photon reconstruction efficiency returned as a weight factor.
// Barrel: 90%,  Endcap: 82%,  Gap/outside: 0 (photon rejected upstream).
// -----------------------------------------------------------------------
double RecoEfficiencyWeight(const RecPhotonFormat* photon)
{
    double abseta = std::fabs(photon->eta());
    if      (abseta < 1.44)                      return 0.90;
    else if (abseta > 1.57 && abseta < 2.5)      return 0.82;
    else                                          return 0.0;   // gap
}

// -----------------------------------------------------------------------
// Photon ID (paper Section 3)
//   - H/E < 5%
//   - Charged-hadron isolation < 5.0 GeV
//   - Photon isolation < 2.75 GeV (EB) or < 2.00 GeV (EE)
// -----------------------------------------------------------------------
bool ApplyPhotonID(const RecPhotonFormat& photon, bool isEB, const EventFormat& event)
{
    // 1. H/E
    if (photon.HEoverEE() >= 0.05)
        return false;

    // 2. Charged-hadron isolation < 5.0 GeV
    double chiso = PHYSICS->Isol->tracker->sumIsolation(&photon, event.rec(), 0.3, 0.);
    if (chiso >= 5.0)
        return false;

    // 3. Photon isolation: 2.75 GeV (EB) or 2.00 GeV (EE)  [paper Sec. 3]
    double phiso = PHYSICS->Isol->calorimeter->sumIsolation(&photon, event.rec(), 0.3, 0.);
    if (isEB  && phiso >= 2.75) return false;
    if (!isEB && phiso >= 2.00) return false;

    return true;
}

// -----------------------------------------------------------------------
bool cms_exo_22_024::Execute(SampleFormat& sample, const EventFormat& event)
{
    // ---- Event weight ----
    double myWeight = 1.0;
    if (!Configuration().IsNoEventWeight())
    {
        if (event.mc() == nullptr || event.mc()->weight() == 0.0) return false;
        myWeight = event.mc()->weight();
    }
    Manager()->InitializeForNewEvent(myWeight);

    // ---- Collect all photons ----
    std::vector<const RecPhotonFormat*> allPhotons;
    if (event.rec() != nullptr)
        for (const auto& ph : event.rec()->photons())
            allPhotons.push_back(&ph);

    for (const auto& ph : allPhotons) {
        Manager()->FillHisto("photon_pt_all",  ph->pt());
        Manager()->FillHisto("photon_eta_all", ph->eta());
    }

    // ================================================================
    // CUT 1: At least 2 photons
    // ================================================================
    if (event.rec() == nullptr || event.rec()->photons().size() < 2)
        return true;
    if (!Manager()->ApplyCut(true, "At least 2 photons")) return true;

    // ================================================================
    // CUT 2: pT > 125 GeV, |eta| < 2.5  (excluding gap 1.44-1.57)
    // Apply energy correction BEFORE the pT cut so the threshold is
    // applied consistently on corrected momenta (paper: offline pT > 125).
    // ================================================================
    //
    // Build (photon*, corrected_4vector) pairs for all candidates,
    // then filter and sort by corrected pT.
    //
    struct PhotonWithCorr {
        const RecPhotonFormat* ph;
        MALorentzVector mom;   // corrected 4-vector
        double corrPt;
        bool isEB;
    };

    std::vector<PhotonWithCorr> corrCandidates;
    for (const auto& ph : allPhotons)
    {
        double abseta = std::fabs(ph->eta());
        if (abseta > 2.5)           continue;  // outside acceptance
        if (abseta > 1.44 && abseta < 1.57) continue;  // gap region

        double factor = PhotonCorrectionFactor(ph);
        MALorentzVector mom = ph->momentum();
        mom *= factor;
        double corrPt = mom.Pt();

        if (corrPt < 125.0) continue;

        PhotonWithCorr pwc;
        pwc.ph     = ph;
        pwc.mom    = mom;
        pwc.corrPt = corrPt;
        pwc.isEB   = (abseta < 1.44);
        corrCandidates.push_back(pwc);
    }

    if (corrCandidates.size() < 2) return true;
    if (!Manager()->ApplyCut(true, "Photon pT > 125 GeV & |eta| < 2.5")) return true;

    // ================================================================
    // Reconstruction efficiency: applied as an event-weight factor
    // (product over the two leading photons, chosen after ID below).
    // We do NOT randomly discard photons — that inflates weight variance.
    // ================================================================

    // ================================================================
    // CUT 3: Photon ID
    // ================================================================
    std::vector<PhotonWithCorr> idCandidates;
    for (const auto& pwc : corrCandidates)
    {
        if (ApplyPhotonID(*pwc.ph, pwc.isEB, event))
            idCandidates.push_back(pwc);
    }

    if (idCandidates.size() < 2) return true;
    if (!Manager()->ApplyCut(true, "Photon ID selection")) return true;

    // Sort by corrected pT (descending) and pick the two leading
    std::sort(idCandidates.begin(), idCandidates.end(),
              [](const PhotonWithCorr& a, const PhotonWithCorr& b){
                  return a.corrPt > b.corrPt;
              });

    const PhotonWithCorr& lead    = idCandidates[0];
    const PhotonWithCorr& sublead = idCandidates[1];

    // Apply per-photon reco efficiency as a weight
    double recoEff = RecoEfficiencyWeight(lead.ph) * RecoEfficiencyWeight(sublead.ph);
    Manager()->SetCurrentEventWeight(myWeight * recoEff);

    // Fill post-ID histograms
    Manager()->FillHisto("photon_pt_afterID",  lead.corrPt);
    Manager()->FillHisto("photon_pt_afterID",  sublead.corrPt);
    Manager()->FillHisto("photon_eta_afterID", lead.ph->eta());
    Manager()->FillHisto("photon_eta_afterID", sublead.ph->eta());

    // Invariant mass from corrected 4-vectors
    MALorentzVector system = lead.mom + sublead.mom;
    double invariantMass   = system.M();

    // ================================================================
    // CUT 4: EBEB || EBEE  (gap photons already excluded above)
    // ================================================================
    bool leadEB    = lead.isEB;
    bool subleadEB = sublead.isEB;
    bool leadEE    = !lead.isEB;
    bool subleadEE = !sublead.isEB;

    bool passesEBEB = (leadEB   && subleadEB);
    bool passesEBEE = ((leadEB  && subleadEE) || (leadEE && subleadEB));

    if (!passesEBEB && !passesEBEE) return true;
    if (!Manager()->ApplyCut(true, "EBEB||EBEE")) return true;

    // ================================================================
    // CUT 5: mgg > 500 GeV
    // ================================================================
    if (invariantMass <= 500.0) return true;
    if (!Manager()->ApplyCut(true, "Mgg > 500GeV")) return true;

    // ================================================================
    // CUT 6: DeltaR > 0.45
    // ================================================================
    double deltaR = CalculateDeltaR(lead.ph->eta(), lead.ph->phi(),
                                    sublead.ph->eta(), sublead.ph->phi());
    if (deltaR <= 0.45) return true;
    if (!Manager()->ApplyCut(true, "DeltaR > 0.45")) return true;

    // ================================================================
    // Mass bin assignment (100 GeV bins, 500-3600 GeV, overflow -> bin 31)
    // ================================================================
    int mass_bin = static_cast<int>((invariantMass - 500.0) / 100.0) + 1;
    if (mass_bin < 1)  mass_bin = 1;
    if (mass_bin > 31) mass_bin = 31;

    // ================================================================
    // Fill region histograms — EBEB and EBEE are independent paths.
    // A failed EBEB bin cut must NOT abort the EBEE evaluation.
    // ================================================================
    if (passesEBEB)
    {
        std::string cut_name = "Mass bin " + std::to_string(mass_bin) + " EBEB";
        Manager()->ApplyCut(true, cut_name);   // no early return here
        Manager()->FillHisto("mgg_EBEB", invariantMass);
    }

    if (passesEBEE)
    {
        std::string cut_name = "Mass bin " + std::to_string(mass_bin) + " EBEE";
        Manager()->ApplyCut(true, cut_name);   // no early return here
        Manager()->FillHisto("mgg_EBEE", invariantMass);
    }

    return true;
}
