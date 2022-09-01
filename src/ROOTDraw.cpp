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
ROOTWidget::ROOTWidget(ROOTWidget &item)
    : QWidget{(QWidget *)item.parent()}, canvas(nullptr)
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
    std::cout <<"Drawing: " <<std::endl;
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

ROOTDraw::ROOTDraw(ROOTDraw &item) : fWinID(item.fWinID + 1), QMainWindow((QWidget *)item.parent()), ui(new Ui::ROOTDraw)
{
    fPlotWin = new PlotWindow(*item.fPlotWin);
    fCanvasOccupied = item.fCanvasOccupied;
    fFeeW = item.fFeeW;
    Setup();
}

void ROOTDraw::Setup()
{
    ui->setupUi(this);

    // ROOT Canvas FEEControlWin:
    auto grid = new QGridLayout(ui->frmDraw);
    grid->addWidget(fPlotWin, 0, 0, 7, 1);
}

void ROOTDraw::closeEvent(QCloseEvent *event)
{
    fPlotWin->closeEvent(event);
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
        fFeeW->SetDrawChannel(ui->boxFiberCh_2->value());
        return;
    }
    if (!gReadManager->IsOpen())
        return;
    gReadManager->DrawHG(ui->boxFiberCh_2->value());
    Update();
}

void ROOTDraw::on_btnFileClose_2_clicked()
{
    gReadManager->Close();
    Update();
}

void ROOTDraw::on_boxFiberCh_2_textChanged(const QString &arg1)
{
    on_btnFileDraw_clicked();
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
