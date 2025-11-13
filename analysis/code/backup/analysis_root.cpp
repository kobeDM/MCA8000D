void analysis_root() {
    // ====== 入出力設定 ======
    const char* input_path  = "/home/msgc/MCA8000D/analysis/output/20251104/per7/SN794_NaI_data.root";
    const char* output_dir  = "/home/msgc/MCA8000D/analysis/output/20251104/per7_investigation/";

    // ====== ROOTファイルを開く ======
    TFile *file = new TFile(input_path, "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: ファイルを開けません。" << std::endl;
        return;
    }

    // ====== TTree取得 ======
    TTree *tree = (TTree*)file->Get("tree");
    if (!tree) {
        std::cerr << "Error: TTree 'tree' が見つかりません。" << std::endl;
        file->Close();
        return;
    }

    // ====== チャンネル定義 ======
    const int nch = 2;
    const int chIndex[nch] = {0, 50};

    Double_t chValue[nch] = {0};
    Double_t sum;
    for (int i = 0; i < nch; i++) {
        TString branchName = Form("ch%d", chIndex[i]);
        if (tree->GetBranch(branchName)) {
            tree->SetBranchAddress(branchName, &chValue[i]);
        } else {
            std::cerr << "Warning: " << branchName << " が存在しません。" << std::endl;
        }
    }
    tree->SetBranchAddress("sum", &sum);

    // ====== イベント数 ======
    Long64_t nentries = tree->GetEntries();
    std::cout << "イベント数: " << nentries << std::endl;

    // ====== データ格納 ======
    std::vector<std::vector<double>> xvals(nch + 1, std::vector<double>(nentries));
    std::vector<std::vector<double>> yvals(nch + 1, std::vector<double>(nentries));

    // ====== データ取得 ======
    for (Long64_t i = 0; i < nentries; i++) {
        tree->GetEntry(i);
        for (int j = 0; j < nch; j++) {
            xvals[j][i] = i;
            yvals[j][i] = chValue[j] / 60.0;   // ch* → /60
        }
        xvals[nch][i] = i;
        yvals[nch][i] = sum / 3600.0;          // sum → /3600
    }

    // ====== プロット ======
    TCanvas *c1 = new TCanvas("c1", "ch*/60 & sum/3600", 1000, 700);
    c1->SetGrid();

    // 色設定
    Color_t colors[nch + 1] = {kRed, kBlue};

    // 凡例
    TLegend *leg = new TLegend(0.85, 0.65, 0.98, 0.9);
    TGraph *graphs[nch + 1];

    // ch* 描画
    for (int j = 0; j < nch; j++) {
        graphs[j] = new TGraph(nentries, &xvals[j][0], &yvals[j][0]);
        graphs[j]->SetLineColor(colors[j]);
        graphs[j]->SetMarkerColor(colors[j]);
        graphs[j]->SetMarkerStyle(20);
        graphs[j]->SetMarkerSize(0.4);
        TString label = Form("ch%d (/60)", chIndex[j]);
        leg->AddEntry(graphs[j], label, "p");

        if (j == 0)
            graphs[j]->Draw("AP");
        else
            graphs[j]->Draw("P same");
    }

    // 軸タイトルなど
    graphs[0]->GetXaxis()->SetTitle("Event Number");
    graphs[0]->GetYaxis()->SetTitle("Scaled Value");
    graphs[0]->SetTitle("ch*/60 & sum/3600 vs Event");

    leg->Draw();

    // ====== 保存 ======
    TString outpath_png = TString::Format("%sch_scaled_plot.png", output_dir);
    c1->SaveAs(outpath_png);

    std::cout << "✅ 出力完了: " << outpath_png << std::endl;

    file->Close();
}
