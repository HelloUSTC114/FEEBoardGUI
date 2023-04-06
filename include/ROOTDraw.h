#ifndef ROOTWIDGET_H
#define ROOTWIDGET_H

#pragma once
// QTCanvas
// Qt
#include <QWidget>
// std
#include <vector>
#include <string>

class QCloseEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QButtonGroup;

class TObject;
class TCanvas;
class TH1;

class PlotWindow;

class ROOTWidget : public QWidget
{
    Q_OBJECT
public:
    friend class PlotWindow;
    ROOTWidget(QWidget *parent = nullptr);
    ROOTWidget(ROOTWidget &item);

    ROOTWidget::~ROOTWidget();

    TCanvas *getCanvas() const;
    // TCanvas wrapper functions
    void cd(const int &ipad = 0);
    void divide(const int &ncols, const int &nrows);
    void Update(); // uppercase to avoid conflict with QWidget::update()

private:
    TCanvas *canvas = NULL;                  //
    std::vector<std::string> fListOfObjName; // stash all objects name in TCanvas;

    void setROOTLogon();
    void on_PlotWindowClosing();

    // // Functions to capture QWidget events and pass them to ROOT
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void changeEvent(QEvent *);

protected:
signals:

public slots:
};

#pragma once
// Qt
#include <QGroupBox>
// ROOT
#include <TH1D.h>
#include <QTimer>
// std

/*! \class PlotWindow
 * A window containing a TCanvas embedded in a QWidget. */
class PlotWindow : public QWidget
{
    Q_OBJECT

public:
    // PlotWindow(const int &wid);
    PlotWindow(const int &wid, QWidget *parent = nullptr); //
    PlotWindow(PlotWindow &item);                          // Clone item
    ~PlotWindow();

    // Required for ROOT functionality
    void changeEvent(QEvent *e);
    int GetWinID() { return winID; }
    ROOTWidget *getROOTWidget() { return fRTWidget; }
    void cd()
    {
        if (fRTWidget)
            fRTWidget->cd();
    }
    void Update()
    {
        if (fRTWidget)
            fRTWidget->Update();
    }

private:
    ROOTWidget *fRTWidget;
    QTimer rootTimer;
    int winID;
    TH1D *histogram;

    // GUI items
    QGroupBox *createPlotBox();
    QGroupBox *createPlotBox(PlotWindow &item);

private slots:
    // Must be run on a timer - forces update of ROOT event loop
    void handleROOTEvents();
public Q_SLOTS:

public:
    void closeEvent(QCloseEvent *event);
};

#endif // ROOTWIDGET_H

#ifndef ROOTDRAW_H
#define ROOTDRAW_H

#include <QMainWindow>

namespace Ui
{
    class ROOTDraw;
}

struct PeakFitValue
{
    double first = 0;  // mean value
    double second = 0; // constant value
    double sigma = 0;  // sigma value
    PeakFitValue(double x, double y, double z) : first(x), second(y), sigma(z){};
};

class FEEControlWin;
class ROOTDraw : public QMainWindow
{
    Q_OBJECT

public:
    explicit ROOTDraw(int wid = 0, QWidget *parent = nullptr);
    explicit ROOTDraw(ROOTDraw &item);
    ~ROOTDraw();

    void Update();
    void SetOccupied(FEEControlWin *feew, bool flag = 1);

    int GetDrawOption();
    bool SetDrawOption(int option);
    int GetDrawChannel();
    bool SetDrawChannel(int ch);

    void cd()
    {
        if (fPlotWin)
            fPlotWin->cd();
    }

    /// @brief Find Peaks for histogram hInput
    /// @param hInput [IN] input histogram
    /// @param outPeaks [OUT] Output peaks, <x,y,z> means <mean, constant, sigma>
    /// @param nRebins [IN] how many times for rebin, N means hInput->Rebin(2^N), set default as 0, which means hInput->Rebin(1);
    /// @param maxPeakSigma [IN] Useful for adjust Peak finding progress
    /// @return [OUT] How many peaks have been found
    static int FindPeaks(TH1 *hInput, std::vector<PeakFitValue> &outPeaks, int nRebins = 0, double maxPeakSigma = 10);

public:
private:
    void Setup();
    Ui::ROOTDraw *ui;
    int fWinID;

    PlotWindow *fPlotWin;

    // Draw Control
    bool fDrawFlag = 0;               // DAQ runing flag
    QTimer fDrawerTimer;              //
    QString sReadFile;                //
    QButtonGroup *fpbtngrpDrawOption; // Draw option button group

    bool fCanvasOccupied = 0;    // Whether this Canvas is occupied by a FEEControlWin
    FEEControlWin *fFeeW = NULL; // Used to tell FEE control win to change draw channel

    void DrawFiberSingle(); //
    void closeEvent(QCloseEvent *event);

    void EnableFitPeaks(bool flag = 1);

private slots:
    // Slots:
    void on_btnFileChoose_clicked();
    void on_btnFileDraw_clicked();
    void on_btnFileClose_2_clicked();
    void on_boxFiberCh_2_textChanged(const QString &arg1);
    void on_btnFitPeaks_clicked();
};

#endif // ROOTDRAW_H
