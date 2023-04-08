// QTCanvas
#include "../include/ROOTDraw.h"
#include "ui_ROOTDraw.h"

// Qt
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QEvent>
#include <QGridLayout>
#include <QButtonGroup>

// std
#include <iostream>

// ROOT
#include <TH1D.h>
#include <TCanvas.h>
#include <TRandom3.h>
#include <TVirtualX.h>
#include <TCanvas.h>

ROOTWidget::ROOTWidget(QWidget *parent)
    : QWidget{parent}, canvas(nullptr)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setMinimumSize(700, 500);
    setUpdatesEnabled(kFALSE);
    setMouseTracking(kTRUE);
    // Register the QWidget in TVirtualX, giving its native window id
    int wid = gVirtualX->AddWindow((ULong_t)winId(), width(), height());

    // Create the ROOT Tcanvas, giving as argument the QWidget registered id
    canvas = new TCanvas("Root Canvas", width(), height(), wid);
    canvas->Clear();
    canvas->SetBorderMode(0);
    canvas->SetFillColor(0);
}

#include <TList.h>
#include <TFile.h>
#include "datamanager.h"
ROOTWidget::ROOTWidget(ROOTWidget &item) : ROOTWidget((QWidget *)(item.parent()))
// : QWidget{(QWidget *)item.parent()}, canvas(nullptr)
{
    // setAttribute(Qt::WA_OpaquePaintEvent, true);
    // setMinimumSize(700, 500);
    // setUpdatesEnabled(kFALSE);
    // setMouseTracking(kTRUE);
    // // Register the QWidget in TVirtualX, giving its native window id
    // int wid = gVirtualX->AddWindow((ULong_t)winId(), width(), height());

    // // Create the ROOT Tcanvas, giving as argument the QWidget registered id
    // canvas = new TCanvas("Root Canvas", width(), height(), wid);
    // canvas->Clear();
    // canvas->SetBorderMode(0);
    // canvas->SetFillColor(0);

    std::vector<std::string> &list2 = item.fListOfObjName;

    if (!gFile)
        return; // if all files are closed, than do not try to draw anything
    for (int i = 0; i < list2.size(); i++)
    {
        canvas->cd();
        auto objName = list2.at(i);

        // Find Object
        auto test = gFile;
        TObject *obj = gFile->FindObjectAnyFile(objName.c_str());
        if (obj)
            obj->Draw();
    }
}

void ROOTWidget::on_PlotWindowClosing()
{
    auto list1 = canvas->GetListOfPrimitives();
    for (int i = 0; i < list1->GetEntries(); i++)
    {
        canvas->cd();
        auto obj = list1->At(i);
        fListOfObjName.push_back(obj->GetName());
    }
}

/*! Destructor. */
ROOTWidget::~ROOTWidget()
{
    delete canvas;
    canvas = NULL;
}

/*! Get the pointer to the ROOT canvas. */
TCanvas *ROOTWidget::getCanvas() const
{
    return canvas;
}

/*! Switch the ROOT current directory to this canvas. */
void ROOTWidget::cd(const int &ipad)
{
    if (canvas)
    {
        canvas->cd(ipad);
    }
    return;
}

/*! Divide the canvas into multiple pads. Uses the same syntax as
 * TCanvas::Divide(). */
void ROOTWidget::divide(const int &ncols, const int &nrows)
{
    canvas->Divide(ncols, nrows);
    return;
}

/*! Force an update of this canvas. */
void ROOTWidget::Update()
{
    if (canvas)
    {
        canvas->Modified();
        canvas->Resize();
        canvas->Update();
    }
    return;
}

/*! Set ROOT styles, placement, etc. Taken from the NOvA project. */
#include "TStyle.h"
#include "TGaxis.h"
#include "TROOT.h"
void ROOTWidget::setROOTLogon()
{
    // Defaults to classic style, but that's OK, we can fix it
    TStyle *novaStyle = new TStyle("novaStyle", "NOvA Style");

    // Centre title
    novaStyle->SetTitleAlign(22);
    novaStyle->SetTitleX((float)(.5));
    novaStyle->SetTitleY((float)(.95));
    novaStyle->SetTitleBorderSize(0);

    // No info box
    // novaStyle->SetOptStat(0);
    novaStyle->SetOptStat(111);
    novaStyle->SetOptFit(111);

    // set the background color to white
    novaStyle->SetFillColor(10);
    novaStyle->SetFrameFillColor(10);
    novaStyle->SetCanvasColor(10);
    novaStyle->SetPadColor(10);
    novaStyle->SetTitleFillColor(0);
    novaStyle->SetStatColor(10);

    // Don't put a colored frame around the plots
    novaStyle->SetFrameBorderMode(0);
    novaStyle->SetCanvasBorderMode(0);
    novaStyle->SetPadBorderMode(0);

    // Set the default line color for a fit function to be red
    novaStyle->SetFuncColor(kRed);

    // Marker settings
    //  novaStyle->SetMarkerStyle(kFullCircle);

    // No border on legends
    novaStyle->SetLegendBorderSize(0);

    // Disabled for violating NOvA style guidelines
    // Scientific notation on axes
    TGaxis::SetMaxDigits(3);

    // Axis titles
    novaStyle->SetTitleSize((float)(.055), "xyz");
    novaStyle->SetTitleOffset((float)(.8), "xyz");
    // More space for y-axis to avoid clashing with big numbers
    novaStyle->SetTitleOffset((float)(.9), "y");
    // This applies the same settings to the overall plot title
    novaStyle->SetTitleSize((float)(.055), "");
    novaStyle->SetTitleOffset((float)(.8), "");
    // Axis labels (numbering)
    novaStyle->SetLabelSize((float)(.04), "xyz");
    novaStyle->SetLabelOffset((float)(.005), "xyz");

    // Prevent ROOT from occasionally automatically zero-suppressing
    novaStyle->SetHistMinimumZero();

    // Thicker lines
    novaStyle->SetHistLineWidth(1);
    novaStyle->SetFrameLineWidth(2);
    novaStyle->SetFuncWidth(2);

    // Set the number of tick marks to show
    novaStyle->SetNdivisions(506, "xyz");

    // Set the tick mark style
    novaStyle->SetPadTickX(1);
    novaStyle->SetPadTickY(1);

    // Fonts
    const int kNovaFont = 42;
    novaStyle->SetStatFont(kNovaFont);
    novaStyle->SetLabelFont(kNovaFont, "xyz");
    novaStyle->SetTitleFont(kNovaFont, "xyz");
    novaStyle->SetTitleFont(kNovaFont, ""); // Apply same setting to plot titles
    novaStyle->SetTextFont(kNovaFont);
    novaStyle->SetLegendFont(kNovaFont);

    // Get moodier colours for colz
    const Int_t NRGBs = 5;
    const Int_t NCont = 255;
    Double_t stops[NRGBs] = {0.00, 0.34, 0.61, 0.84, 1.00};
    Double_t red[NRGBs] = {0.00, 0.00, 0.87, 1.00, 0.51};
    Double_t green[NRGBs] = {0.00, 0.81, 1.00, 0.20, 0.00};
    Double_t blue[NRGBs] = {0.51, 1.00, 0.12, 0.00, 0.00};
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    novaStyle->SetNumberContours(NCont);

    gROOT->SetStyle("novaStyle");

    // Uncomment this line if you want to force all plots loaded from files
    // to use this same style
    gROOT->ForceStyle();
    return;
}

/*! Capture the mouse movement. */
void ROOTWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (canvas)
    {
        if (e->buttons() & Qt::LeftButton)
        {
            canvas->HandleInput(kButton1Motion, e->x(), e->y());
        }
        else if (e->buttons() & Qt::MiddleButton)
        {
            canvas->HandleInput(kButton2Motion, e->x(), e->y());
        }
        else if (e->buttons() & Qt::RightButton)
        {
            canvas->HandleInput(kButton3Motion, e->x(), e->y());
        }
        else
        {
            canvas->HandleInput(kMouseMotion, e->x(), e->y());
        }
    }
    return;
}

/*! Capture the mouse clicks. */
void ROOTWidget::mousePressEvent(QMouseEvent *e)
{
    if (canvas)
    {
        switch (e->button())
        {
        case Qt::LeftButton:
            canvas->HandleInput(kButton1Down, e->x(), e->y());
            break;

        case Qt::MiddleButton:
            canvas->HandleInput(kButton2Down, e->x(), e->y());
            break;

        case Qt::RightButton:
            canvas->HandleInput(kButton3Down, e->x(), e->y());
            break;

        default:
            break;
        }
    }
    return;
}

/*! Handle mouse releases. */
void ROOTWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (canvas)
    {
        switch (e->button())
        {
        case Qt::LeftButton:
            canvas->HandleInput(kButton1Up, e->x(), e->y());
            break;

        case Qt::MiddleButton:
            canvas->HandleInput(kButton2Up, e->x(), e->y());
            break;

        case Qt::RightButton:
            canvas->HandleInput(kButton3Up, e->x(), e->y());
            break;

        default:
            break;
        }
    }
    return;
}

/*! Force an update when a new paint happens for this widget. */
void ROOTWidget::paintEvent(QPaintEvent *)
{
    if (canvas)
    {
        Update();
    }
    return;
}

/*! Force an update when a resize happens for this widget. */
void ROOTWidget::resizeEvent(QResizeEvent *)
{
    if (canvas)
    {
        Update();
    }
    return;
}

/*! Force an update when a change happens for this widget. */
void ROOTWidget::changeEvent(QEvent *)
{
    if (canvas)
    {
        Update();
    }
    return;
}

// Qt
#include <QGridLayout>
// ROOT
#include <TRandom3.h>
// std
#include <string>

/*! Constructor. */
PlotWindow::PlotWindow(const int &wid, QWidget *parent) : QWidget(parent),
                                                          // PlotWindow::PlotWindow(const int &wid) : QWidget(nullptr),
                                                          winID(wid)
{
    // Window properties
    setMinimumSize(800, 600);
    std::string title = "Plot " + std::to_string(wid);
    setWindowTitle(title.c_str());

    // GUI elements
    auto grid = new QGridLayout(this);
    grid->addWidget(createPlotBox(), 0, 0, 7, 1);
    setLayout(grid);
    resize(1000, 800);

    // show();

    // Tell the QTCanvas to Update *after* we have placed all the widgets and
    // displayed the window to make sure it draws/draws correctly
    fRTWidget->Update();

    // Window properties
    setMinimumSize(250, 400);

    // Set the ROOT event timer, which allows ROOT to process its events
    // periodically. Should be done in the window you expect to be the last
    // one closed
    connect(&rootTimer, SIGNAL(timeout()),
            this, SLOT(handleROOTEvents()));
    rootTimer.start(20); // ms
    rootTimer.setTimerType(Qt::VeryCoarseTimer);
}

PlotWindow::PlotWindow(PlotWindow &item) : QWidget((QWidget *)item.parent()),
                                           winID(item.winID + 1)
{
    // Window properties
    setMinimumSize(800, 600);
    std::string title = "Plot " + std::to_string(winID);
    setWindowTitle(title.c_str());

    // GUI elements
    auto grid = new QGridLayout(this);
    grid->addWidget(createPlotBox(item), 0, 0, 7, 1);
    setLayout(grid);
    resize(1000, 800);

    // show();

    // Tell the QTCanvas to Update *after* we have placed all the widgets and
    // displayed the window to make sure it draws/draws correctly
    fRTWidget->Update();

    // Window properties
    setMinimumSize(250, 400);

    // Set the ROOT event timer, which allows ROOT to process its events
    // periodically. Should be done in the window you expect to be the last
    // one closed
    connect(&rootTimer, SIGNAL(timeout()),
            this, SLOT(handleROOTEvents()));
    rootTimer.start(20); // ms
    rootTimer.setTimerType(Qt::VeryCoarseTimer);
}

/*! Force ROOT to process its events. */
#include <TSystem.h>
void PlotWindow::handleROOTEvents()
{
    gSystem->ProcessEvents();
    return;
}

/*! Destructor. */
PlotWindow::~PlotWindow()
{
}

/*! Propagate changes to the window's state to ROOT for interactivity. */
void PlotWindow::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::WindowStateChange)
    {
        auto event = static_cast<QWindowStateChangeEvent *>(e);
        if ((event->oldState() & Qt::WindowMaximized) || (event->oldState() & Qt::WindowMinimized) || (event->oldState() == Qt::WindowNoState && this->windowState() == Qt::WindowMaximized))
        {
            if (fRTWidget->getCanvas())
            {
                fRTWidget->getCanvas()->Resize();
                fRTWidget->getCanvas()->Update();
            }
        }
    }
    return;
}

#include <TFile.h>
/*! Create the Plot box. */
QGroupBox *PlotWindow::createPlotBox()
{
    auto gbox = new QGroupBox(tr("Plot"));
    auto grid = new QGridLayout(gbox);

    // Canvas
    fRTWidget = new ROOTWidget;
    // grid becomes the parent of QTCanvas - no need to worry about cleaning
    // it up
    grid->addWidget(fRTWidget);

    // Create a histogram to draw in the fRTWidget
    fRTWidget->cd();
    // Seed with winID so we don't just generate the same plot over and over
    TRandom3 r(winID);
    // Generate a name unique to this window to avoid histogram name collision
    std::string hname = "hGaus" + std::to_string(winID);
    histogram = new TH1D(hname.c_str(), "Draw Test: Gaussian Distribution", 100, 0, 100);
    for (int i = 0; i < 10000; i++)
    {
        histogram->Fill(r.Gaus(50.0, 5.0));
    }
    // histogram->Draw("hist");
    histogram->Draw("");
    return gbox;
}

QGroupBox *PlotWindow::createPlotBox(PlotWindow &item)
{

    auto gbox = new QGroupBox(tr("Plot"));
    auto grid = new QGridLayout(gbox);

    // Canvas
    fRTWidget = new ROOTWidget(*item.fRTWidget);
    // grid becomes the parent of QTCanvas - no need to worry about cleaning
    // it up
    grid->addWidget(fRTWidget);

    // Create a histogram to draw in the fRTWidget
    fRTWidget->cd();
    return gbox;
}

void PlotWindow::closeEvent(QCloseEvent *close)
{
    fRTWidget->on_PlotWindowClosing();
}

#include "ROOTDraw.h"
#include "ui_ROOTDraw.h"

#include <QGridLayout>

ROOTDraw::ROOTDraw(int wid, QWidget *parent) : fWinID(wid), QMainWindow(parent),
                                               ui(new Ui::ROOTDraw)
{
    fPlotWin = new PlotWindow(wid, parent);
    Setup();
}

#include <QPushButton>
ROOTDraw::ROOTDraw(ROOTDraw &item) : fWinID(item.fWinID + 1), QMainWindow((QWidget *)item.parent()), ui(new Ui::ROOTDraw)
{
    fPlotWin = new PlotWindow(*item.fPlotWin);
    fCanvasOccupied = item.fCanvasOccupied;
    fFeeW = item.fFeeW;
    Setup();
}

#include <TH1.h>
#include <TSpectrum.h>
#include <TF1.h>
int ROOTDraw::FindPeaks(TH1 *hInput, std::vector<PeakFitValue> &outPeaks, int nRebins, double maxPeakSigma)
{
    static TSpectrum *sp = new TSpectrum;
    if (nRebins > 0)
        hInput->Rebin((int)TMath::Power(2, nRebins));

    sp->Search(hInput, maxPeakSigma);
    hInput->Draw();
    sp->Draw("same");

    std::vector<PeakFitValue> tempPeaks;
    for (int i = 0; i < sp->GetNPeaks(); i++)
        tempPeaks.push_back(PeakFitValue(sp->GetPositionX()[i], sp->GetPositionY()[i], 0));
    std::sort(tempPeaks.begin(), tempPeaks.end(), [](const PeakFitValue &temp1, const PeakFitValue &temp2)
              { return temp1.first < temp2.first; });

    if (tempPeaks.size() <= 1)
    {
        std::cout << "Error: only found 1 peak. return false." << std::endl;
        return 0;
    }

    for (int peakNo = 0; peakNo < tempPeaks.size(); peakNo++)
    {
        double dev2Next = 0;
        if (peakNo == 0)
        {
            dev2Next = tempPeaks[1].first - tempPeaks[0].first;
        }
        else
        {
            dev2Next = tempPeaks[peakNo].first - tempPeaks[peakNo - 1].first;
        }

        double currentPeak = tempPeaks[peakNo].first;
        static int count = 0;
        auto f0 = new TF1(Form("gausn%d", count++), "gausn", 0, 65536);
        TF1 &f = *f0;
        if (peakNo == 0)
            hInput->Fit(&f, "", "", currentPeak - dev2Next * 0.3, currentPeak + dev2Next * 0.3);
        else
            hInput->Fit(&f, "+", "", currentPeak - dev2Next * 0.3, currentPeak + dev2Next * 0.3);
        double fitCurrentPeak = f.GetParameter(1);
        double fitPeakAmplitude = f.GetParameter(0);
        if (fitCurrentPeak <= currentPeak - dev2Next * 0.3 || fitCurrentPeak >= currentPeak + dev2Next * 0.3)
        {
            tempPeaks[peakNo].first = -1;
            tempPeaks[peakNo].second = -1;
            std::cout << "Error: Peak " << peakNo << " is out of range: [" << currentPeak - dev2Next * 0.3 << ", " << currentPeak + dev2Next * 0.3 << "]. Return false." << std::endl;
            //            return 0;
        }
        else
        {
            tempPeaks[peakNo].first = fitCurrentPeak;
            tempPeaks[peakNo].second = fitPeakAmplitude;
            tempPeaks[peakNo].sigma = f.GetParameter(2);
        }
    }

    // Process
    outPeaks.clear();
    for (int i = 0; i < tempPeaks.size(); i++)
    {
        if (tempPeaks[i].first > 0)
        {
            outPeaks.push_back(tempPeaks[i]);
        }
    }

    return outPeaks.size();
}

double GetGain(std::vector<PeakFitValue> &peakArray, std::vector<double> &devArray)
{
    for (int peak = 0; peak < peakArray.size(); peak++)
    {
        if (peak > 0)
        {
            double dev = peakArray[peak].first - peakArray[peak - 1].first;
            devArray.push_back(dev);
        }
    }
    if (devArray.size() < 4)
    {
        std::cout << "Warning: Count of peaks is too small to evaluate mean gain";
        return 0;
    }
    double gain = (devArray[1] + devArray[2] + devArray[3]) / 3.0;
    return gain;
}

void ROOTDraw::Setup()
{
    ui->setupUi(this);

    // ROOT Canvas FEEControlWin:
    auto grid = new QGridLayout(ui->frmDraw);
    grid->addWidget(fPlotWin, 0, 0, 7, 1);

    // DAQ Draw
    fpbtngrpDrawOption = new QButtonGroup(this);
    fpbtngrpDrawOption->addButton(ui->btnHGDraw, 0);
    fpbtngrpDrawOption->addButton(ui->btnLGDraw, 1);
    fpbtngrpDrawOption->addButton(ui->btnTDCDraw, 2);
    fpbtngrpDrawOption->button(0)->setChecked(1);

    // Draw control;
    connect(fpbtngrpDrawOption, SIGNAL(buttonClicked(int)), this, SLOT(on_btnFileDraw_clicked()));

    // Set Table head
    QTableWidgetItem *headerItem;
    QStringList headerText;
    headerText << "Peak No."
               << "ADC Value"
               << "Count"
               << "Sigma"
               << "Dev from pre";
    ui->tablePeaks->setColumnCount(headerText.count());
    for (int i = 0; i < ui->tablePeaks->columnCount(); i++)
    {
        headerItem = new QTableWidgetItem(headerText.at(i));
        QFont font = headerItem->font();
        font.setBold(true);
        font.setPointSize(8);
        headerItem->setFont(font);
        ui->tablePeaks->setHorizontalHeaderItem(i, headerItem);
    }
    ui->tablePeaks->verticalHeader()->setVisible(false);
    ui->tablePeaks->setEditTriggers(QAbstractItemView::NoEditTriggers);
    EnableFitPeaks(0);
}

void ROOTDraw::closeEvent(QCloseEvent *event)
{
    fPlotWin->closeEvent(event);
}

void ROOTDraw::EnableFitPeaks(bool flag)
{
    ui->grpPeakStat->setEnabled(flag);
}

void ROOTDraw::Update()
{
    if (this->isHidden())
        return;
    fPlotWin->Update();
}

ROOTDraw::~ROOTDraw()
{
    delete ui;
    delete fPlotWin;
}

void ROOTDraw::SetOccupied(FEEControlWin *feew, bool flag)
{
    fCanvasOccupied = flag;
    if (flag)
    {
        fFeeW = feew;
    }
    else
    {
        fFeeW = NULL;
    }
}

bool ROOTDraw::SetDrawOption(int option)
{
    if (option < 0 || option > 2)
        return false;
    fpbtngrpDrawOption->button(option)->setChecked(1);
    return true;
}

int ROOTDraw::GetDrawOption()
{
    return fpbtngrpDrawOption->checkedId();
}

int ROOTDraw::GetDrawChannel()
{
    return ui->boxFiberCh_2->value();
}

bool ROOTDraw::SetDrawChannel(int ch)
{
    if (ch < 0 || ch > 32)
        return false;
    ui->boxFiberCh_2->setValue(ch);
    return true;
}

void ROOTDraw::cd()
{
    if (fPlotWin)
        fPlotWin->cd();
}

#include <QFileDialog>
#include "datamanager.h"
void ROOTDraw::on_btnFileChoose_clicked()
{
    sReadFile = QFileDialog::getOpenFileName(NULL, tr("Choosing File Path"), QDir::currentPath() + "/../MuonTestControl/Data/", "*.root");
    ui->lblReadFileName->setText(sReadFile);
    if (sReadFile.isEmpty())
    {
        return;
    }
    gReadManager->Init(sReadFile.toStdString());
}

#include "FEEControlWidget.h"
void ROOTDraw::on_btnFileDraw_clicked()
{
    if (fCanvasOccupied)
    {
        fFeeW->SetDrawChannel(GetDrawChannel());
        fFeeW->SetDrawOption(GetDrawOption());
        return;
    }
    if (!gReadManager->IsOpen())
        return;

    // std::cout << "Test: Drawing: " << std::endl;
    // gReadManager->DrawHG(GetDrawChannel());
    gReadManager->Draw(GetDrawChannel(), (DrawOption)GetDrawOption());
    // std::cout << "Test: Drawed" << std::endl;
    EnableFitPeaks(true);
    Update();
}

void ROOTDraw::on_btnFileClose_2_clicked()
{
    gReadManager->Close();
    EnableFitPeaks(false);
    Update();
}

void ROOTDraw::on_boxFiberCh_2_textChanged(const QString &arg1)
{
    on_btnFileDraw_clicked();
}
void ROOTDraw::on_btnFitPeaks_clicked()
{
    if (fCanvasOccupied)
        return;

    std::string sHistoName = Form("hHG%d", GetDrawChannel());
    auto h = (TH1 *)gFile->Get(sHistoName.c_str());
    if (!h)
    {
        std::cout << "Error: Cannot find Histogram: " << std::endl;
        return;
    }
    std::vector<PeakFitValue> peakArray;
    FindPeaks(h, peakArray, ui->boxRebin->value(), ui->lineSigmaInput->text().toDouble());
    Update();

    ui->tablePeaks->clearContents();
    if (peakArray.size() < 2)
    {
        std::cout << "Error: Cannot find enough peaks." << std::endl;
        return;
    }

    std::vector<double> devArray;
    ui->tablePeaks->setRowCount(peakArray.size());
    for (int peak = 0; peak < peakArray.size(); peak++)
    {
        QTableWidgetItem *item;

        item = new QTableWidgetItem(QString::number(peak));
        ui->tablePeaks->setItem(peak, 0, item);

        item = new QTableWidgetItem(QString::number(peakArray[peak].first));
        ui->tablePeaks->setItem(peak, 1, item);

        item = new QTableWidgetItem(QString::number(peakArray[peak].sigma));
        ui->tablePeaks->setItem(peak, 3, item);

        item = new QTableWidgetItem(QString::number(peakArray[peak].second));
        ui->tablePeaks->setItem(peak, 2, item);

        item = new QTableWidgetItem;
        if (peak > 0)
        {
            double dev = peakArray[peak].first - peakArray[peak - 1].first;
            devArray.push_back(dev);
            item->setText(QString::number(dev));
        }
        ui->tablePeaks->setItem(peak, 4, item);
    }

    if (devArray.size() < 4)
    {
        std::cout << "Warning: Count of peaks is too small to evaluate mean gain";
        ui->lineGainOutput->setText(QString::number(0));
        ui->lineSPE->setText(QString::number(0));
        ui->linePed->setText(QString::number(0));
        return;
    }
    double gain = (devArray[1] + devArray[2] + devArray[3]) / 3.0;
    ui->lineGainOutput->setText(QString::number(gain));
    ui->lineSPE->setText(QString::number(peakArray[1].first));
    ui->linePed->setText(QString::number(peakArray[0].first));
}
