#ifndef CMDS_H
#define CMDS_H

// commands table
typedef enum
{
    up_exit = 0,
    up_getSimpTemp,
    up_getHgFifoLen,
    up_getHgQueueLen,
    up_getHgData,
    up_getLgFifoLen,
    up_getLgQueueLen,
    up_getLgData,
    up_getTestFifoLen,
    up_getTestQueueLen,
    up_getTestData,
    up_getTdcFifoLen,
    up_getTdcQueueLen,
    up_getTdcData,
    up_getSi570Freq,
    up_setSi570Freq,
    up_writeAD9645,
    up_readAD9645,
    up_logicSelect,
    up_channelMask,
    up_writeRegTest,
    up_readRegTest,
    up_configCitiroc,
    up_configC11204,
    up_cleanQueue
} cmd_up;

#endif

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

#define gBoard (FEEControl::Instance())
class FEEControl
{
    friend class DataManager;

public:
    FEEControl(std::string ip = "192.168.1.115", int port = 1306);
    FEEControl(int boardNo);
    ~FEEControl();
    void InitPort(std::string ip = "192.168.1.115", int port = 1306);
    void InitPort(uint8_t boardNo);
    static FEEControl *&Instance();

    // Basic FEE Board Control
    bool server_exit();
    bool citiroc1a_configure(const char *sc_file_name, const char *probe_file_name);
    bool ad9645_reg_write(int addr, int wr_data);
    bool ad9645_reg_read(int addr, int &reg);
    bool si570_set(double freq);
    bool si570_get(double &freq);
    bool hg_fifo_length_read(int &len);
    bool hg_queue_length_read(int &len);
    bool hg_data_read(int data_num, uint32_t *data_addr);
    bool lg_fifo_length_read(int &len);
    bool lg_queue_length_read(int &len);
    bool lg_data_read(int data_num, uint32_t *data_addr);
    bool test_fifo_length_read(int &len);
    bool test_queue_length_read(int &len);
    bool test_data_read(int data_num, uint32_t *data_addr);
    bool tdc_fifo_length_read(int &len);
    bool tdc_queue_length_read(int &len);
    bool tdc_data_read(int data_num, uint32_t *data_addr);
    bool sipm_temp_read(double *temp);
    bool logic_select(int select_data);
    bool send_ch_masks(uint32_t mask_num);
    bool write_reg_test(int addr, int wr_data);
    bool clean_queue(int queue_id);

    /// @brief Read test register value
    /// @param addr Register number
    /// @param reg Output value
    /// @return whether is successful
    bool read_reg_test(int addr, int &reg);

    // Basic FEE Board Control END

    // Queue Read functions
    bool hg_fifo_read(int read_num, int loop_times, const char *hg_file_name);
    bool lg_fifo_read(int read_num, int loop_times, const char *lg_file_name);
    bool tdc_fifo_read(int read_num, int loop_times, const char *tdc_file_name);
    bool test_fifo_read(int read_num, int loop_times, const char *test_file_name);

    // HV Control
    /// @brief HV configuration through command line
    /// @param  none
    /// @return whether is successful
    bool HV_config(void);
    int HVSend(std::string scmd);   // HV configuration through string scmd
    int HVOFF();                    // Turn HV OFF
    int HVON();                     // Turn HV ON
    int HVMonitor();                // HPO
    int HVSet(double setT);         // Set bias HV
    void HVPrint();                 // Print HV information
    HVStatus GetHV() { return hv; } // Get HV Status
    // HV Control END

    // Other Board Status Monitor
    bool TestConnect();             // Test connnection of board
    int BoardCheck();               // Check HV, citiroc, ad9635, si570, fifo information
    bool ReadTemp();                // Read temperature
    int TestReg();                  // Test Register
    double ReadFreq();              // Read Clock Frequency
    bool BoardExit();               // Exit Board control
    void GetTemp(double *tempArray) // Get temperature in tempArray[out], no responsibility to check length of tempArray
    {
        memcpy(tempArray, fTemp, 4 * sizeof(double));
    };
    // Board Status Monitor END

    // Inside Info
    inline std::string GetIP() const { return ip_address; };
    inline int GetPort() const { return fPort; };
    inline unsigned __int64 GetSock() { return fSock; };
    static void GenerateIP(int boardNo, std::string &ip, int &port);
    static int GetPortBase() { return fPortBase; };
    // Inside Info END

    // CITIROC Configuration
    int SendConfig(ConfigFileParser *parser); // Send configure in parser to Board
    // CITIROC END

    // Mask Control
    /// @brief Set ch's Channel Mask as flag
    /// @param ch Channel to be set
    /// @param flag 0 or 1
    /// @param reg value of mask
    /// @return changed mask
    static uint32_t &GenerateChMask(int ch, bool flag, uint32_t &reg);
    static uint32_t GenerateMasks(bool *flag);
    static bool GetMask(int ch, uint32_t reg);
    // Mask Control END

    // FIFO Read Control
    /// @brief Read fifo Controller. Contains HG, LG, TDC fifo read actions. If no break flag is set, read fifo length continuously, until fifo length is larger than approximate leansNEvents * factors, factors varies from fifos.
    /// @param sleepms when fifos are not long enough, how long should be wait for next length read try, in unit of ms, default set as 1ms
    /// @param leastNEvents At least N events should be read. If fifo length is less than this, wait sleepms until next try
    /// @return whether is successful
    bool ReadFifo(int sleepms = 1, int leastNEvents = 10);
    void SetFifoReadBreak() { fBreakFlag = 1; }     // Set Read stop status
    bool GetDataValidation() { return fDataFlag; }; // Validate fifo data

    int GetHGQueueMonitor() { return fHGQueueLengthMonitor; }   // Get HG Queue length monitor
    int GetLGQueueMonitor() { return fLGQueueLengthMonitor; }   // Get LG Queue length monitor
    int GetTDCQueueMonitor() { return fTDCQueueLengthMonitor; } // Get TDC Queue length monitor

    int GetTestDataLength() { return fTestDataLength; }; // Get Data length;
    int GetHGDataLength() { return fHGDataLength; };     // Get HG Data length;
    int GetLGDataLength() { return fLGDataLength; };     // Get LG Data length;
    int GetTDCDataLength() { return fTDCDataLength; };   // Get TDC Data length;

    const uint32_t *GetTestFIFOData() { return fTestQueueData; }; // Get fifodata pointer
    const uint32_t *GetTDCFIFOData() { return fTDCQueueData; };   // Get fifodata pointer
    const uint32_t *GetHGFIFOData() { return fHGQueueData; };     // Get fifodata pointer
    const uint32_t *GetLGFIFOData() { return fLGQueueData; };     // Get fifodata pointer
    // FIFO Read Control END

private:
    // Connection status
    std::string ip_address;
    static const int fPortBase;
    static const int fIPBase;
    int fPort;
    int fBoardNum = 0;
    unsigned __int64 fSock; // SOCKET fSock, only not include <winsock2.h> in this file

    // Socket Init & Close
    bool fSockInitFlag = 0;

    // command, data address
    char cmd[MUON_TEST_CONTROL_SOCKET_MAX_LOAD_LENGTH];   // Useful for all commands, initiate at construction
    char reply[MUON_TEST_CONTROL_SOCKET_MAX_LOAD_LENGTH]; // only useful for HV control
    char *InitCMD(int length);                            // set cmd array as o
    void InitReply() { memset(reply, '\0', MUON_TEST_CONTROL_SOCKET_MAX_LOAD_LENGTH); };

    // Status data
    HVStatus hv;     // HV status
    double fTemp[4]; // temperature of SiPM0-7, 8-15, 16-23, 24-31

    // DAQ Read control
    uint32_t *fTestQueueData; // fifo data
    uint32_t *fHGQueueData;   // HG fifo data
    uint32_t *fLGQueueData;   // LG fifo data
    uint32_t *fTDCQueueData;  // TDC fifo data
    bool fDataFlag = 0;       // fifo data validation

    int fHGDataLength = 0;   // Read HG fifo data length
    int fLGDataLength = 0;   // Read LG fifo data length
    int fTDCDataLength = 0;  // Read TDC fifo data length
    int fTestDataLength = 0; // Read Test fifo data length

    int fHGQueueLengthMonitor = 0;  // Queue length monitor, update while readfifo
    int fLGQueueLengthMonitor = 0;  // Queue length monitor, update while readfifo
    int fTDCQueueLengthMonitor = 0; // Queue length monitor, update while readfifo

    int fifoReadCount = 0;        // fifo read counter
    volatile bool fBreakFlag = 0; // fifo read break flag

public:
    static const int fMaxSaveEvents;  // Limit of save events
    static const int fHGPointFactor;  // How many HG points in one event, not so accurate
    static const int fLGPointFactor;  // How many LG points in one event, not so accurate
    static const int fTDCPointFactor; // How many TDC points in one event, not so accurate

private:
    // private basic control function
    // int InitSock();
    // void CloseSock();
    bool start_socket();
    void close_socket();
    bool length_read(cmd_up cmd, int &len);
    bool data_read(cmd_up cmd, int data_num, uint32_t *data_addr);
    bool reg_write(cmd_up cmd, int addr, int wr_data);
    bool reg_read(cmd_up cmd, int addr, int &reg);
    bool send_cmd(cmd_up cmd, char *arg, int size);
    bool recv_data(char *buffer, int size);
    bool SendAll(char *buffer, int size);
    bool RecvAll(char *buffer, int size);

public:
    // public function for all members
    static void str_process(char *in_str, char *out_str);
    static int hexToDec(char *source);
    static int getIndexOfSigns(char ch);
};

// extern FEEControl *gBoard;
bool operator==(const FEEControl &a, const FEEControl &b);

//! TODO: add multi board manager
class FEEList
{
    void ScanBoard();

private:
    std::vector<FEEControl *> fBoardList;
};

#endif
// FEECONTROL_H
