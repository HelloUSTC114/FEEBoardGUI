#ifndef FTANALYZERWIN_H
#define FTANALYZERWIN_H

#include <QMainWindow>

class QStandardItemModel;
class PlotWindow;
class string;

namespace Ui
{
    class FTAnalyzerWin;
}

class FTAnalyzerWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit FTAnalyzerWin(QWidget *parent = nullptr);
    ~FTAnalyzerWin();

private slots:
    void on_btnFTFolder_clicked();
    void on_tabwFiles_cellActivated(int row, int column);
    void on_boxSelectCh_textChanged(const QString &arg1);
    void on_btnFit_clicked();

private:
    Ui::FTAnalyzerWin *ui;

    // File List
    QString sFolder;
    std::string fFileName;
    PlotWindow *fdrawWin = NULL;
    bool fTabActivated = 0;

    // Fit Result
};

#endif // FTANALYZERWIN_H
