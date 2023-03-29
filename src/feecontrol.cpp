// QT
#include <QMutex>

// C++ STL
#include <WinSock2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// User define
#include "../include/feecontrol.h"
#include "configfileparser.h"

#pragma comment(lib, "Ws2_32.lib")

#define SIZE_STR 150

const int FEEControl::fPortBase = 1306; // Port Base for all fee
const int FEEControl::fIPBase = 101;    // IP Base for all fee

const int FEEControl::fMaxSaveEvents = 3000;                       // How many Events can be saved in one time, must be multipliers of 20
const int FEEControl::fRetrieveUnit = 10;                          // Retrieve unit, used to be 20
const int FEEControl::fHGPointFactor = 176 * 2;                    // How many HG points in one event, not so accurate, *2 means how many Bytes, used to be 1380 * 2
const int FEEControl::fLGPointFactor = FEEControl::fHGPointFactor; // How many LG points in one event, not so accurate, *2 means how many Bytes
const int FEEControl::fTDCPointFactor = 136 * 2;                   // How many TDC points in one event, not so accurate, *2 means how many Bytes

WSADATA fWsaData; // Unused unimportant data, used to put inside class FEEControl, but win sock2.h is like a piece of shit, which is conflict with <TTree.h>, <zaber>
using namespace std;
QMutex mutex;

bool operator==(const FEEControl &a, const FEEControl &b)
{
    bool flag1 = a.GetIP() == b.GetIP();
    bool flag2 = a.GetPort() == b.GetPort();
    return flag1 && flag2;
}

FEEControl::FEEControl(std::string ip, int port) : ip_address(ip), fPort(port)
{
    fTestQueueData = new uint32_t[5000];
    fHGQueueData = new uint32_t[fMaxSaveEvents * fHGPointFactor / sizeof(uint32_t)];
    fLGQueueData = new uint32_t[fMaxSaveEvents * fLGPointFactor / sizeof(uint32_t)];
    fTDCQueueData = new uint32_t[fMaxSaveEvents * fTDCPointFactor / sizeof(uint32_t)];

    fTestQueueDataU16 = new uint16_t[5000];
    fHGQueueDataU16 = new uint16_t[fMaxSaveEvents * fHGPointFactor / sizeof(uint16_t)];
    fLGQueueDataU16 = new uint16_t[fMaxSaveEvents * fLGPointFactor / sizeof(uint16_t)];
    fTDCQueueDataU16 = new uint16_t[fMaxSaveEvents * fTDCPointFactor / sizeof(uint16_t)];
}

FEEControl::FEEControl(int boardNo) : FEEControl()
{
    // John Test: Test whether arrays are malloced
    std::cout << fTestQueueData[0] << std::endl;

    InitPort(boardNo);
}

FEEControl::~FEEControl()
{
    delete[] fTestQueueData;
    delete[] fHGQueueData;
    delete[] fLGQueueData;
    delete[] fTDCQueueData;
    fTestQueueData = NULL;
    fHGQueueData = NULL;
    fLGQueueData = NULL;
    fTDCQueueData = NULL;

    delete[] fTestQueueDataU16;
    delete[] fHGQueueDataU16;
    delete[] fLGQueueDataU16;
    delete[] fTDCQueueDataU16;
    fTestQueueDataU16 = NULL;
    fHGQueueDataU16 = NULL;
    fLGQueueDataU16 = NULL;
    fTDCQueueDataU16 = NULL;
}

#include "General.h"
void FEEControl::GenerateIP(int boardNo, std::string &ip, int &port)
{
    ip = UserDefine::GetIPPrefix() + to_string(fIPBase + boardNo);
    // ip = "192.168.1." + to_string(fIPBase + boardNo);
    port = fPortBase + boardNo;
    std::cout << "IP & Port Generated." << std::endl
              << "IP: " << ip << '\t' << "Port: " << port << std::endl;
}

void FEEControl::InitPort(std::string ip, int port)
{
    ip_address = ip;
    fPort = port;
}

void FEEControl::InitPort(uint8_t boardNo)
{
    GenerateIP(boardNo, ip_address, fPort);
}

FEEControl *&FEEControl::Instance()
{
    static FEEControl *instance = new FEEControl;
    return instance;
}

// static SOCKET sock;

bool FEEControl::server_exit()
{
    if (!start_socket())
    {
        return false;
    }
    if (!send_cmd(up_exit, NULL, 1))
    {
        return false;
    }
    close_socket();
    return true;
}

bool FEEControl::citiroc1a_configure(const char *sc_file_name, const char *probe_file_name)
{
    // send and receive data
    FILE *fp_sc, *fp_probe;
    char str_get[SIZE_STR] = {0};
    char str_temp[SIZE_STR]{0};
    char str_out[1500] = "11111111";

    if ((fp_sc = fopen(sc_file_name, "r")) == NULL)
    {
        printf("can\'t open file!\n");
        return false;
    }
    if ((fp_probe = fopen(probe_file_name, "r")) == NULL)
    {
        printf("can\'t open file!\n");
        return false;
    }

    while (fgets(str_get, SIZE_STR, fp_sc) != NULL)
    {
        str_process(str_get, str_temp);
        strcat(str_out, str_temp);
    }

    while (fgets(str_get, SIZE_STR, fp_probe) != NULL)
    {
        str_process(str_get, str_temp);
        strcat(str_out, str_temp);
    }

    fclose(fp_sc);
    fclose(fp_probe);

    if (!start_socket())
    {
        return false;
    }
    if (!send_cmd(up_configCitiroc, str_out, (int)strlen(str_out) + 2))
    {
        return false;
    }
    close_socket();
    return true;
}

bool FEEControl::ad9645_reg_write(int addr, int wr_data)
{
    return reg_write(up_writeAD9645, addr, wr_data);
}

bool FEEControl::ad9645_reg_read(int addr, int &reg)
{
    return reg_read(up_readAD9645, addr, reg);
}

bool FEEControl::si570_set(double freq)
{
    if (!start_socket())
    {
        return false;
    }
    if (!send_cmd(up_setSi570Freq, (char *)(&freq), sizeof(double) + 1))
    {
        return false;
    }
    close_socket();
    return true;
}

bool FEEControl::si570_get(double &freq)
{
    if (!start_socket())
    {
        return false;
    }
    if (!send_cmd(up_getSi570Freq, NULL, 1))
    {
        return false;
    }
    if (!recv_data((char *)(&freq), sizeof(double)))
    {
        return false;
    }
    close_socket();
    return true;
}

bool FEEControl::hg_fifo_length_read(int &len)
{
    return length_read(up_getHgFifoLen, len);
}

bool FEEControl::hg_queue_length_read(int &len)
{
    return length_read(up_getHgQueueLen, len);
}

bool FEEControl::hg_data_read(int data_num, uint32_t *data_addr)
{
    return data_read(up_getHgData, data_num, data_addr);
}

bool FEEControl::lg_fifo_length_read(int &len)
{
    return length_read(up_getLgFifoLen, len);
}

bool FEEControl::lg_queue_length_read(int &len)
{
    return length_read(up_getLgQueueLen, len);
}

bool FEEControl::lg_data_read(int data_num, uint32_t *data_addr)
{
    return data_read(up_getLgData, data_num, data_addr);
}

bool FEEControl::test_fifo_length_read(int &len)
{
    return length_read(up_getTestFifoLen, len);
}

bool FEEControl::test_queue_length_read(int &len)
{
    return length_read(up_getTestQueueLen, len);
}

bool FEEControl::test_data_read(int data_num, uint32_t *data_addr)
{
    return data_read(up_getTestData, data_num, data_addr);
}

bool FEEControl::tdc_fifo_length_read(int &len)
{
    return length_read(up_getTdcFifoLen, len);
}

bool FEEControl::tdc_queue_length_read(int &len)
{
    return length_read(up_getTdcQueueLen, len);
}

bool FEEControl::tdc_data_read(int data_num, uint32_t *data_addr)
{
    return data_read(up_getTdcData, data_num, data_addr);
}

bool FEEControl::sipm_temp_read(double *temp)
{
    if (!start_socket())
    {
        return false;
    }
    if (!send_cmd(up_getSimpTemp, NULL, 1))
    {
        return false;
    }
    if (!recv_data((char *)temp, 4 * sizeof(double)))
    {
        return false;
    }
    close_socket();

    printf("SiPM0_7 temperature is %.2f degrees Celsius\n", *temp);
    printf("SiPM8_15 temperature is %.2f degrees Celsius\n", *(temp + 1));
    printf("SiPM16_23 temperature is %.2f degrees Celsius\n", *(temp + 2));
    printf("SiPM24_31 temperature is %.2f degrees Celsius\n", *(temp + 3));

    return true;
}

bool FEEControl::logic_select(int select_data)
{
    if (!start_socket())
    {
        return false;
    }
    if (!send_cmd(up_logicSelect, (char *)(&select_data), sizeof(int) + 1))
    {
        return false;
    }
    close_socket();
    fLogicSelceted = select_data;
    return true;
}

bool FEEControl::send_ch_masks(uint32_t mask_num)
{
    if (!start_socket())
    {
        return false;
    }
    if (!send_cmd(up_channelMask, (char *)(&mask_num), sizeof(uint32_t) + 1))
    {
        return false;
    }
    close_socket();
    return true;
}

bool FEEControl::send_ch_probe(char channel)
{
    if (channel < 0 || channel > 31)
        return false;
    int reg_addr = 53, reg_wr_data = 1 << channel;
    if (!write_reg_test(reg_addr, reg_wr_data))
    {
        std::cout << "Error while setting signal probe." << std::endl;
        return false;
    }
    return true;
}

bool FEEControl::write_reg_test(int addr, int wr_data)
{
    return reg_write(up_writeRegTest, addr, wr_data);
}

bool FEEControl::read_reg_test(int addr, int &reg)
{
    return reg_read(up_readRegTest, addr, reg);
}

bool FEEControl::HV_config(void)
{
    char input_cmd[15] = {0};
    char HV_reply[50] = {0};
    printf("--------------------please input HV command!--------------------\n");
    printf(">>>>>>> input \"HPO\" to get status.\n");
    printf(">>>>>>> input \"HST 56.6\" to set the voltage to 56.6V at 30 degrees Celsius.\n");
    printf(">>>>>>> input \"HOF\" to turn off HV output.\n");
    printf(">>>>>>> input \"HON\" to turn on HV output.\n");
    printf(">>>>>>> input \"ext\" to exit HV set.\n");
    while (1)
    {
        printf(">>> your input cmd:");
        gets_s(input_cmd);
        if (!strcmp(input_cmd, "ext"))
        {
            break;
        }

        if (!start_socket())
        {
            return false;
        }

        if (!send_cmd(up_configC11204, input_cmd, (int)strlen(input_cmd) + 2))
        {
            return false;
        }

        // if (recv(fSock, HV_reply, 50, 0) <= 0)
        if (!RecvAll(HV_reply, 50))
        {
            cout << "receive error." << endl;
            close_socket();
            return false;
        }
        close_socket();

        char HV_cmd[4] = {0};
        strncpy(HV_cmd, input_cmd, 3);
        if (!strcmp(HV_cmd, "HPO") && strlen(HV_reply) == 12)
        {
            char OV_setting[5] = {0};
            strncpy(OV_setting, HV_reply, 4);
            char OV_monitor[5] = {0};
            strncpy(OV_monitor, HV_reply + 4, 4);
            char OC_monitor[5] = {0};
            strncpy(OC_monitor, HV_reply + 8, 4);
            double OV_set = (double)hexToDec(OV_setting) * 1.812 / 1000;
            double OV_moni = (double)hexToDec(OV_monitor) * 1.812 / 1000;
            double OC_moni = (double)hexToDec(OC_monitor) * 4.98 / 1000;

            printf("output voltage setting: %f\n", OV_set);
            printf("output voltage monitor: %f\n", OV_moni);
            printf("output current monitor: %f\n\n", OC_moni);
        }
        else
        {
            printf("%s\n\n", HV_reply);
        }
    }
    return true;
}

bool FEEControl::clean_queue(int queue_id)
{
    if (!start_socket())
    {
        return false;
    }
    if (!send_cmd(up_cleanQueue, (char *)(&queue_id), sizeof(int) + 1))
    {
        return false;
    }
    close_socket();
    return true;
}

bool FEEControl::length_read(cmd_up cmd, int &len)
{
    if (!start_socket())
    {
        return false;
    }
    if (!send_cmd(cmd, NULL, 1))
    {
        return false;
    }
    if (!recv_data((char *)(&len), sizeof(int)))
    {
        return false;
    }
    close_socket();
    return true;
}

bool FEEControl::data_read(cmd_up cmd, int data_num, uint32_t *data_addr)
{
    // auto test1 = clock();
    if (!start_socket())
    {
        return false;
    }
    // auto test2 = clock();

    if (!send_cmd(cmd, (char *)(&data_num), sizeof(int) + 1))
    {
        return false;
    }
    // auto test3 = clock();

    if (!recv_data((char *)data_addr, data_num * sizeof(uint8_t)))
    {
        return false;
    }
    // auto test4 = clock();

    close_socket();
    // auto test5 = clock();
    // std::cout << test2 - test1 << '\t' << test3 - test2 << '\t' << test4 - test3 << '\t' << test5 - test4 << std::endl;
    // std::cout << "Package size: " << (double)data_num / 1024.0 / 1024.0 << "MB" << std::endl;
    // if(test4-test3)
    // std::cout << "Network speed(Mb/s): " << data_num / (test4 - test3) / 1024 * 8 << std::endl;
    // else
    //     std::cout << "Network speed(Mb/s): " << 0 << std::endl;

    return true;
}

bool FEEControl::reg_write(cmd_up cmd, int addr, int wr_data)
{
    if (!start_socket())
    {
        return false;
    }
    int arg[2];
    arg[0] = addr;
    arg[1] = wr_data;
    if (!send_cmd(cmd, (char *)arg, sizeof(int) + sizeof(int) + 1))
    {
        return false;
    }
    close_socket();
    return true;
}

bool FEEControl::reg_read(cmd_up cmd, int addr, int &reg)
{
    if (!start_socket())
    {
        return false;
    }
    if (!send_cmd(cmd, (char *)(&addr), sizeof(addr) + 1))
    {
        return false;
    }
    if (!recv_data((char *)(&reg), sizeof(int)))
    {
        return false;
    }
    close_socket();
    return true;
}

bool FEEControl::send_cmd(cmd_up input_cmd, char *arg, int size)
{
    // when arg is NULL, size must be 1
    InitCMD(size);
    memset(cmd, '\0', size);
    *cmd = (char)input_cmd;

    if (arg != NULL)
    {
        // for (int i = 1; i < size; i++)
        // {
        //     *(cmd + i) = *(arg + i - 1);
        // }

        //! TODO: Test this block

        memcpy(cmd + 1, arg, size - 1);
    }
    if (!SendAll(cmd, size))
    {
        cout << "send error." << endl;
        close_socket();
        return false;
    }
    return true;
}

bool FEEControl::recv_data(char *buffer, int size)
{
    if (!RecvAll(buffer, size))
    {
        cout << "receive error." << endl;
        close_socket();
        return false;
    }
    return true;
}

bool FEEControl::SendAll(char *buffer, int size)
{
    if (!fConnectionFlag)
        return false;
    while (size > 0)
    {
        int SendSize = send(fSock, buffer, size, 0);
        if (SendSize == SOCKET_ERROR)
        {
#ifdef USE_FEE_CONTROL_MONITOR
            gFEEMonitor->ProcessConnectionBroken(fBoardNum);
#endif
            fConnectionFlag = false;
            return false;
        }
        size = size - SendSize;
        buffer += SendSize;
    }
    return true;
}

bool FEEControl::RecvAll(char *buffer, int size)
{
    if (!fConnectionFlag)
        return false;
    while (size > 0)
    {
        // auto test3 = clock();
        int RecvSize = recv(fSock, buffer, size, 0);
        // auto test4 = clock();

        // std::cout << "Package size: " << (double)RecvSize / 1024.0 / 1024.0 << "MB" << std::endl;
        // if (test4 - test3)
        //     std::cout << "Network speed(Mb/s): " << RecvSize / (test4 - test3) / 1024 * 8 << std::endl;
        // else
        //     std::cout << "Network speed(Mb/s): " << 0 << std::endl;
        if (RecvSize == SOCKET_ERROR)
        {
#ifdef USE_FEE_CONTROL_MONITOR
            gFEEMonitor->ProcessConnectionBroken(fBoardNum);
#endif
            fConnectionFlag = false;
            return false;
        }
        size = size - RecvSize;
        buffer += RecvSize;
    }
    return true;
}

bool FEEControl::start_socket()
{
    mutex.lock();
    if (WSAStartup(MAKEWORD(2, 2), &fWsaData) != 0)
    {
        fSockInitFlag = false;
        cout << "Startup error." << endl;
        return false;
    }

    // create socket
    fSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fSock == INVALID_SOCKET)
    {
        fSockInitFlag = false;
        cout << "Socket error." << endl;
        return false;
    }

    /*
    Add TCP protocol control
    */
    // set socket buffer length
    int opt = 1;
    int SendBufSize = 7944192;
    int RecvBufSize = 7944192;
    // int getSendBuf, getRecvBuf;
    // socklen_t sendBufSizeLen = sizeof(getSendBuf);
    // socklen_t RecvBufSizeLen = sizeof(getRecvBuf);
    // getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&getSendBuf, &sendBufSizeLen);
    // getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&getRecvBuf, &RecvBufSizeLen);
    // printf("send buffer size: %d, recv buffer size: %d\n", getSendBuf, getRecvBuf);

    setsockopt(fSock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    setsockopt(fSock, SOL_SOCKET, SO_SNDBUF, (char *)&SendBufSize, sizeof(int));
    setsockopt(fSock, SOL_SOCKET, SO_RCVBUF, (char *)&RecvBufSize, sizeof(int));

    // getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&getSendBuf, &sendBufSizeLen);
    // getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&getRecvBuf, &RecvBufSizeLen);
    // printf("send buffer size: %d, recv buffer size: %d\n", getSendBuf, getRecvBuf);

    // Connect to the server
    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(ip_address.c_str());
    sockAddr.sin_port = htons(fPort);
    if (connect(fSock, (SOCKADDR *)&sockAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        cout << "Connect error." << endl;
        fSockInitFlag = 0;
        close_socket();
#ifdef USE_FEE_CONTROL_MONITOR
        gFEEMonitor->ProcessConnectionBroken(fBoardNum);
#endif
        fConnectionFlag = false;
        return false;
    }
    fSockInitFlag = true;
    return fSockInitFlag;
}

void FEEControl::close_socket()
{
    closesocket(fSock);
    WSACleanup();
    mutex.unlock();
}

void FEEControl::str_process(char *in_str, char *out_str)
{
    int i = 0, j = 0;
    while (*(in_str + i) != '%')
    {
        if (*(in_str + i) != ' ')
        {
            *(out_str + j) = *(in_str + i);
            j++;
        }
        i++;
    }
    *(out_str + j) = '\0';
}

int FEEControl::hexToDec(char *source)
{
    int sum = 0;
    int t = 1;

    int len = (int)strlen(source);
    for (int i = len - 1; i >= 0; i--)
    {
        sum += t * getIndexOfSigns(*(source + i));
        t *= 16;
    }
    return sum;
}

int FEEControl::getIndexOfSigns(char ch)
{
    if (ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F')
    {
        return ch - 'A' + 10;
    }
    if (ch >= 'a' && ch <= 'f')
    {
        return ch - 'a' + 10;
    }
    return -1;
}

// bool FEEControl::SendAll(SOCKET sock, char *buffer, int size)
// {
//     while (size > 0)
//     {
//         int SendSize = send(sock, buffer, size, 0);
//         if (SendSize == SOCKET_ERROR)
//             return false;
//         size = size - SendSize;
//         buffer += SendSize;
//     }
//     return true;
// }

// bool FEEControl::RecvAll(SOCKET sock, char *buffer, int size)
// {
//     while (size > 0)
//     {
//         int RecvSize = recv(sock, buffer, size, 0);
//         if (RecvSize == SOCKET_ERROR)
//             return false;
//         size = size - RecvSize;
//         buffer += RecvSize;
//     }
//     return true;
// }

int FEEControl::HVSend(string scmd)
{
    char input_cmd[256];
    strcpy(input_cmd, scmd.c_str());

    if (!start_socket())
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = (int)(sizeof(char) + strlen(input_cmd) + 1);
    InitCMD(byte_num);
    *cmd = up_configC11204;
    strcat(cmd, input_cmd);
    // if (send(fSock, cmd, byte_num, 0) <= 0)
    if (!SendAll(cmd, byte_num))
    {
        cout << "send error." << endl;
        close_socket();
        return EXIT_FAILURE;
    }

    // if (recv(fSock, reply, 50, 0) <= 0)
    if (!RecvAll(reply, 50))
    {
        cout << "receive error." << endl;
        close_socket();
        return EXIT_FAILURE;
    }

    // close socket
    close_socket();
    return EXIT_SUCCESS;
}

int FEEControl::HVOFF()
{
    return HVSend("HOF");
}

int FEEControl::HVON()
{
    return HVSend("HON");
}

int FEEControl::HVSet(double setT)
{
    char hvBuf[10];
    sprintf(hvBuf, "%.2f", setT);
    return HVSend((string) "HST" + hvBuf);
}

int FEEControl::HVMonitor()
{
    auto rtn = HVSend("HPO");
    if (rtn == EXIT_FAILURE)
        return rtn;
    if (strlen(reply) == 12)
    {
        hv = HVStatus((string)reply);
        hv.Print();
    }
    else
    {
        printf("Warning: HPO reply: %s\n", reply);
        hv.Print();
    }
    return rtn;
}

void FEEControl::HVPrint()
{
    hv.Print();
}

char *FEEControl::InitCMD(int length)
{
    if (length <= MUON_TEST_CONTROL_SOCKET_MAX_LOAD_LENGTH)
    {
        memset(cmd, '\0', length);
    }
    else
    {
        memset(cmd, '\0', MUON_TEST_CONTROL_SOCKET_MAX_LOAD_LENGTH);
    }
    return cmd;
};

// int FEEControl::InitSock()
// {
//     if (WSAStartup(MAKEWORD(2, 2), &fWsaData) != 0)
//     {
//         fSockInitFlag = EXIT_FAILURE;
//         cout << "Startup error." << endl;
//         return EXIT_FAILURE;
//     }

//     // create socket
//     fSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//     if (fSock == INVALID_SOCKET)
//     {
//         fSockInitFlag = EXIT_FAILURE;
//         cout << "Socket error." << endl;
//         return EXIT_FAILURE;
//     }

//     // Connect to the server
//     sockaddr_in sockAddr;
//     memset(&sockAddr, 0, sizeof(sockAddr));
//     sockAddr.sin_family = AF_INET;
//     sockAddr.sin_addr.s_addr = inet_addr(ip_address.c_str());
//     sockAddr.sin_port = htons(fPort);
//     if (connect(fSock, (SOCKADDR *)&sockAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
//     {
//         cout << "Connect error." << endl;
//         fSockInitFlag = 0;
//         close_socket();
//         return EXIT_FAILURE;
//     }
//     fSockInitFlag = EXIT_SUCCESS;
//     return fSockInitFlag;
// }

// void FEEControl::CloseSock()
// {
//     // close socket
//     closesocket(fSock);
//     WSACleanup();
// }

bool FEEControl::TestConnect()
{
    fConnectionFlag = true;
    // reg test
    int reg_test, reg_addr_test = 59, wr_data_test = 0xf;
    wr_data_test = 0x1;
    if (!write_reg_test(reg_addr_test, wr_data_test))
    {
        cout << "AD9635 write failed." << endl;
        return 0;
    }
    auto rtn = read_reg_test(reg_addr_test, reg_test);
    if (!rtn)
    {
        fConnectionFlag = false;
        return false;
    }
    if (reg_test == wr_data_test)
        fConnectionFlag = true;
    else
        fConnectionFlag = false;
    return fConnectionFlag;
}

double FEEControl::ReadFreq()
{
    // si570 read and set
    double freq;
    si570_get(freq);
    printf("\nThe output clock frequency of the Si570 is %.4f.\n", freq);
    // if (!si570_set(320))
    // {
    //     cout << "Si570 set failed." << endl;
    //     return -1;
    // }
    // si570_get(freq);
    // printf("The output clock frequency of the Si570 is %.4f.\n\n", freq);
    return freq;
}

int FEEControl::BoardCheck()
{
    // //ad9635 write and read
    int reg, reg_addr = 0x19, wr_data = 0xff;
    ad9645_reg_read(reg_addr, reg);
    printf("\nThe value of AD9645 reg %02x is %02x.\n", reg_addr, reg);
    if (!ad9645_reg_write(reg_addr, wr_data))
    {
        cout << "AD9645 write failed." << endl;
        return -1;
    }

    return 1;
}

bool FEEControl::ReadTemp()
{
    // sipm temperature read
    if (!sipm_temp_read(fTemp))
    {
        cout << "sipm temp read failed." << endl;
        return 0;
    }
    return 1;
}

int FEEControl::TestReg()
{
    // reg test
    int reg_test, reg_addr_test = 59, wr_data_test = 0xf;
    read_reg_test(reg_addr_test, reg_test);
    printf("\nThe value of test reg %2d is %02x.\n", reg_addr_test, reg_test);
    if (!write_reg_test(reg_addr_test, wr_data_test))
    {
        cout << "AD9635 write failed." << endl;
        return -1;
    }

    return reg_test;
}

bool FEEControl::get_real_counter(uint32_t &count)
{
    static const int realAddr = 6;
    int result = 0;
    auto rtn = read_reg_test(realAddr, result);
    count = (uint32_t)result;
    return rtn;
}

bool FEEControl::get_live_counter(uint32_t &count)
{
    static const int liveAddr = 5;
    int result = 0;
    auto rtn = read_reg_test(liveAddr, result);
    count = (uint32_t)result;
    return rtn;
}

bool FEEControl::BoardExit()
{
    // exit
    if (!server_exit())
    {
        cout << "exit failed." << endl;
        return 0;
    }
    return 1;
}

#include <QtConcurrent/QtConcurrent>
#include <General.h>
bool FEEControl::ReadFifo(int sleepms, int leastNEvents)
{
    // test queue read
    int maxEvents = (int)(0.9 * fMaxSaveEvents);
    int nEvents = leastNEvents > (maxEvents) ? maxEvents : leastNEvents;

    fHGQueueLengthMonitor = 0;
    fLGQueueLengthMonitor = 0;
    fTDCQueueLengthMonitor = 0;
    fReadGroupMonitor = 0;

    // Deside when to start read queue, break flag means force to stop loop
    while (!fBreakFlag)
    {
        if (!hg_queue_length_read(fHGQueueLengthMonitor))
            break;
        if (!lg_queue_length_read(fLGQueueLengthMonitor))
            break;
        if (!tdc_queue_length_read(fTDCQueueLengthMonitor))
            break;
        // std::cout << "hg length: " << fHGQueueLengthMonitor << std::endl;
        if (fHGQueueLengthMonitor > nEvents * fHGPointFactor)
            break;

        Sleep(sleepms);
    }
    fBreakFlag = 0; // Set break flag to zero, regardless of whether is break.

    // Judge read length
    hg_queue_length_read(fHGQueueLengthMonitor);
    lg_queue_length_read(fLGQueueLengthMonitor);
    tdc_queue_length_read(fTDCQueueLengthMonitor);
    fReadGroupMonitor = fHGQueueLengthMonitor / (fHGPointFactor * fRetrieveUnit);

    // Compare queue length with save array length, take smaller one as read length
    //! TODO: Change compared length. Length means read Nbytes from queue
    int nReadGroup = (fHGQueueLengthMonitor > maxEvents * fHGPointFactor) ? maxEvents * fHGPointFactor / (fHGPointFactor * fRetrieveUnit) : fHGQueueLengthMonitor / (fHGPointFactor * fRetrieveUnit);

    fHGDataLength = nReadGroup * (fHGPointFactor * fRetrieveUnit);
    fLGDataLength = nReadGroup * (fLGPointFactor * fRetrieveUnit);
    fTDCDataLength = nReadGroup * (fTDCPointFactor * fRetrieveUnit);

    // fHGDataLength = (fHGQueueLengthMonitor > maxEvents * fHGPointFactor) ? maxEvents * fHGPointFactor : fHGQueueLengthMonitor / (fHGPointFactor * fRetrieveUnit) * (fHGPointFactor * fRetrieveUnit);
    // fLGDataLength = (fLGQueueLengthMonitor > maxEvents * fLGPointFactor) ? maxEvents * fLGPointFactor : fLGQueueLengthMonitor / (fLGPointFactor * fRetrieveUnit) * (fLGPointFactor * fRetrieveUnit);
    // fTDCDataLength = (fTDCQueueLengthMonitor > maxEvents * fTDCPointFactor) ? maxEvents * fTDCPointFactor : fTDCQueueLengthMonitor / (fTDCPointFactor * fRetrieveUnit) * (fTDCPointFactor * fRetrieveUnit);
    // std::cout << fHGDataLength / (fHGPointFactor * fRetrieveUnit) << '\t' << fLGDataLength / (fLGPointFactor * fRetrieveUnit) << '\t' << fTDCDataLength / (fTDCPointFactor * fRetrieveUnit) << std::endl;

    // fHGDataLength = (fHGPointFactor * fRetrieveUnit);
    // fLGDataLength = (fLGPointFactor * fRetrieveUnit);
    // fTDCDataLength = (fTDCPointFactor * fRetrieveUnit);

    // auto test1 = clock();
    if (!hg_data_read(fHGDataLength, fHGQueueData))
    {
        cout << "HG queue read failed." << endl;
        fDataFlag = 0;
        fHGDataLength = 0;
        return false;
    }

    // std::cout <<"HG Queue Data Test: " << std::endl;
    // for(int i = 0; i < fHGDataLength; i++)
    // {
    //     std::cout << fHGQueueData[i] << '\t';
    // }
    // std::cout << std::endl;
    // auto test2 = clock();
    // std::cout << "test2 - test1: " << test2 - test1 << '\t' << fHGDataLength << '\t' << (double)fHGDataLength / (test2 - test1) << std::endl;

    if (!lg_data_read(fLGDataLength, fLGQueueData))
    {
        cout << "LG queue read failed." << endl;
        fDataFlag = 0;
        fLGDataLength = 0;
        return false;
    }
    // auto test3 = clock();
    // std::cout << "test3 - test2: " << test3 - test2 << '\t' << fLGDataLength << '\t' << (double)fLGDataLength / (test3 - test2) << std::endl;

    if (!tdc_data_read(fTDCDataLength, fTDCQueueData))
    {
        cout << "TDC queue read failed." << endl;
        fDataFlag = 0;
        fTDCDataLength = 0;
        return false;
    }
    // auto test4 = clock();
    // std::cout << "test4 - test3: " << test4 - test3 << '\t' << fTDCDataLength << '\t' << (double)fTDCDataLength / (test4 - test3) << std::endl;

    // Convert uint32_t to uint16_t
    QFuture<void> futureHG = QtConcurrent::run(UserDefine::ConvertUInt32ToUInt16s, fHGQueueData, fHGDataLength / 4, fHGQueueDataU16, &fHGDataLengthPoints);
    QFuture<void> futureLG = QtConcurrent::run(UserDefine::ConvertUInt32ToUInt16s, fLGQueueData, fLGDataLength / 4, fLGQueueDataU16, &fLGDataLengthPoints);
    QFuture<void> futureTDC = QtConcurrent::run(UserDefine::ConvertUInt32ToUInt16s, fTDCQueueData, fTDCDataLength / 4, fTDCQueueDataU16, &fTDCDataLengthPoints);
    futureHG.waitForFinished();
    futureLG.waitForFinished();
    futureTDC.waitForFinished();

    // auto test5 = clock();
    // std::cout << "test5 - test4: " << test5 - test4 << std::endl;

    fifoReadCount++;
    fDataFlag = 1;
    return true;
}

int FEEControl::SendConfig(ConfigFileParser *parser)
{
    if (!parser->sConfigValidate())
        return -1;

    if (!start_socket())
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = (int)((parser->GetString().size() + 2) * sizeof(char));
    InitCMD(byte_num);
    memset(cmd, '\0', byte_num);
    *cmd = up_configCitiroc;
    strcat(cmd, parser->GetString().c_str());
    if (!SendAll(cmd, byte_num))
    {
        cout << "send error." << endl;
        close_socket();
        return EXIT_FAILURE;
    }

    // close socket
    close_socket();

    return EXIT_SUCCESS;
}

uint32_t &FEEControl::GenerateChMask(int ch, bool flag, uint32_t &reg)
{
    uint32_t flag_now = GetMask(ch, reg);
    reg += (int)(flag - flag_now) * (1 << ch);
    return reg;
}

uint32_t FEEControl::GenerateMasks(bool *flag)
{
    uint32_t reg;
    for (int i = 0; i < 32; i++)
    {
        GenerateChMask(i, flag[i], reg);
    }
    return reg;
}

bool FEEControl::GetMask(int ch, uint32_t reg)
{
    return (reg >> ch) & 1;
}

bool FEEControl::hg_fifo_read(int read_num, int loop_times, const char *hg_file_name)
{
    int i, j, queue_num, fifo_num;
    for (i = 0; i < loop_times; i++)
    {
        if (!hg_queue_length_read(queue_num))
        {
            cout << "hg queue length read failed." << endl;
            return false;
        }
        while (queue_num < read_num)
        {
            if (!hg_queue_length_read(queue_num))
            {
                cout << "hg queue length read failed." << endl;
                return false;
            }
            if (!hg_fifo_length_read(fifo_num))
            {
                cout << "hg fifo length read failed." << endl;
                return false;
            }
            printf("hg_queue_num:%5d, hg_fifo_num:%3d, loop times:%3d\n", queue_num, fifo_num, i);
        }

        if (!hg_data_read(read_num, fHGQueueData))
        {
            cout << "hg data read failed." << endl;
            return false;
        }

        FILE *fp;
        j = 0;
        if (!i)
        {
            if ((fp = fopen(hg_file_name, "w")) == NULL)
            {
                printf("cannot open hg_data.dat");
                return false;
            }
        }
        else
        {
            if ((fp = fopen(hg_file_name, "a")) == NULL)
            {
                printf("cannot open this hg_data.dat");
                return false;
            }
        }
        while (j < read_num)
        {
            // fprintf(fp,"%f\n",queue_data[j]/32.768-1000);
            fprintf(fp, "%d\n", fHGQueueData[j]);
            j++;
        }
        fclose(fp);
    }
    return true;
}

bool FEEControl::lg_fifo_read(int read_num, int loop_times, const char *lg_file_name)
{
    int i, j, queue_num, fifo_num;
    for (i = 0; i < loop_times; i++)
    {
        if (!lg_queue_length_read(queue_num))
        {
            cout << "lg queue length read failed." << endl;
            return false;
        }
        while (queue_num < read_num)
        {
            if (!lg_queue_length_read(queue_num))
            {
                cout << "lg queue length read failed." << endl;
                return false;
            }
            if (!lg_fifo_length_read(fifo_num))
            {
                cout << "lg fifo length read failed." << endl;
                return false;
            }
            printf("lg_queue_num:%5d, lg_fifo_num:%3d, loop times:%3d\n", queue_num, fifo_num, i);
        }

        if (!lg_data_read(read_num, fLGQueueData))
        {
            cout << "lg data read failed." << endl;
            return false;
        }

        FILE *fp;
        j = 0;
        if (!i)
        {
            if ((fp = fopen(lg_file_name, "w")) == NULL)
            {
                printf("cannot open lg_data.dat");
                return false;
            }
        }
        else
        {
            if ((fp = fopen(lg_file_name, "a")) == NULL)
            {
                printf("cannot open this lg_data.dat");
                return false;
            }
        }
        while (j < read_num)
        {
            // fprintf(fp,"%f\n",queue_data[j]/32.768-1000);
            fprintf(fp, "%d\n", fLGQueueData[j]);
            j++;
        }
        fclose(fp);
    }
    return true;
}

bool FEEControl::test_fifo_read(int read_num, int loop_times, const char *test_file_name)
{
    int i, j, queue_num, fifo_num;
    for (i = 0; i < loop_times; i++)
    {
        if (!test_queue_length_read(queue_num))
        {
            cout << "test queue length read failed." << endl;
            return false;
        }
        while (queue_num < read_num)
        {
            if (!test_queue_length_read(queue_num))
            {
                cout << "test queue length read failed." << endl;
                return false;
            }
            if (!test_fifo_length_read(fifo_num))
            {
                cout << "test fifo length read failed." << endl;
                return false;
            }
            printf("test_queue_num:%5d, test_fifo_num:%3d, loop times:%3d\n", queue_num, fifo_num, i);
        }

        if (!test_data_read(read_num, fTestQueueData))
        {
            cout << "test data read failed." << endl;
            return false;
        }

        FILE *fp;
        j = 0;
        if (!i)
        {
            if ((fp = fopen(test_file_name, "w")) == NULL)
            {
                printf("cannot open test_data.dat");
                return false;
            }
        }
        else
        {
            if ((fp = fopen(test_file_name, "a")) == NULL)
            {
                printf("cannot open this test_data.dat");
                return false;
            }
        }
        while (j < read_num)
        {
            // fprintf(fp,"%f\n",queue_data[j]/32.768-1000);
            fprintf(fp, "%d\n", fTestQueueData[j]);
            j++;
        }
        fclose(fp);
    }
    return true;
}

bool FEEControl::tdc_fifo_read(int read_num, int loop_times, const char *tdc_file_name)
{
    int i, j, queue_num, fifo_num;
    for (i = 0; i < loop_times; i++)
    {
        if (!tdc_queue_length_read(queue_num))
        {
            cout << "tdc queue length read failed." << endl;
            return false;
        }
        while (queue_num < read_num)
        {
            if (!tdc_queue_length_read(queue_num))
            {
                cout << "tdc queue length read failed." << endl;
                return false;
            }
            if (!tdc_fifo_length_read(fifo_num))
            {
                cout << "tdc fifo length read failed." << endl;
                return false;
            }
            printf("tdc_queue_num:%5d, tdc_fifo_num:%3d, loop times:%3d\n", queue_num, fifo_num, i);
        }

        if (!tdc_data_read(read_num, fTDCQueueData))
        {
            cout << "tdc data read failed." << endl;
            return false;
        }

        FILE *fp;
        j = 0;
        if (!i)
        {
            if ((fp = fopen(tdc_file_name, "w")) == NULL)
            {
                printf("cannot open tdc_data.dat");
                return false;
            }
        }
        else
        {
            if ((fp = fopen(tdc_file_name, "a")) == NULL)
            {
                printf("cannot open this tdc_data.dat");
                return false;
            }
        }
        while (j < read_num)
        {
            // fprintf(fp,"%f\n",queue_data[j]/32.768-1000);
            fprintf(fp, "%d\n", fTDCQueueData[j]);
            j++;
        }
        fclose(fp);
    }
    return true;
}

// FEEControl *gBoard = new FEEControl;

HVStatus::HVStatus(string s)
{
    if (s.size() != 12)
    {
        OV_set = 0;
        OV_moni = 0;
        OC_moni = 0;
    }

    char OV_setting[5] = {0};
    strncpy(OV_setting, s.data(), 4);
    char OV_monitor[5] = {0};
    strncpy(OV_monitor, s.data() + 4, 4);
    char OC_monitor[5] = {0};
    strncpy(OC_monitor, s.data() + 8, 4);

    OV_set = (double)FEEControl::hexToDec(OV_setting) * 1.812 / 1000;
    OV_moni = (double)FEEControl::hexToDec(OV_monitor) * 1.812 / 1000;
    OC_moni = (double)FEEControl::hexToDec(OC_monitor) * 4.98 / 1000;
}

void HVStatus::Print()
{
    printf("output voltage setting: %f\n", OV_set);
    printf("output voltage monitor: %f\n", OV_moni);
    printf("output current monitor: %f\n\n", OC_moni);
}

#ifdef USE_FEE_CONTROL_MONITOR
QtUserConnectionMonitor *QtUserConnectionMonitor::Instance()
{
    static QtUserConnectionMonitor *instance = new QtUserConnectionMonitor;
    return instance;
}
#endif
