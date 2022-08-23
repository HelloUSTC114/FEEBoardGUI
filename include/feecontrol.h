#ifndef FEECONTROL_H
#define FEECONTROL_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#define MUON_TEST_CONTROL_SOCKET_MAX_LOAD_LENGTH 2048
#define MUON_TEST_CONTROL_FIFO_BUFFER_LENGTH 64600

#include <string>
#include <vector>

class DataManager;
class ConfigFileParser;

struct HVStatus
{
    HVStatus() = default;
    HVStatus(std::string s);
    void Print();
    double OV_set = 0;
    double OV_moni = 0;
    double OC_moni = 0;
};

class FEEControl
{
    friend class DataManager;

public:
    FEEControl(std::string ip = "192.168.1.115", int port = 1306);
    ~FEEControl();
    void InitPort(std::string ip = "192.168.1.115", int port = 1306);

    int server_exit();

    int citiroc1a_configure(std::string sc_file_name, std::string probe_file_name);

    int ad9635_reg_write(const int dev_addr, int wr_data);

    int ad9635_reg_read(const int dev_addr);

    int si570_set(double freq);

    double si570_get();

    int lg_fifo_length_read();

    int lg_fifo_read(int data_num, uint32_t *data_addr);

    int hg_fifo_length_read();

    int hg_fifo_read(int data_num, uint32_t *data_addr);

    int test_fifo_length_read();

    int test_fifo_read(int data_num, uint32_t *data_addr);

    int sipm_temp_read(double *temp);

    int logic_select(int select_data);

    int send_ch_masks(uint32_t mask);

    int read_reg_test(int reg_num);

    int write_reg_test(int reg_num, int wr_data);

    int HV_config(void);            // HV configuration through command line
    int HVSend(std::string scmd);   // HV configuration through string scmd
    int HVOFF();                    // Turn HV OFF
    int HVON();                     // Turn HV ON
    int HVMonitor();                // HPO
    int HVSet(double setT);         // Set bias HV
    void HVPrint();                 // Print HV information
    HVStatus GetHV() { return hv; } // Get HV Status

    bool TestConnect(); // Test connnection of board
    int BoardCheck();   // Check HV, citiroc, ad9635, si570, fifo information
    bool ReadTemp();    // Read temperature
    int TestReg();      // Test Register
    double ReadFreq();  // Read Clock Frequency
    bool BoardExit();   // Exit Board control

    int ReadFifo(int sleepms = 200);                    // Read fifo once (sleep time in unit of ms)
    const uint32_t *GetFifoData() { return fifoData; }; // Get fifodata pointer
    bool GetDataValidation() { return fDataFlag; };     // Validate fifo data
    int GetDataLength() { return fDataLength; };        // Get Data length;

    void GetTemp(double *tempArray) { memcpy(tempArray, fTemp, 4 * sizeof(double)); }; // Get temperature in tempArray[out], no responsibility to check length of tempArray

    inline std::string GetIP() { return ip_address; };
    inline void SetIP(std::string ip) { ip_address = ip; };
    inline int GetPort() { return fPort; };
    inline void SetFEEPort(int port) { fPort = port; };
    inline unsigned __int64 GetSock() { return fSock; };

    // Configuration File Parser:
    int SendConfig(ConfigFileParser *parser); // Send configure in parser to Board

    void SetFifoReadBreak() { fBreakFlag = 1; }

    // uint32_t ParseChannelMask(int ch, )

    /// @brief Set ch's Channel Mask as flag
    /// @param ch Channel to be set
    /// @param flag 0 or 1
    /// @param reg value of mask
    /// @return changed mask
    static uint32_t &SetChannelMask(int ch, bool flag, uint32_t &reg);
    static uint32_t &SetMasks(bool *flag);
    static bool GetMask(int ch, uint32_t reg);

private:
    std::string ip_address;
    int fPort;

    unsigned __int64 fSock; // SOCKET fSock, only not include <winsock2.h> in this file
    int InitSock();
    void CloseSock();
    int fSockInitFlag;

    char cmd[MUON_TEST_CONTROL_SOCKET_MAX_LOAD_LENGTH];
    char reply[MUON_TEST_CONTROL_SOCKET_MAX_LOAD_LENGTH];
    char *InitCMD(int length);
    void InitReply() { memset(reply, '\0', MUON_TEST_CONTROL_SOCKET_MAX_LOAD_LENGTH); };
    HVStatus hv;

    double fTemp[4]; // temperature of SiPM0-7, 8-15, 16-23, 24-31

    uint32_t *fifoData;           // fifo data
    bool fDataFlag = 0;           // fifo data validation
    int fDataLength = 0;          // fifo data length
    int fifoReadCount = 0;        // fifo read counter
    volatile bool fBreakFlag = 0; // fifo read break flag

public:
};

extern FEEControl *gBoard;

class CITIROC_Config_Parse
{
public:
private:
};

class FEEList
{
    void ScanBoard();

private:
    std::vector<FEEControl *> fBoardList;
};

#endif

#ifndef CMDS_H
#define CMDS_H

// commands table
typedef enum
{

    cmd_exit = 0,

    cmd_getSimpTemp,

    cmd_getLgFifoLen,

    cmd_getLgFifoData,

    cmd_getHgFifoLen,

    cmd_getHgFifoData,

    cmd_getTestFifoLen,

    cmd_getTestFifoData,

    cmd_getSi570Freq,

    cmd_setSi570Freq,

    cmd_writeAD9635,

    cmd_readAD9635,

    cmd_logicSelect,

    cmd_channelMask,

    cmd_writeRegTest,

    cmd_readRegTest,

    cmd_configCitiroc,

    cmd_configC11204

} cmd_t;

#endif // FEECONTROL_H
