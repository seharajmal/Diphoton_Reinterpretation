#include "SampleAnalyzer/User/Analyzer/cms_exo_22_024.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
using namespace MA5;
using namespace std;

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
    // Diagnostic only: raw mgg
    Manager()->AddHisto("mgg_preselection", 40, 0, 2000);

    cout << "END Initialization" << endl;
    return true;
}

void cms_exo_22_024::Finalize(const SampleFormat& summary, const std::vector<SampleFormat>& files)
{
    std::cout << "BEGIN Finalization" << std::endl;
    std::cout << "END Finalization" << std::endl;
}

// -----------------------------------------------------------------------
// DeltaR
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
// High-pT photon ID efficiency proxy for R9 + sigmaIetaIeta
// -----------------------------------------------------------------------
double PhotonIDEfficiencyFactor(bool isEB)
{
    return isEB ? 0.90 : 0.82;
}

// -----------------------------------------------------------------------
// Photon ID
// -----------------------------------------------------------------------
bool ApplyPhotonID(const RecPhotonFormat& photon, bool isEB, const EventFormat& event)
{
    // 1. H/E
    if (photon.HEoverEE() >= 0.05)
        return false;

    // 2. Electron veto
    if (event.rec() != nullptr)
    {
        const double dRElectronVeto = 0.1;
        for (const auto& el : event.rec()->electrons())
        {
            double dR = CalculateDeltaR(photon.eta(), photon.phi(), el.eta(), el.phi());
            if (dR < dRElectronVeto)
                return false;
        }
    }

    // 3. Charged-hadron isolation < 5.0 GeV
    double chiso = PHYSICS->Isol->eflow->sumIsolation(&photon, event.rec(), 0.3, 0.5,
                                                       IsolationEFlow::TRACK_COMPONENT);
    if (chiso >= 5.0)
        return false;

    // 4. Photon isolation: 2.75 GeV (EB) or 2.00 GeV (EE)
    double phiso = PHYSICS->Isol->eflow->sumIsolation(&photon, event.rec(), 0.3, 0.5,
                                                       IsolationEFlow::PHOTON_COMPONENT);
    phiso -= photon.pt();
    if (phiso < 0.0) phiso = 0.0;

    if (isEB && phiso >= 2.75) return false;
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
    
    Manager()->SetCurrentEventWeight(myWeight);
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

    // Diagnostic only: raw mgg
    if (allPhotons.size() >= 2)
    {
        std::vector<const RecPhotonFormat*> sortedAll = allPhotons;
        std::sort(sortedAll.begin(), sortedAll.end(),
                  [](const RecPhotonFormat* a, const RecPhotonFormat* b){
                      return a->pt() > b->pt();
                  });
        MALorentzVector rawSystem = sortedAll[0]->momentum() + sortedAll[1]->momentum();
        Manager()->FillHisto("mgg_preselection", rawSystem.M());
    }

    // ================================================================
    // CUT 1: At least 2 photons
    // ================================================================
    if (!Manager()->ApplyCut(allPhotons.size() >= 2, "At least 2 photons")) return true;

    // ================================================================
    // CUT 2: pT > 125 GeV, |eta| < 2.5
    // ================================================================

    struct PhotonCand {
        const RecPhotonFormat* ph;
        MALorentzVector mom;
        double pt;
        bool isEB;
    };

    std::vector<PhotonCand> corrCandidates;
    for (const auto& ph : allPhotons)
    {
        double abseta = std::fabs(ph->eta());
        if (abseta > 2.5) continue;
        if (abseta > 1.4442 && abseta < 1.566) continue;

        MALorentzVector mom = ph->momentum();
        double pt = mom.Pt();

        if (pt < 125.0) continue;

        PhotonCand pc;
        pc.ph   = ph;
        pc.mom  = mom;
        pc.pt   = pt;
        pc.isEB = (abseta < 1.4442);
        corrCandidates.push_back(pc);
    }

    if (!Manager()->ApplyCut(corrCandidates.size() >= 2, "Photon pT > 125 GeV & |eta| < 2.5")) return true;

    // Pick the two leading-pT candidates
    std::sort(corrCandidates.begin(), corrCandidates.end(),
              [](const PhotonCand& a, const PhotonCand& b){
                  return a.pt > b.pt;
              });

    const PhotonCand& lead    = corrCandidates[0];
    const PhotonCand& sublead = corrCandidates[1];

    // ================================================================
    // CUT 3: Photon ID
    // ================================================================
    bool leadPassesID    = ApplyPhotonID(*lead.ph,    lead.isEB,    event);
    bool subleadPassesID = ApplyPhotonID(*sublead.ph, sublead.isEB, event);
    bool passesID = leadPassesID && subleadPassesID;

    double idWeight = Manager()->GetCurrentEventWeight() * PhotonIDEfficiencyFactor(lead.isEB)
                                                     * PhotonIDEfficiencyFactor(sublead.isEB);
    Manager()->SetCurrentEventWeight(idWeight);

    if (!Manager()->ApplyCut(passesID, "Photon ID selection")) return true;

    // Fill post-ID histograms
    Manager()->FillHisto("photon_pt_afterID",  lead.pt);
    Manager()->FillHisto("photon_pt_afterID",  sublead.pt);
    Manager()->FillHisto("photon_eta_afterID", lead.ph->eta());
    Manager()->FillHisto("photon_eta_afterID", sublead.ph->eta());

    // Invariant mass
    MALorentzVector system = lead.mom + sublead.mom;
    double invariantMass   = system.M();

    // ================================================================
    // CUT 4: EBEB || EBEE
    // ================================================================
    bool passesEBEB = (lead.isEB && sublead.isEB);
    bool passesEBEE = ((lead.isEB && !sublead.isEB) || (!lead.isEB && sublead.isEB));

    if (!Manager()->ApplyCut(passesEBEB || passesEBEE, "EBEB||EBEE")) return true;

    // ================================================================
    // CUT 5: mgg > 500 GeV
    // ================================================================
    if (!Manager()->ApplyCut(invariantMass > 500.0, "Mgg > 500GeV")) return true;

    // ================================================================
    // CUT 6: DeltaR > 0.45
    // ================================================================
    double deltaR = CalculateDeltaR(lead.ph->eta(), lead.ph->phi(),
                                    sublead.ph->eta(), sublead.ph->phi());
    if (!Manager()->ApplyCut(deltaR > 0.45, "DeltaR > 0.45")) return true;

    // ================================================================
    // Mass bin assignment
    // ================================================================
    int mass_bin = static_cast<int>((invariantMass - 500.0) / 100.0) + 1;
    if (mass_bin < 1)  mass_bin = 1;
    if (mass_bin > 31) mass_bin = 31;

    // ================================================================
    // Fill region histograms
    // ================================================================
    if (passesEBEB)
    {
        std::string cut_name = "Mass bin " + std::to_string(mass_bin) + " EBEB";
        Manager()->ApplyCut(true, cut_name);
        Manager()->FillHisto("mgg_EBEB", invariantMass);
    }

    if (passesEBEE)
    {
        std::string cut_name = "Mass bin " + std::to_string(mass_bin) + " EBEE";
        Manager()->ApplyCut(true, cut_name);
        Manager()->FillHisto("mgg_EBEE", invariantMass);
    }

    return true;
}
