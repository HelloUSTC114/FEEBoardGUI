#ifndef GENERAL_H
#define GENERAL_H

// Qt
#include <QTime>

// C++
#include <string>
#include <vector>

namespace UserDefine
{
    struct DAQRequestInfo
    {
        std::string sPath;
        std::string sFileName;
        int nDAQCount;
        QTime DAQTime;
        int msBufferSleep;
        int leastBufferEvent;
        bool clearQueueFlag;
    };

    bool ParseLine(std::string sInput, std::vector<double> &vOutput);
}

#endif
