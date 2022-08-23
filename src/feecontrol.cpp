#include "../include/feecontrol.h"
#include <WinSock2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "configfileparser.h"

WSADATA fWsaData; // Unused unimportant data, used to put inside class FEEControl, but win sock2.h is like a piece of shit, which is conflict with <TTree.h>, <zaber>

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

#define SIZE_STR 150

static void str_process(char *in_str, char *out_str);

static bool SendAll(SOCKET sock, char *buffer, int size);

static bool RecvAll(SOCKET sock, char *buffer, int size);

static int hexToDec(char *source);

static int getIndexOfSigns(char ch);

FEEControl::FEEControl(std::string ip, int port) : ip_address(ip), fPort(port)
{
    fifoData = new uint32_t[MUON_TEST_CONTROL_FIFO_BUFFER_LENGTH];
}

FEEControl::~FEEControl()
{
    delete[] fifoData;
    fifoData = NULL;
}

void FEEControl::InitPort(std::string ip, int port)
{
    ip_address = ip;
    fPort = port;
}

int FEEControl::server_exit()
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send an receive data
    int byte_num = sizeof(char);
    InitCMD(byte_num);
    *cmd = cmd_exit;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return EXIT_SUCCESS;
}

#include <fstream>
#include <QDebug>

int FEEControl::citiroc1a_configure(string sc_file_name, string probe_file_name)
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    FILE *fp_sc, *fp_probe;
    char str_get[SIZE_STR] = {0};
    char str_temp[SIZE_STR]{0};
    char str_out[1500] = "11111111";

    if ((fp_sc = fopen(sc_file_name.c_str(), "r")) == NULL)
    {
        printf("can\'t open file!\n");
        exit(0);
    }
    if ((fp_probe = fopen(probe_file_name.c_str(), "r")) == NULL)
    {
        printf("can\'t open file!\n");
        exit(0);
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

    int byte_num = (int)((strlen(str_out) + 2) * sizeof(char));
    InitCMD(byte_num);
    memset(cmd, '\0', byte_num);
    *cmd = cmd_configCitiroc;
    strcat(cmd, str_out);
    if (!SendAll(fSock, cmd, byte_num))
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return EXIT_SUCCESS;
}

int FEEControl::ad9635_reg_write(const int dev_addr, int wr_data)
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = (int)(sizeof(char) + 2 * sizeof(int));
    InitCMD(byte_num);
    *cmd = cmd_writeAD9635;
    int *arg = (int *)(cmd + 1);
    *arg = dev_addr;
    *(arg + 1) = wr_data;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return EXIT_SUCCESS;
}

int FEEControl::ad9635_reg_read(const int dev_addr)
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = (int)(sizeof(char) + sizeof(int));
    InitCMD(byte_num);
    *cmd = cmd_readAD9635;
    int *arg = (int *)(cmd + 1);
    *arg = dev_addr;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    int reg;
    if (recv(fSock, (char *)(&reg), sizeof(int), 0) <= 0)
    {
        cout << "receive error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return reg;
}

int FEEControl::logic_select(int select_data)
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = sizeof(char) + sizeof(int);
    InitCMD(byte_num);
    *cmd = cmd_logicSelect;
    int *arg = (int *)(cmd + 1);
    *arg = select_data;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return EXIT_SUCCESS;
}

int FEEControl::send_ch_masks(uint32_t mask)
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = sizeof(char) + sizeof(uint32_t);
    InitCMD(byte_num);
    *cmd = cmd_channelMask;
    uint32_t *arg = (uint32_t *)(cmd + 1);
    *arg = mask;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return EXIT_SUCCESS;
}

int FEEControl::si570_set(double freq)
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = (int)(sizeof(char) + sizeof(double));
    InitCMD(byte_num);
    *cmd = cmd_setSi570Freq;
    double *arg = (double *)(cmd + 1);
    *arg = freq;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return EXIT_SUCCESS;
}

double FEEControl::si570_get()
{

    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = (int)sizeof(char);
    InitCMD(byte_num);
    *cmd = cmd_getSi570Freq;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    double freq;
    if (recv(fSock, (char *)(&freq), sizeof(double), 0) <= 0)
    {
        cout << "receive error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return freq;
}

int FEEControl::lg_fifo_length_read()
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = sizeof(char);
    InitCMD(byte_num);
    *cmd = cmd_getLgFifoLen;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    int len;
    if (recv(fSock, (char *)(&len), sizeof(int), 0) <= 0)
    {
        cout << "receive error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return len;
}

int FEEControl::lg_fifo_read(int data_num, uint32_t *data_addr)
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = sizeof(char) + sizeof(int);
    InitCMD(byte_num);
    *cmd = cmd_getLgFifoData;
    int *arg = (int *)(cmd + 1);
    *arg = data_num;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    if (!RecvAll(fSock, (char *)data_addr, data_num * sizeof(uint32_t)))
    {
        cout << "receive error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return EXIT_SUCCESS;
}

int FEEControl::hg_fifo_length_read()
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = sizeof(char);
    InitCMD(byte_num);
    *cmd = cmd_getHgFifoLen;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    int len;
    if (recv(fSock, (char *)(&len), sizeof(int), 0) <= 0)
    {
        cout << "receive error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return len;
}

int FEEControl::hg_fifo_read(int data_num, uint32_t *data_addr)
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = sizeof(char) + sizeof(int);
    InitCMD(byte_num);
    *cmd = cmd_getHgFifoData;
    int *arg = (int *)(cmd + 1);
    *arg = data_num;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    if (!RecvAll(fSock, (char *)data_addr, data_num * sizeof(uint32_t)))
    {
        cout << "receive error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return EXIT_SUCCESS;
}

int FEEControl::test_fifo_length_read()
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = sizeof(char);
    InitCMD(byte_num);
    *cmd = cmd_getTestFifoLen;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    int len;
    if (recv(fSock, (char *)(&len), sizeof(int), 0) <= 0)
    {
        cout << "receive error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return len;
}

int FEEControl::test_fifo_read(int data_num, uint32_t *data_addr)
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = sizeof(char) + sizeof(int);
    InitCMD(byte_num);
    *cmd = cmd_getTestFifoData;
    int *arg = (int *)(cmd + 1);
    *arg = data_num;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    if (!RecvAll(fSock, (char *)data_addr, data_num * sizeof(uint32_t)))
    {
        cout << "receive error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return EXIT_SUCCESS;
}

int FEEControl::sipm_temp_read(double *temp)
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = (int)sizeof(char);
    InitCMD(byte_num);
    *cmd = cmd_getSimpTemp;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    int num = 4;
    if (recv(fSock, (char *)temp, num * sizeof(double), 0) <= 0)
    {
        cout << "receive error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    printf("SiPM0_7 temperature is %.2f degrees Celsius\n", *temp);
    printf("SiPM8_15 temperature is %.2f degrees Celsius\n", *(temp + 1));
    printf("SiPM16_23 temperature is %.2f degrees Celsius\n", *(temp + 2));
    printf("SiPM24_31 temperature is %.2f degrees Celsius\n", *(temp + 3));

    // close socket
    CloseSock();

    return EXIT_SUCCESS;
}

int FEEControl::read_reg_test(int reg_num)
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = (int)(sizeof(char) + sizeof(int));
    InitCMD(byte_num);
    *cmd = cmd_readRegTest;
    int *arg = (int *)(cmd + 1);
    *arg = reg_num;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    int reg;
    if (recv(fSock, (char *)(&reg), sizeof(int), 0) <= 0)
    {
        cout << "receive error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return reg;
}

int FEEControl::write_reg_test(int reg_num, int wr_data)
{
    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = sizeof(char) + 2 * sizeof(int);
    InitCMD(byte_num);
    *cmd = cmd_writeRegTest;
    int *arg = (int *)(cmd + 1);
    *arg = reg_num;
    *(arg + 1) = wr_data;
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return EXIT_SUCCESS;
}

int FEEControl::HV_config(void)
{
    char input_cmd[15] = {0};
    printf("--------------------please input HV command!--------------------\n");
    printf(">>>>>>> input \"HPO\" to get status.\n");
    printf(">>>>>>> input \"HST 56.6\" to set the voltage to 56.6V at 30 degrees Celsius.\n");
    printf(">>>>>>> input \"HOF\" to turn off HV output.\n");
    printf(">>>>>>> input \"HON\" to turn on HV output.\n");
    printf(">>>>>>> input \"ext\" to exit HV set.\n");
    while (strcmp(input_cmd, "ext"))
    {
        printf(">>> your input cmd:");
        gets_s(input_cmd);
        if (!strcmp(input_cmd, "ext"))
        {
            break;
        }
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            return EXIT_FAILURE;
        }

        // create socket
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
        {
            cout << "Socket error." << endl;
            return EXIT_FAILURE;
        }

        // Connect to the server
        sockaddr_in sockAddr;
        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = inet_addr(ip_address.c_str());
        sockAddr.sin_port = htons(fPort);
        if (connect(sock, (SOCKADDR *)&sockAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
        {
            cout << "Connect error." << endl;
            closesocket(sock);
            return EXIT_FAILURE;
        }

        // send and receive data
        int byte_num = (int)(sizeof(char) + strlen(input_cmd) + 1);
        char *cmd = (char *)malloc(byte_num);
        memset(cmd, '\0', byte_num);
        *cmd = cmd_configC11204;
        strcat(cmd, input_cmd);
        if (send(sock, cmd, byte_num, 0) <= 0)
        {
            cout << "send error." << endl;
            closesocket(sock);
            free(cmd);
            return EXIT_FAILURE;
        }
        free(cmd);

        char HV_reply[50] = {0};
        if (recv(sock, HV_reply, 50, 0) <= 0)
        {
            cout << "receive error." << endl;
            closesocket(sock);
            free(cmd);
            return EXIT_FAILURE;
        }

        // close socket
        closesocket(sock);
        WSACleanup();

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
    return EXIT_SUCCESS;
}

int FEEControl::HVSend(string scmd)
{
    char input_cmd[256];
    strcpy(input_cmd, scmd.c_str());

    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = (int)(sizeof(char) + strlen(input_cmd) + 1);
    InitCMD(byte_num);
    *cmd = cmd_configC11204;
    strcat(cmd, input_cmd);
    if (send(fSock, cmd, byte_num, 0) <= 0)
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    if (recv(fSock, reply, 50, 0) <= 0)
    {
        cout << "receive error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();
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

int FEEControl::InitSock()
{
    if (WSAStartup(MAKEWORD(2, 2), &fWsaData) != 0)
    {
        fSockInitFlag = EXIT_FAILURE;
        cout << "Startup error." << endl;
        return EXIT_FAILURE;
    }

    // create socket
    fSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fSock == INVALID_SOCKET)
    {
        fSockInitFlag = EXIT_FAILURE;
        cout << "Socket error." << endl;
        return EXIT_FAILURE;
    }

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
        closesocket(fSock);
        return EXIT_FAILURE;
    }
    fSockInitFlag = EXIT_SUCCESS;
    return fSockInitFlag;
}

void FEEControl::CloseSock()
{
    // close socket
    closesocket(fSock);
    WSACleanup();
}

bool FEEControl::TestConnect()
{
    // reg test
    int reg_test, reg_addr_test = 50, wr_data_test = 0xf;
    wr_data_test = 0x1;
    if (write_reg_test(reg_addr_test, wr_data_test) < 0)
    {
        cout << "AD9635 write failed." << endl;
        return 0;
    }
    reg_test = read_reg_test(reg_addr_test);

    return (reg_test == wr_data_test);
}

double FEEControl::ReadFreq()
{
    // si570 read and set
    double freq;
    freq = si570_get();
    printf("\nThe output clock frequency of the Si570 is %.4f.\n", freq);
    if (si570_set(320) < 0)
    {
        cout << "Si570 set failed." << endl;
        return -1;
    }
    freq = si570_get();
    printf("The output clock frequency of the Si570 is %.4f.\n\n", freq);
    return freq;
}

int FEEControl::BoardCheck()
{
    // //ad9635 write and read
    int reg, reg_addr = 0x19, wr_data = 0xff;
    reg = ad9635_reg_read(reg_addr);
    printf("\nThe value of AD9635 reg %02x is %02x.\n", reg_addr, reg);
    if (ad9635_reg_write(reg_addr, wr_data) < 0)
    {
        cout << "AD9635 write failed." << endl;
        return -1;
    }
    reg = ad9635_reg_read(reg_addr);
    printf("The value of AD9635 reg %02x is %02x.\n", reg_addr, reg);

    return 1;
}

bool FEEControl::ReadTemp()
{
    // sipm temperature read
    if (sipm_temp_read(fTemp) < 0)
    {
        cout << "sipm temp read failed." << endl;
        return 0;
    }
    return 1;
}

int FEEControl::TestReg()
{
    // reg test
    int reg_test, reg_addr_test = 50, wr_data_test = 0xf;
    reg_test = read_reg_test(reg_addr_test);
    printf("\nThe value of test reg %2d is %02x.\n", reg_addr_test, reg_test);
    if (write_reg_test(reg_addr_test, wr_data_test) < 0)
    {
        cout << "AD9635 write failed." << endl;
        return -1;
    }
    reg_test = read_reg_test(reg_addr_test);
    printf("The value of test reg %2d is %02x.\n", reg_addr_test, reg_test);

    return reg_test;
}

bool FEEControl::BoardExit()
{
    // exit
    if (server_exit() < 0)
    {
        cout << "exit failed." << endl;
        return 0;
    }
    return 1;
}

int FEEControl::ReadFifo(int sleepms)
{
    // test fifo read
    int read_num = MUON_TEST_CONTROL_FIFO_BUFFER_LENGTH; // the read_num must be Integer multiple of 5, and no more than MUON_TEST_CONTROL_FIFO_BUFFER_LENGTH

    int fifo_length = hg_fifo_length_read();
    while (2 * fifo_length < read_num && !fBreakFlag)
    {
        //        printf("Data in fifo is insufficient, waiting 2s. Fifo length: %d\n", 2 * fifo_length);
        //         return -1;
        fifo_length = hg_fifo_length_read();
        // cout << "There are not enough numbers in fifo. waiting 0.2s" << endl;
        // cout << "fifo_length: " << fifo_length << endl;
        // Sleep(1); // Sleep 1 ms
        Sleep(sleepms); // Sleep 1 ms
    }
    fBreakFlag = 0;

    if (hg_fifo_read(read_num, fifoData) < 0)
    {
        cout << "fifo read failed." << endl;
        fDataFlag = 0;
        fDataLength = 0;
        return -1;
    }
    fifoReadCount++;
    fDataFlag = 1;
    fDataLength = fifo_length;
    return fifo_length;
}

int FEEControl::SendConfig(ConfigFileParser *parser)
{
    if (!parser->sConfigValidate())
        return -1;

    if (InitSock() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    // send and receive data
    int byte_num = (int)((parser->GetString().size() + 2) * sizeof(char));
    InitCMD(byte_num);
    memset(cmd, '\0', byte_num);
    *cmd = cmd_configCitiroc;
    strcat(cmd, parser->GetString().c_str());
    if (!SendAll(fSock, cmd, byte_num))
    {
        cout << "send error." << endl;
        closesocket(fSock);
        return EXIT_FAILURE;
    }

    // close socket
    CloseSock();

    return EXIT_SUCCESS;
}

uint32_t &FEEControl::SetChannelMask(int ch, bool flag, uint32_t &reg)
{
    uint32_t flag_now = GetMask(ch, reg);
    reg += (int)(flag - flag_now) * (1 << ch);
    return reg;
}

uint32_t &FEEControl::SetMasks(bool *flag)
{
    uint32_t reg;
    for (int i = 0; i < 32; i++)
    {
        SetChannelMask(i, flag[i], reg);
    }
    return reg;
}

bool FEEControl::GetMask(int ch, uint32_t reg)
{
    return (reg >> ch) & 1;
}

FEEControl *gBoard = new FEEControl;

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

    OV_set = (double)hexToDec(OV_setting) * 1.812 / 1000;
    OV_moni = (double)hexToDec(OV_monitor) * 1.812 / 1000;
    OC_moni = (double)hexToDec(OC_monitor) * 4.98 / 1000;
}

void HVStatus::Print()
{
    printf("output voltage setting: %f\n", OV_set);
    printf("output voltage monitor: %f\n", OV_moni);
    printf("output current monitor: %f\n\n", OC_moni);
}

static void str_process(char *in_str, char *out_str)
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

static bool SendAll(SOCKET sock, char *buffer, int size)
{
    while (size > 0)
    {
        int SendSize = send(sock, buffer, size, 0);
        if (SendSize == SOCKET_ERROR)
            return false;
        size = size - SendSize;
        buffer += SendSize;
    }
    return true;
}

static bool RecvAll(SOCKET sock, char *buffer, int size)
{
    while (size > 0)
    {
        int RecvSize = recv(sock, buffer, size, 0);
        if (RecvSize == SOCKET_ERROR)
            return false;
        size = size - RecvSize;
        buffer += RecvSize;
    }
    return true;
}

static int hexToDec(char *source)
{
    int sum = 0;
    int t = 1;
    size_t len;

    len = strlen(source);
    for (int i = (int)len - 1; i >= 0; i--)
    {
        sum += t * getIndexOfSigns(*(source + i));
        t *= 16;
    }
    return sum;
}

static int getIndexOfSigns(char ch)
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
