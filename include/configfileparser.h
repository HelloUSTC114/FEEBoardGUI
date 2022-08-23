#ifndef CONFIGFILEPARSER_H
#define CONFIGFILEPARSER_H

#define _CRT_SECURE_NO_WARNINGS
#define TOTAL_PASER_STRING_LENGTH 1408
#define SC_FILE_STRING_LENGTH 1152
#include <string>

class ConfigFileParser
{
public:
    ConfigFileParser();
    bool Init(std::string scFileName = "../MuonTestControl/Configuration/sc_register_pd.txt", std::string probeFileName = "../MuonTestControl/Configuration/probe_register.txt");
    static bool ProcessConfigFile(std::string scFileName, std::string probeFileName, std::string &sVoidOut);
    bool PrintConfigFile(std::string scFileName, std::string probeFileName); // Print Config to Files
    inline const std::string &GetString() { return sConfig; }

    bool EnableMainBiasDAC(bool flag);     //
    int GetMainBiasDACSwitch();            // -1 means read error
    bool SetBiasDAC(int ch, int bias);     //  8 bit Bias DAC, from 0 to 255
    int GetBiasDAC(int ch);                //
    bool EnableBiasDAC(int ch, bool flag); // DAC switch for single channels
    int GetBiasDACSwitch(int ch);          // DAC switch for single channels
    int GetDACRef();                       // 1 = external 4.5V, 0 = internal 2.5V, -1 = Error
    bool SetDACRef(bool ref);              // 1 = external 4.5V, 0 = internal 2.5V

    bool Set_AMP_HG_DAC(int ch, int amp); // [5, ..., 0] in sConfig String
    bool Set_AMP_LG_DAC(int ch, int amp); // [5, ..., 0] in sConfig String
    int Get_AMP_HG_DAC(int ch);           // Give DAC value
    int Get_AMP_LG_DAC(int ch);           // Give DAC value
    int Get_PA_Switcher(int ch);          // Get PreAmp swithcer
    bool DisablePA(int ch, bool flag);    // Disable PreAmp

    bool SetDiscDAC1(int dac); // Set Threshold DAC1 value
    bool SetDiscDAC2(int dac); // Set Threshold DAC2 value
    int GetDiscDAC1();         // Get Threshold DAC1 value
    int GetDiscDAC2();         // Get Threshold DAC2 value

    int GetBit(int nPos);             // Get bit in position nPos
    bool SetBit(int nPos, bool flag); // Set bit in position nPos, successful with 1, fail return 0

    inline bool sConfigValidate() { return (sConfig.size() == TOTAL_PASER_STRING_LENGTH); }; // validate sConfig
private:
    std::string sConfig;
    inline bool chValidate(int ch) { return (ch >= 0 && ch < 32); }
    inline bool bitValidate(int nPos) { return (nPos >= 0 && nPos < TOTAL_PASER_STRING_LENGTH); };
    static bool CheckOneLineConfig(const std::string &s);

public:
    static void str_process(const char *in_str, char *out_str);
    static void str_process(const std::string &in_str, std::string &out_str);
    static char ConvertFlag(bool flag);
    inline static bool ConvertFlag(char *c) { return (c[0] == '1') ? 1 : 0; };
    static std::string ConvertBiasDAC(unsigned char bias); //  8 bit Bias DAC, from 0 to 255, (DAC7…DAC0)
    static int ConvertBiasDAC(std::string sBias);          //  8 bit Bias DAC, from 0 to 255, (DAC7…DAC0)
    static std::string ConvertAmp(unsigned char amp);      //  6 bit Amp, from 0 to 63, [5, ..., 0]
    static int ConvertAmp(std::string sAmp);               //  6 bit Amp, from 0 to 63, [5, ..., 0]
    static std::string ConvertDiscDAC(unsigned short dac); // 10 bit threshold DAC, 10-bit DAC1 (MSB-LSB): 00 1100 0000 for 0.5 p.e. charge discriminator
    static int ConvertDiscDAC(std::string sDAC);           // 10 bit threshold DAC, 10-bit DAC1 (MSB-LSB): 00 1100 0000 for 0.5 p.e. charge discriminator
};

extern ConfigFileParser *gParser;

typedef enum
{
    Head = 0,
    DAC_t = 8,                                             // 4-bit DAC_t ([0..3])
    DAC = 136,                                             // 4-bit DAC ([0..3])
    Enable_Disc = 264,                                     // Enable discriminator
    Disable_Trigger_Disc_Power_Pulseing = 265,             // Disable trigger discriminator power pulsing mode (force ON)
    Latched_Or_Direct_Output = 266,                        // Select latched (RS : 1) or direct output (trigger : 0)
    Enable_Disc_2 = 267,                                   // Enable Discriminator Two
    Disable_Trigger_Disc_Power_Pulseing2,                  // Disable trigger discriminator power pulsing mode (force ON)
    EN_4b_dac,                                             // EN_4b_dac
    PP_4b_dac,                                             // PP: 4b_dac
    EN_4b_dac_t,                                           // EN_4b_dac_t
    PP_4b_dac_t,                                           // PP: 4b_dac_t
    Mask_Disc = 273,                                       // Allows to Mask Discriminator (channel 0 to 31) [active low]
    Disable_High_Gain_Track_Hold_power_pulsing_Mode = 305, // Disable High Gain Track & Hold power pulsing mode (force ON)
    Enable_High_Gain_Track_Hold,                           // Enable High Gain Track & Hold
    Disable_Low_Gain_Track_Hold_power_pulsing_Mode,        // Disable Low Gain Track & Hold power pulsing mode (force ON)
    Enable_Low_Gain_Track_Hold,                            // Enable Low Gain Track & Hold
    SCA_Bias,                                              // SCA bias ( 1 = weak bias, 0 = high bias 5MHz ReadOut Speed)
    PP_HG_Pdet,                                            // PP: HG Pdet
    EN_HG_Pdet,                                            // EN_HG_Pdet
    PP_LG_Pdet,                                            // PP: LG Pdet
    EN_LG_Pdet,                                            // EN_LG_Pdet
    Sel_SCA_PeakD_HG,                                      // Sel SCA or PeakD HG
    Sel_SCA_PeakD_LG,                                      // Sel SCA or PeakD LG
    Bypass_Peak_Sensing_Cell,                              // Bypass Peak Sensing Cell
    Sel_Trig_Ext_PSC,                                      // Sel Trig Ext PSC
    Disable_Fast_Shaper_Follower_Power_Pulsing_Mode,       // Disable fast shaper follower power pulsing mode (force ON)
    Enable_Fast_Shaper,                                    // Enable fast shaper
    Disable_Fast_Shaper_Power_Pulsing_Mode,                // Disable fast shaper power pulsing mode (force ON)
    Disable_LG_Slow_Shaper_Power_Pulsing_Mode,             // Disable low gain slow shaper power pulsing mode (force ON)
    Enable_LG_Slow_Shaper_Power,                           // Enable Low Gain Slow Shaper
    LG_Shaper_Time_Constant = 323,                         // Low gain shaper time constant commands (0…2)  [active low] 100
    Disable_HG_Slow_Shaper_Power_Pulsing_Mode = 326,       // Disable high gain slow shaper power pulsing mode (force ON)
    Enable_HG_Slow_Shaper_Power,                           // Enable high gain Slow Shaper
    HG_Shaper_Time_Constant = 328,                         // High gain shaper time constant commands (0…2)  [active low] 100
    LG_Pre_Amp_Bias = 331,                                 // Low Gain PreAmp bias ( 1 = weak bias, 0 = normal bias)
    Disable_HG_Preamp_Power_Pulsing_Mode,                  // Disable High Gain preamp power pulsing mode (force ON)
    Enable_HG_Preamp,                                      // Enable High Gain preamp
    Disable_LG_Preamp_Power_Pulsing_Mode,                  // Disable Low Gain preamp power pulsing mode (force ON)
    Enable_LG_Preamp,                                      // Enable Low Gain preamp
    Select_LG_PA_2_Fast_Shaper,                            // Select LG PA to send to Fast Shaper
    Enable_32_input_8_bit_DAC,                             // Enable 32 input 8-bit DACs
    VRef_8_bit_input_DAC = 338,                            // 8-bit input DAC Voltage Reference (1 = external 4,5V , 0 = internal 2,5V)

    Bias_Voltage_DAC = 339, // Input 8-bit DAC Data channel 0 – (DAC7…DAC0 + DAC ON), higher-higher bias, 9 bit for one channel
    Amp_DAC = 627,          // PreAmp config (HG gain[5..0], LG gain [5..0], CtestHG, CtestLG, PA disabled), 15 bit for one channel

    Disable_Temperature_Sensor_power_pulsing_Mode = 1107, // Disable Temperature Sensor power pulsing mode (force ON)
    Enable_Temperature_Sensor,                            // Enable Temperature Sensor
    Disable_BandGap_power_pulsing_Mode,                   // Disable BandGap power pulsing mode (force ON)
    Enable_BandGap,                                       // Enable BandGap
    Enable_DAC1,                                          // Enable DAC1
    Disable_DAC1_power_pulsing_Mode,                      // Disable DAC1 power pulsing mode (force ON)
    Enable_DAC2,                                          // Enable DAC2
    Disable_DAC2_power_pulsing_Mode,                      // Disable DAC2 power pulsing mode (force ON)
    Disc_DAC1 = 1115,                                     // 10-bit DAC1 (MSB-LSB): 00 1100 0000 for 0.5 p.e. charge discriminator threshold
    Disc_DAC2 = 1125,                                     // 10-bit DAC2 (MSB-LSB): 00 1100 0000 for 0.5 p.e. time discriminator threshold
    Enable_High_Gain_OTA_ = 1135,                         // Enable High Gain OTA//  -- start byte
    Disable_High_Gain_OTA_Power_Pulsing_Mode,             // Disable High Gain OTA power pulsing mode (force ON)
    Enable_Low_Gain_OTA,                                  // Enable Low Gain OTA
    Disable_Low_Gain_OTA_power_Pulsing_Mode,              // Disable Low Gain OTA power pulsing mode (force ON)
    Enable_Probe_OTA,                                     // Enable Probe OTA
    Disable_Probe_OTA_power_Pulsing_Mode,                 // Disable Probe OTA power pulsing mode (force ON)
    Otaq_test_bit,                                        // Otaq test bit
    Enable_Val_Evt_receiver,                              // Enable Val_Evt receiver
    Disable_Val_Evt_receiver_power_Pulsing_Mode,          // Disable Val_Evt receiver power pulsing mode (force ON)
    Enable_Raz_Chn_receiver,                              // Enable Raz_Chn receiver
    Disable_Raz_Chn_receiver_Pulsing_Mode,                // Disable Raz Chn receiver power pulsing mode (force ON)
    Enable_digital_multiplexed_output,                    // Enable digital multiplexed output (hit mux out)
    Enable_digital_OR32_output,                           // Enable digital OR32 output
    Enable_digital_OR32_Open_Collector_output,            // Enable digital OR32 Open Collector output
    Trigger_Polarity,                                     // Trigger Polarity
    Enable_digital_OR32_T_Open_Collector_output,          // Enable digital OR32_T Open Collector output
    Enable_32_channels_triggers_Outputs,                  // Enable 32 channels triggers outputs

    Out_fs = 1152,                    // Out_fs From channel 0 to 31 Analog
    Out_ssh_LG = 1184,                // Out_ssh_LG	From channel 0 to 31 Analog
    PeakSensing_modeb_LG = 1216,      // PeakSensing_modeb_LG	From channel 0 to 31 Digital
    Out_ssh_HG = 1248,                // Out_ssh_HG From channel 0 to 31 Analog
    PeakSensing_modeb_HG = 1280,      // PeakSensing_modeb_HG	32 From channel 0 to 31 Digital
    Out_PA_HG_Out_PA_LG_0_15 = 1312,  // Out_PA_HG/Out_PA_LG	64 From channel 0 to 15 Analog
    Out_PA_HG_Out_PA_LG_16_31 = 1344, // Out_PA_HG/Out_PA_LG	64 From channel 16 to 31 Analog
    Input_DAC_output = 1376,          // Input DAC output channel 0 to 31 Analog
} ConfigStartIndex;

#endif // CONFIGFILEPARSER_H
