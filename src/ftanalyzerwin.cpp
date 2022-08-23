#include "ftanalyzerwin.h"
#include "ui_ftanalyzerwin.h"

#include "analyzer.h"
#include "ROOTDraw.h"

#include <QFileDialog>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStringList>
#include <QRect>
#include <QScrollBar>
#include <QGridLayout>

#include <TDatime.h>

#include <iostream>
FTAnalyzerWin::FTAnalyzerWin(QWidget *parent) : QMainWindow(parent),
                                                ui(new Ui::FTAnalyzerWin)
{
    ui->setupUi(this);

    // File Tab
    ui->tabwFiles->setColumnCount(6);
    QStringList hlabels = tr("File Name, Fiber No., End No., Test Serial, Date, Time").split(",");
    ui->tabwFiles->setHorizontalHeaderLabels(hlabels);
    ui->lineFilePath->setFocusPolicy(Qt::NoFocus);

    auto grid = new QGridLayout(ui->wdgPreview);
    fdrawWin = new PlotWindow(2);
    grid->addWidget(fdrawWin, 0, 0, 7, 1);

    // Fit Channel Tab
    ui->tabwFitResult->setColumnCount(7);
    QStringList hlabels2;
    hlabels2 << "Channel"
             << "Const"
             << "err"
             << "Mean"
             << "err"
             << "Sigma"
             << "err";
    ui->tabwFitResult->setHorizontalHeaderLabels(hlabels2);

    // Should be put in the end;
    show();
    fdrawWin->getROOTWidget()->Update();
}

FTAnalyzerWin::~FTAnalyzerWin()
{
    std::cout << "Closing: " << std::endl;
    delete ui;
}

#include <QString>
void FTAnalyzerWin::on_btnFTFolder_clicked()
{
    auto folder = QFileDialog::getExistingDirectory(this, "Open Folder", "F:/Projects/MuonTestControl/Data/FiberTest-0811/Data");
    ui->lineFilePath->setText(folder);
    if (folder == "")
        return;
    sFolder = folder;
    gFTFolderParser->Init(folder.toStdString());

    ui->tabwFiles->clearContents();
    ui->tabwFiles->setRowCount(gFTFolderParser->GetArray().size());

    QStringList vlabels;

    QTableWidgetItem *item = 0;
    for (int i = 0; i < gFTFolderParser->GetArray().size(); i++)
    {
        vlabels << QString::number(i);

        auto ft = gFTFolderParser->GetArray()[i];
        item = new QTableWidgetItem(QString::fromStdString(ft->GetName()));
        ui->tabwFiles->setItem(i, 0, item);
        item = new QTableWidgetItem(QString::number(ft->GetFiber()));
        ui->tabwFiles->setItem(i, 1, item);
        item = new QTableWidgetItem(QString::number(ft->GetEnd()));
        ui->tabwFiles->setItem(i, 2, item);
        item = new QTableWidgetItem(QString::number(ft->GetTestSerial()));
        ui->tabwFiles->setItem(i, 3, item);
        item = new QTableWidgetItem(QString::number(ft->GetDatime()->GetDate()));
        ui->tabwFiles->setItem(i, 4, item);
        item = new QTableWidgetItem(QString::number(ft->GetDatime()->GetTime()));
        ui->tabwFiles->setItem(i, 5, item);
    }

    ui->tabwFiles->setVerticalHeaderLabels(vlabels);
    ui->tabwFiles->setShowGrid(true);
    ui->tabwFiles->setGridStyle(Qt::DashDotDotLine);
    ui->tabwFiles->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tabwFiles->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tabwFiles->resizeColumnsToContents();
    ui->tabwFiles->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    ui->grpFileList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tabwFiles->show();
}

#include <string>
#include "datamanager.h"
void FTAnalyzerWin::on_tabwFiles_cellActivated(int row, int column)
{
    if (row < 0 || row >= gFTFolderParser->GetArray().size())
        return;
    fFileName = gFTFolderParser->GetArray()[row]->GetName();
    gReadManager->Init((sFolder.toStdString() + "/" + fFileName));
    gReadManager->Draw(ui->boxSelectCh->value());
    fTabActivated = 1;
    fdrawWin->getROOTWidget()->Update();

    gFTAnalyzer->FitFile(sFolder.toStdString() + "/" + fFileName);
    ui->tabwFitResult->setRowCount(2);
    auto par16 = gFTAnalyzer->GetPar16();
    auto parE16 = gFTAnalyzer->GetParE16();
    auto par18 = gFTAnalyzer->GetPar18();
    auto parE18 = gFTAnalyzer->GetParE18();

    ui->tabwFitResult->setItem(0, 0, new QTableWidgetItem(QString::number(16)));
    ui->tabwFitResult->setItem(1, 0, new QTableWidgetItem(QString::number(18)));
    for (int i = 0; i < 3; i++)
    {
        ui->tabwFitResult->setItem(0, 2 * i + 1, new QTableWidgetItem(QString::number(par16[i])));
        ui->tabwFitResult->setItem(0, 2 * i + 2, new QTableWidgetItem(QString::number(parE16[i])));
        ui->tabwFitResult->setItem(1, 2 * i + 1, new QTableWidgetItem(QString::number(par18[i])));
        ui->tabwFitResult->setItem(1, 2 * i + 2, new QTableWidgetItem(QString::number(parE18[i])));
    }
    QStringList vlabels;
    vlabels << QString::number(0) << QString::number(1);

    ui->tabwFitResult->setVerticalHeaderLabels(vlabels);
    ui->tabwFitResult->setShowGrid(true);
    ui->tabwFitResult->setGridStyle(Qt::DashDotDotLine);
    ui->tabwFitResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tabwFitResult->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tabwFitResult->resizeColumnsToContents();
    ui->tabwFitResult->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    ui->tabwFiles->show();
}

void FTAnalyzerWin::on_boxSelectCh_textChanged(const QString &arg1)
{
    if (!fTabActivated)
        return;
    gReadManager->Draw(ui->boxSelectCh->value());
    fdrawWin->getROOTWidget()->Update();
}

void FTAnalyzerWin::on_btnFit_clicked()
{
    int ch = ui->boxSelectCh->value();
    double par[3], parE[3];
    if (!gReadManager->FitChannel(ch, par, parE))
    {
        return;
    }
    QStringList labels;

    int size = ui->tabwFitResult->rowCount();
    ui->tabwFitResult->setRowCount(size + 1);
    ui->tabwFitResult->setItem(size, 0, new QTableWidgetItem(QString::number(ch)));

    for (int i = 0; i < size + 1; i++)
    {
        labels << QString::number(i);
    }
    ui->tabwFitResult->setVerticalHeaderLabels(labels);

    for (int i = 0; i < 3; i++)
    {
        ui->tabwFitResult->setItem(size, 2 * i + 1, new QTableWidgetItem(QString::number(par[i])));
        ui->tabwFitResult->setItem(size, 2 * i + 2, new QTableWidgetItem(QString::number(parE[i])));
    }
    fdrawWin->getROOTWidget()->Update();
}
