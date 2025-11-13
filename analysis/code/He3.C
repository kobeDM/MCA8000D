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

int He3()
{
    const int MCACh = 8191;
    const int header = 12;

    std::string base_in  = "/home/msgc/MCA8000D/data/";
    std::string base_out = "/home/msgc/MCA8000D/analysis/output/";

    std::string analysis_dir;
    std::cout << "解析ディレクトリを入力（例：20251104_calibration/per0_137Cs）: ";
    std::cin >> analysis_dir;

    // 入出力パス生成
    std::string in_dir  = base_in  + analysis_dir + "/";
    std::string out_dir = base_out + analysis_dir + "/";
    fs::create_directories(out_dir);

    // ディレクトリ内の .mca ファイル一覧を取得
    std::vector<std::string> filelist;
    for (const auto &entry : fs::directory_iterator(in_dir)) {
        std::string fname = entry.path().filename().string();
		if (fname.rfind("SN718_", 0) == 0 && entry.path().extension() == ".mca") {
            filelist.push_back(entry.path().string());
        }
    }

    if (filelist.empty()) {
        std::cerr << "Error: " << in_dir << " に .mca ファイルが見つかりません。" << std::endl;
        return 1;
    }

    TH1D *hist = new TH1D("hist", (analysis_dir + "   3He;ch;Counts / sec").c_str(), MCACh, 0, MCACh);
    double sum[MCACh] = {0};

    for (const auto &filepath : filelist) {
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) {
            std::cerr << "Error opening " << filepath << std::endl;
            continue;
        }

        std::string line;
        for (int i = 0; i < header; ++i) getline(ifs, line); // ヘッダー読み飛ばし

        double val;
        for (int ch = 0; ch < MCACh && ifs >> val; ++ch) {
            sum[ch] += val;
        }
    }

    int NFILES = filelist.size();
    for (int ch = 0; ch < MCACh; ++ch) {
        double plot = sum[ch] / (NFILES * 60);
        double energy = ch;
        hist->SetBinContent(hist->FindBin(energy), plot);
    }

    TCanvas *c1 = new TCanvas("c1", "MCA Spectrum", 800, 600);
    hist->Draw();

    // 保存
    std::string out_root = out_dir + "3He.root";
    std::string out_png  = out_dir + "3He.png";
    c1->SaveAs(out_png.c_str());

    TFile *outfile = new TFile(out_root.c_str(), "RECREATE");
    hist->Write();
    outfile->Close();

    std::cout << "解析完了: 出力 -> " << out_png << std::endl;
    return 0;
}
