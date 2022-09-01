//#include "datamanager.h"
#include "FEEControlWidget.h"
#include <QApplication>

// This three headers are conflicted with each other, should arrage like: ROOT, ZML, winsock
// #include "feecontrol.h"
#include "configfileparser.h"
#include "ROOTDraw.h"

#include <QDateTime>

#include <QDir>
#include <QTime>

#include <fstream>

// ROOT
#include <TApplication.h>

int main(int argc, char *argv[])
{
    QApplication qapp(argc, argv);
    new TApplication("QTCanvas Demo", &argc, argv);

    //    std::vector<Device> deviceList;

    //    ConnectionManager conM;
    //    conM.ScanDevice();
    //    cout << conM.getPortNameNow() << endl;
    //    cout << gcon.toString() << endl;

    //    deviceList = conM.getDeviceList();
    //    std::cout << "Successful! Found " << deviceList.size() << " devices." << std::endl;

    //    Device dev = deviceList[0];
    //    dev.home();
    //    dev.moveAbsolute(2, Units::LENGTH_MILLIMETRES);

    //    gconm->ScanDevice();
    //    std::vector<Device> deviceList;
    //    deviceList = gconm->getDeviceList();
    //    std::cout << "Successful! Found " << deviceList.size() << " devices." << std::endl;

    //    Device dev = deviceList[0];
    //    dev.home();
    //    dev.moveAbsolute(2, Units::LENGTH_MILLIMETRES);

    //    FEEControl board("192.168.1.115", 1306);
    //    board.ReadFifo();
    //    DataManager data;
    //    data.ProcessFEEData(&board);

    {
        // FEEControlWin w;
        // w.show();
        gFEEControlWin->show();
        return qapp.exec();
    }

    // {
    //     ZaberFunctionTestClass test;
    //     gZaberWindow->show();
    //     test.startMove();
    //     return qapp.exec();
    // }

    // {
    //     QTime a;
    //     a = QTime::fromMSecsSinceStartOfDay(10000);
    //     // std::cout << a.toString("yyyy-MM-dd-hh-mm-ss").toStdString() << std::endl;
    //     std::cout << a.isValid() << std::endl;
    //     std::cout << a.minute() << '\t' << a.second() << '\t' << a.msec() << std::endl;

    //     QTime b(0, 0, 0, 10);
    //     // std::cout << b.toString("yyyy-MM-dd-hh-mm-ss").toStdString() << std::endl;
    //     std::cout << b.isValid() << std::endl;
    //     std::cout << b.minute() << '\t' << b.second() << '\t' << b.msec() << std::endl;
    //     return 1;
    // }

//     {
//         ROOTDraw canvas;
//         canvas.show();
//         auto h = new TH1D("h", "h", 100, -1, 1);
//         h->FillRandom("gaus");
//         h->Draw("hist");
//         return qapp.exec();
//     }

//     {
////         PlotWindow win(0);
////         win.show();
//         ROOTWidget a;
//         a.show();
//         return qapp.exec();
//     }

    // {
    //     FTFolderParser ftParser;
    //     ftParser.Init("F:/Projects/MuonTestControl/Data/FiberTest-0811/Data");
    //     ftParser.Print();
    //     // auto ftime = new TDatime;
    //     // delete ftime;
    // }

    // {
    //     FTAnalyzerWin win;
    //     win.show();
    //     return qapp.exec();
    // }

    return qapp.exec();
    // return 1;

    // {
    //     // Test Read Fifo Code
    //     using namespace std;
    //     gBoard->InitPort("192.168.1.115", 1306);

    //     gBoard->logic_select(0);
    //     gParser->Init();
    //     gBoard->SendConfig(gParser);

    //     gDataManager->Init((QDir::currentPath() + "/../MuonTestControl/Data.root").toStdString());
    //     for (int event = 0; event < 100;)
    //     {
    //         cout << "Reading: " << event << " events." << endl;
    //         gBoard->ReadFifo();
    //         event += gDataManager->ProcessFEEData(gBoard);
    //     }

    //     // auto rtn = gBoard->ReadFifo();
    //     // cout << endl
    //     //      << rtn << endl;
    //     // auto dat = gBoard->GetTestFIFOData();
    //     // for (int i = 0; i < gBoard->GetDataLength(); i++)
    //     // {
    //     //     cout << dat[i] << ' ';
    //     // }

    //     gDataManager->Close();
    // }

    //    {
    //        using namespace std;
    //        QString s1 = "./asdflasdjf/sdafasdf/scfsla.txt";
    //        std::cout <<s1.toStdString() << std::endl;
    //        auto sl = s1.split('/');
    //        cout << sl.at(sl.size()-1).toStdString()<< endl;
    //    }

    // QDir dir;
    // std::string sc_file_name = "./Configuration/sc_register_pd.txt";
    // std::string probe_file_name = "./Configuration/probe_register.txt";

    // std::string sPath = dir.currentPath().toStdString() + "/../MuonTestControl/";

    // std::string sVoid;
    // ConfigFileParser::ProcessConfigFile(sPath + sc_file_name, sPath + probe_file_name, sVoid);

    //    ConfigFileParser file;
    //    using namespace std;
    //    file.Init();
    //    {
    //        int bias = 255;
    //        std::cout << ConfigFileParser::ConvertBiasDAC(bias) << std::endl;
    //        std::cout << ConfigFileParser::ConvertBiasDAC(ConfigFileParser::ConvertBiasDAC(bias)) << std::endl;

    //        std::cout << "channel bias: " << file.GetBiasDAC(2) << std::endl;
    //        file.SetBiasDAC(2, 10);
    //        std::cout << "channel bias: " << file.GetBiasDAC(2) << std::endl;

    //        cout << "channel bias switcher: " << file.GetBiasDACSwitch(2) << endl;
    //        file.EnableBiasDAC(2, 0);
    //        cout << "channel bias switcher: " << file.GetBiasDACSwitch(2) << endl;

    //        cout << "bias Ref: " << file.GetDACRef() << endl;
    //        file.SetDACRef(0);
    //        cout << "bias Ref: " << file.GetDACRef() << endl;

    //        cout << "AMP HG DAC: " << file.Get_AMP_HG_DAC(2) << endl;
    //        file.Set_AMP_HG_DAC(2, 12);
    //        cout << "AMP HG DAC: " << file.Get_AMP_HG_DAC(2) << endl;

    //        cout << "AMP LG DAC: " << file.Get_AMP_LG_DAC(2) << endl;
    //        file.Set_AMP_LG_DAC(2, 12);
    //        cout << "AMP LG DAC: " << file.Get_AMP_LG_DAC(2) << endl;

    //        cout << "Charge Threshold: " << file.GetDiscDAC1() << endl;
    //        file.SetDiscDAC1(250);
    //        cout << "Charge Threshold: " << file.GetDiscDAC1() << endl;

    //        cout << "Time Threshold: " << file.GetDiscDAC2() << endl;
    //        file.SetDiscDAC2(250);
    //        cout << "Time Threshold: " << file.GetDiscDAC2() << endl;

    //        cout << "PreAmp Switcher: " << file.Get_PA_Switcher(2) << endl;
    //        file.DisablePA(2, 1);
    //        cout << "PreAmp Switcher: " << file.Get_PA_Switcher(2) << endl;
    //    }

    // {
    //     // string s = "45";
    //     // int amp = 45;
    //     string amp = "110011";
    //     cout << ConfigFileParser::ConvertAmp(amp) << endl;
    //     cout << ConfigFileParser::ConvertAmp(ConfigFileParser::ConvertAmp(amp)) << endl;

    //     cout << endl;

    //     int dac = 45;
    //     // string dac = "0011100000";
    //     cout << ConfigFileParser::ConvertDiscDAC(dac) << endl;
    //     cout << ConfigFileParser::ConvertDiscDAC(ConfigFileParser::ConvertDiscDAC(dac)) << endl;
    // }

    // FEEControl t("192.168.1.115", 1306);
    // std::cout << t.GetIP() << std::endl;

    //    std::cout << gBoard->GetIP() << '\t' << gBoard->GetPort() << std::endl;
    //     std::cout << gBoard->TestConnect() << std::endl;
    ////    std::cout << gBoard->ReadFreq() << std::endl;
    ////    std::cout << gBoard->TestReg() << std::endl;
    ////    gBoard->HVMonitor();
    ////    gBoard->HVPrint();
    //    std::cout << gBoard->GetSock() << std::endl;

    //    {
    //        QDateTime time(QDateTime::currentDateTime());
    //        ConfigFileParser file;
    //        using namespace std;
    //        file.Init();

    //        auto path = QDir::currentPath() + "/../MuonTestControl/";
    //        QFile::remove(path+"scTest.txt");
    //        QFile::remove(path+"probeTest.txt");
    //        file.PrintConfigFile((path+"scTest.txt").toStdString(), (path+"probeTest.txt").toStdString());
    //        file.Init((path+"scTest.txt").toStdString(), (path+"probeTest.txt").toStdString());
    //        file.PrintConfigFile((path+"scTest2.txt").toStdString(), (path+"probeTest2.txt").toStdString());
    //    }

    return 1;

    return 1;
}
