#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TF1.h>

namespace fs = std::filesystem;

// 定数
const int START = 13;
const int END = 8204;
const int N_LINES = END - START + 1;
const int N_FILES = 60;

// ==========================
// データ読み込み
// ==========================
bool read_data(const std::string& prefix, const std::string& in_dir,
               std::vector<std::vector<double>>& data) {
    for (int i = 0; i < N_FILES; ++i) {
        std::ifstream f(in_dir + prefix + std::to_string(i) + ".mca");
        if (!f) {
            std::cerr << "Missing: " << prefix << i << "\n";
            continue;
        }

        std::string line;
        int n = 0, cnt = 0;
        while (std::getline(f, line)) {
            if (++n < START) continue;
            if (n > END) break;
            if (line.find_first_not_of(" \t\r") == std::string::npos) {
                std::cerr << "エラー: 空行を検出しました → "
                          << prefix << i << ".mca, 行番号: " << n << std::endl;
                return false;
            }
            data[i][cnt++] = std::stod(line);
        }
    }
    return true;
}

// ==========================
// 合計計算
// ==========================
void sum_data(std::vector<std::vector<double>>& data) {
    for (int j = 0; j < N_LINES; ++j)
        for (int i = 0; i < N_FILES; ++i)
            data[N_FILES][j] += data[i][j];
}

// ==========================
// ROOTファイル出力
// ==========================
void save_root(const std::string& prefix, const std::string& out_dir,
               const std::vector<std::vector<double>>& data) {
    TFile f((out_dir + prefix + "NaI_data.root").c_str(), "RECREATE");
    TTree t("tree", (prefix + " summed data").c_str());

    double ch[60], sum;
    for (int i = 0; i < 60; ++i)
        t.Branch(("ch" + std::to_string(i)).c_str(), &ch[i],
                 ("ch" + std::to_string(i) + "/D").c_str());
    t.Branch("sum", &sum, "sum/D");

    for (int j = 0; j < N_LINES; ++j) {
        for (int i = 0; i < 60; ++i) ch[i] = data[i][j];
        sum = data[N_FILES][j];
        t.Fill();
    }

    t.Write();
    f.Close();
}

// ==========================
// フィット + プロット
// ==========================
void fit_and_plot(const std::string& prefix, const std::string& out_dir,
                  const std::vector<double>& ydata) {
    // x軸生成
    std::vector<double> x(N_LINES);
    for (int i = 0; i < N_LINES; ++i) x[i] = i;

    TCanvas c((prefix + "c").c_str(), "", 800, 600);
    // c.SetLogy();
    c.SetGrid();

    TGraph g(N_LINES, &x[0], &ydata[0]);
    g.SetTitle((prefix + " summed spectrum;Channel;Counts").c_str());
    g.Draw("AP");

    c.SaveAs((out_dir + prefix + "NaI.png").c_str());

    // --- ガウシアン＋線形フィット ---
    TF1 *fitFunc_low = new TF1("fitFunc_low",
        "[0]*exp(-0.5*((x-[1])/[2])**2) + [3]*x + [4]", 440, 800);

    // --- 各パラメータに意味のある名前を付ける ---
    fitFunc_low->SetParName(0, "Amplitude");
    fitFunc_low->SetParName(1, "Mean");
    fitFunc_low->SetParName(2, "Sigma");
    fitFunc_low->SetParName(3, "Slope");
    fitFunc_low->SetParName(4, "Offset");

    // --- 初期値設定 ---
    double p0_amplitude_low = 18000;
    double p1_mean_low      = 600;
    double p2_sigma_low     = 50;
    double p3_slope_low     = -500;
    double p4_offset_low    = 3500;

    fitFunc_low->SetParameters(p0_amplitude_low, p1_mean_low, p2_sigma_low, p3_slope_low, p4_offset_low);

    TF1 *fitFunc_high = new TF1("fitFunc_high",
        "[0]*exp(-0.5*((x-[1])/[2])**2) + [3]*x + [4]", 1300, 1650);

    fitFunc_high->SetParName(0, "Amplitude");
    fitFunc_high->SetParName(1, "Mean");
    fitFunc_high->SetParName(2, "Sigma");
    fitFunc_high->SetParName(3, "Slope");
    fitFunc_high->SetParName(4, "Offset");

    // --- 初期値設定 ---
    double p0_amplitude_high = 2200;
    double p1_mean_high      = 1550;
    double p2_sigma_high     = 10;
    double p3_slope_high     = -5;
    double p4_offset_high    = 20;

    fitFunc_high->SetParameters(p0_amplitude_high, p1_mean_high, p2_sigma_high, p3_slope_high, p4_offset_high);

    // --- フィット実行 ---
    g.Fit(fitFunc_low, "R"); // 範囲 [1000,2000] でフィット
    g.Fit(fitFunc_high, "R"); // 範囲 [1000,2000] でフィット
    
    TCanvas c_fit((prefix + "_fit").c_str(), "", 800, 600);
    g.Draw("AP");

    // fitFunc_low->Draw("same");
    // fitFunc_high->Draw("same");

    g.GetXaxis()->SetRangeUser(0, 1500);

    c_fit.SaveAs((out_dir + prefix + "NaI_fit.png").c_str());

}

// ==========================
// メイン解析関数
// ==========================
void analyze(const std::string& prefix,
             const std::string& in_dir, const std::string& out_dir) {
    std::vector<std::vector<double>> data(N_FILES + 1, std::vector<double>(N_LINES, 0));

    if (!read_data(prefix, in_dir, data)) return;
    sum_data(data);
    save_root(prefix, out_dir, data);
    fit_and_plot(prefix, out_dir, data[N_FILES]);

    std::cout << prefix << " done.\n";
}

// ==========================
// main
// ==========================
int main() {
    const std::string base_in  = "/home/msgc/MCA8000D/data/";
    const std::string base_out = "/home/msgc/MCA8000D/analysis/output/";

    std::string sub;
    std::cout << "解析ディレクトリを入力（例：20251103/per1）: ";
    std::cin >> sub;

    std::string in_dir = base_in + sub + "/";
    std::string out_dir = base_out + sub + "/";
    fs::create_directories(out_dir);

    analyze("SN794_", in_dir, out_dir);

    return 0;
}
