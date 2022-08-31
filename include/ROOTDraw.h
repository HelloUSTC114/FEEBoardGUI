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

class TObject;
class TCanvas;

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
    void Update() { fRTWidget->Update(); }

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

public:
private:
    void Setup();
    Ui::ROOTDraw *ui;
    int fWinID;

    PlotWindow *fPlotWin;

    // Draw Control
    bool fDrawFlag = 0;  // DAQ runing flag
    QTimer fDrawerTimer; //
    QString sReadFile;   //

    bool fCanvasOccupied = 0;    // Whether this Canvas is occupied by a FEEControlWin
    FEEControlWin *fFeeW = NULL; // Used to tell FEE control win to change draw channel

    void DrawFiberSingle(); //
    void closeEvent(QCloseEvent *event);

private slots:
    // Slots:
    void on_btnFileChoose_clicked();
    void on_btnFileDraw_clicked();
    void on_btnFileClose_2_clicked();
    void on_boxFiberCh_2_textChanged(const QString &arg1);
};

#endif // ROOTDRAW_H
