// Qt
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QTime>

// C++ STL
#include <fstream>
#include <iostream>
#include <time.h>

// ROOT
#include <TApplication.h>

// User include
#include "datamanager.h"
#include "FEEControlWidget.h"
// #include "feecontrol.h"
#include "configfileparser.h"
#include "ROOTDraw.h"
#include "VDeviceController.h"
#include "VisaDAQControl.h"

int main(int argc, char *argv[])
{
    QApplication qapp(argc, argv);
    new TApplication("QTCanvas Demo", &argc, argv);

    {
        gFEEControlWin->show();
        // gVisaDAQWin->show();
        return qapp.exec();
        // gAFGVisa;
        // // // std::cout << gAFGVisa->WriteCMD("output1 on") << std::endl;
        // std::cout << gAFGVisa->SetChannelStatus(1, 1) << std::endl;


        // gAgi1344Visa->InitMeasure();
        // auto start = clock();
        // for (int i = 0; i < 40; i++)
        // {
        //     auto start0 = clock();
        //     double mes = gAgi1344Visa->MeasureOnce();
        //     std::cout << i << '\t' << mes << '\t' << "Start time: " << start0 << "\t running time: " << clock() - start0 << std::endl;
        // }
        // std::cout << "Total running time: " << clock()-start << '\t' << std::endl;

        // gAgi1344Visa->InitMeasure34410();
        // std::vector<double> vResult;
        // gAgi1344Visa->Measure34410(vResult);
        // return 1;
    }

    // TestDevice a;
    // a.StartTest();

    //    std::vector<Device> deviceList;

    //    ZaberConnectionManager conM;
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

    //    {
    //        // FEEControlWin w;
    //        // w.show();
    //        gFEEControlWin->show();
    //        return qapp.exec();
    //    }

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

    // {
    //     // gFEEControlWin->Test();
    //     gFEEControlWin->show();
    //     // ROOTDraw a;
    //     // a.show();
    //     return qapp.exec();
    //     return 1;
    // }

    // /*! \class dataManager test
    //  *
    //  */

    // {
    //     auto datm = new DataManager("testtdc.root");
    //     std::ifstream fin("tdc_data.dat");
    //     const int readnum = 500;
    //     uint32_t readdata[readnum];

    //     // for (int i = 0; i < readnum; i++)
    //     // {
    //     //     fin >> readdata[i];
    //     //     // std::cout << readdata[i] << std::endl;
    //     // }
    //     int eventCounts = 0;
    //     for (int readtime = 0; fin.good() && fin.is_open(); readtime++)
    //     // for (int readtime = 0; readtime < 10; readtime++)
    //     {
    //         std::cout << "ReadTime: " << readtime << '\t' << "Read Points: " << readtime * readnum << std::endl;
    //         for (int i = 0; i < readnum; i++)
    //         {
    //             fin >> readdata[i];
    //             // std::cout << readdata[i] << std::endl;
    //         }
    //         int rtn = datm->ProcessTDCEvents(readdata, readnum);
    //         eventCounts += rtn;
    //         std::cout << "Processed Event: " << rtn << " Total: " << eventCounts << std::endl;
    //         // datm->PrintTDCBuffer();
    //     }

    //     datm->Close();
    //     fin.close();
    //     return 1;
    // }

    // {
    //     auto datm = new DataManager("test.root");
    //     std::ifstream fin("hg_data.dat");
    //     const int readnum = 5000;
    //     uint32_t readdata[readnum];

    //     // for (int i = 0; i < readnum; i++)
    //     // {
    //     //     fin >> readdata[i];
    //     //     // std::cout << readdata[i] << std::endl;
    //     // }
    //     for (int readtime = 0; fin.good() && fin.is_open(); readtime++)
    //     {
    //         std::cout << "ReadTime: " << readtime << '\t' << "Read Points: " << readtime * readnum << std::endl;
    //         for (int i = 0; i < readnum; i++)
    //         {
    //             fin >> readdata[i];
    //             // std::cout << readdata[i] << std::endl;
    //         }
    //         int rtn = datm->ProcessADCEvents(0, readdata, readnum);
    //         std::cout << "Processed Event: " << rtn << std::endl;
    //         // datm->PrintHGBuffer();
    //     }

    //     datm->Close();
    //     fin.close();
    //     return 1;
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
