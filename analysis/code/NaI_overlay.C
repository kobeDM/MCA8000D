// ----------------------- //
// DrawMCA.C -- root macro
// Author: K. Mizukoshi
// Date  : Updated 2025-11-05
// Description: 
// Reads multiple MCA files from a user-specified directory
// ----------------------- //

#include <TFile.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

int NaI_overlay()
{
    const int MCACh = 8191;
    const int header = 12;

    std::string base_in  = "/home/msgc/MCA8000D/data/";
    std::string base_out = "/home/msgc/MCA8000D/analysis/output/";

    std::string analysis_dir;
    std::cout << "解析ディレクトリを入力（例：20251104_calibration/per0_137Cs）: ";
    std::cin >> analysis_dir;

    std::string in_dir  = base_in  + analysis_dir + "/";
    std::string out_dir = base_out + analysis_dir + "/";
    fs::create_directories(out_dir);

    // ディレクトリ内の .mca ファイル一覧を取得
    std::vector<std::string> filelist;
    for (const auto &entry : fs::directory_iterator(in_dir)) {
        std::string fname = entry.path().filename().string();
		if (fname.rfind("SN794_", 0) == 0 && entry.path().extension() == ".mca") {
            filelist.push_back(entry.path().string());
        }
    }

    if (filelist.empty()) {
        std::cerr << "Error: " << in_dir << " に .mca ファイルが見つかりません。" << std::endl;
        return 1;
    }

    int NFILES = filelist.size();
    std::cout << "読み込んだファイル数: " << NFILES << std::endl;

    std::vector<double> sum_all(MCACh, 0.0);
    std::vector<double> sum_begin(MCACh, 0.0);
    std::vector<double> sum_end(MCACh, 0.0);

    for (size_t i = 0; i < filelist.size(); ++i) {
        std::ifstream ifs(filelist[i]);
        if (!ifs.is_open()) {
            std::cerr << "Error opening " << filelist[i] << std::endl;
            continue;
        }

        std::string line;
        for (int i = 0; i < header; ++i) getline(ifs, line); // ヘッダー読み飛ばし
           
        double val;
        for (int ch = 0; ch < MCACh && ifs >> val; ++ch) {
            sum_all[ch] += val;
            if (i < 5) sum_begin[ch] += val;
            if (i >= 15 && i < 20) sum_end[ch] += val;
        }
    }

    TH1D *hist_all = new TH1D("hist_all", (analysis_dir + "   NaI;Energy [keV];Counts / keV / sec").c_str(), MCACh, 0, MCACh);
    TH1D *hist_begin = new TH1D("hist_begin", (analysis_dir + "   NaI;Energy [keV];Counts / keV / sec").c_str(), MCACh, 0, MCACh);
    TH1D *hist_end = new TH1D("hist_end", (analysis_dir + "   NaI;Energy [keV];Counts / keV / sec").c_str(), MCACh, 0, MCACh);

    for (int ch = 0; ch < MCACh; ++ch) {
        double energy = (ch - 47.7106) / 1.13414;
        double plot_all = (sum_all[ch] * 1.13414) / (NFILES * 60);
        double plot_begin = (sum_begin[ch] * 1.13414) / (10 * 60);
        double plot_end   = (sum_end[ch]   * 1.13414) / (10 * 60);
    
        hist_all->SetBinContent(hist_all->FindBin(energy), plot_all);
        hist_begin->SetBinContent(hist_begin->FindBin(energy), plot_begin);
        hist_end->SetBinContent(hist_end->FindBin(energy), plot_end);

    }

    TCanvas *c_all = new TCanvas("c_all", "MCA Spectrum", 800, 600);
    hist_all->SetLineColor(kRed);
    hist_all->Draw();
    std::string out_all_png = out_dir + "NaI.png";
    c_all->SaveAs(out_all_png.c_str());

    
    TCanvas *c_overlay = new TCanvas("c_overlay", "MCA Spectrum Overlay", 800, 600);
    hist_begin->SetLineColor(kRed);
    hist_end->SetLineColor(kBlue);
    hist_begin->Draw();
    hist_end->Draw("SAME");

    auto legend = new TLegend(0.7, 0.75, 0.9, 0.9);
    legend->AddEntry(hist_begin, "First 10 files (0-9)", "l");
    legend->AddEntry(hist_end,   "Files 40-59", "l");
    legend->Draw();

    std::string out_overlay_png  = out_dir + "NaI_overlay.png";
    c_overlay->SaveAs(out_overlay_png.c_str());

    std::string out_root = out_dir + "NaI.root";
    TFile *outfile = new TFile(out_root.c_str(), "RECREATE");
    hist_all->Write();
    outfile->Close();

    std::cout << "解析完了: 出力 -> " << out_all_png << std::endl;
    return 0;
}
