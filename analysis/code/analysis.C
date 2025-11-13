{
    // --- ファイルを開く ---
    TFile *f1 = new TFile("../output/20251105/per5/NaI.root");
    TFile *f2 = new TFile("../output/20251105/per8/NaI.root");

    // --- ヒストグラムを取得 ---
    TH1D *h1 = (TH1D*)f1->Get("hist");
    TH1D *h2 = (TH1D*)f2->Get("hist");

    // --- スタイル設定（色や線） ---
    h1->SetLineColor(kRed);
    h1->SetLineWidth(2);
    h2->SetLineColor(kBlue);
    h2->SetLineWidth(2);

    // --- 描画 ---
    TCanvas *c1 = new TCanvas("c1", "Comparison", 800, 600);
    h1->Draw("HIST");           // 1枚目を描く
    h2->Draw("HIST SAME");      // 2枚目を重ね描き

    // --- 凡例を追加 ---
    auto legend = new TLegend(0.6, 0.7, 0.88, 0.88);
    legend->AddEntry(h1, "per5", "l");
    legend->AddEntry(h2, "per8", "l");
    legend->Draw();

    // --- 保存 ---
    c1->SaveAs("analysis_NaI.png");
}
