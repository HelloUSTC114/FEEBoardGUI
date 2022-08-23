#include "include/configfileparser.h"

#include <iostream>
#include <fstream>

#define SIZE_STR 150

#define _CRT_SECURE_NO_WARNINGS

#define FILE_NOT_OPEN_STRING "FILE_NOT_OPEN_STRING"
#define FILE_PARSER_ERROR "FILE_PARSER_ERROR"

using namespace std;

ConfigFileParser *gParser = new ConfigFileParser;

ConfigFileParser::ConfigFileParser()
{
}

bool ConfigFileParser::Init(string scFileName, string probeFileName)
{
    return ProcessConfigFile(scFileName, probeFileName, sConfig);
}

bool ConfigFileParser::ProcessConfigFile(string sc_file_name, string probe_file_name, string &sVoidOut)
{
    string &sTotal = sVoidOut;
    string sInit = "11111111";
    sTotal = sInit;

    ifstream finsc(sc_file_name), finpb(probe_file_name);
    cout << "Open: " << sc_file_name << '\t' << probe_file_name << endl;

    if (!finsc.is_open())
    {
        cout << "Can't Open File: " << sc_file_name << endl;
        sTotal = FILE_NOT_OPEN_STRING;
        return false;
    }
    if (!finpb.is_open())
    {
        cout << "Can't Open File: " << probe_file_name << endl;
        sTotal = FILE_NOT_OPEN_STRING;
        return false;
    }

    string sLineGetTemp, sLineProcessedTemp;

    for (int lineCounter = 0; getline(finsc, sLineGetTemp); lineCounter++)
    {
        str_process(sLineGetTemp, sLineProcessedTemp);
        if (!CheckOneLineConfig(sLineProcessedTemp))
        {
            cout << "Error: Unexpected char in SC file, Line " << lineCounter + 1 << endl;
            cout << sLineGetTemp << endl;
            cout << "Start index of this line: " << sTotal.size() << endl;
            sTotal = FILE_PARSER_ERROR;
            // cout << '\t' << "End index: " << sTotal.size() << '\t' << endl;
            finsc.close();
            return false;
        }
        sTotal += sLineProcessedTemp;
    }
    if (sTotal.size() != SC_FILE_STRING_LENGTH)
    {
        cout << "ERROR: Total string length: " << sTotal.size() << endl;
        cout << "Please Check SC File: " << endl;
        sTotal = FILE_PARSER_ERROR;
        finsc.close();
        return false;
    }

    for (int lineCounter = 0; getline(finpb, sLineGetTemp); lineCounter++)
    {
        str_process(sLineGetTemp, sLineProcessedTemp);
        if (!CheckOneLineConfig(sLineProcessedTemp))
        {
            cout << "Error: Unexpected char in Probe file, Line " << lineCounter + 1 << endl;
            cout << sLineGetTemp << endl;
            cout << "Start index of this line: " << sTotal.size() << endl;
            sTotal = FILE_PARSER_ERROR;
            // cout << '\t' << "End index: " << sTotal.size() << '\t' << endl;
            finpb.close();
            return false;
        }
        sTotal += sLineProcessedTemp;
    }

    if (sTotal.size() != TOTAL_PASER_STRING_LENGTH)
    {
        cout << "ERROR: Total string length: " << sTotal.size() << endl;
        cout << "Please Check Probe File: " << endl;
        sTotal = FILE_PARSER_ERROR;
        finpb.close();
        return false;
    }

    finsc.close();
    finpb.close();
    cout << "Parsed Succsessfully" << endl;
    return true;
}

bool ConfigFileParser::PrintConfigFile(std::string scFileName, std::string probeFileName)
{
    if (!sConfigValidate())
        return false;

    ifstream ftestsc(scFileName), ftestpb(probeFileName);
    if (ftestsc.is_open() || ftestpb.is_open())
    {
        cout << "Error: File exists" << endl;
        ftestsc.close();
        ftestpb.close();
        return false;
    }

    ofstream foutsc(scFileName), foutpb(probeFileName);
    if (!foutsc.is_open() || !foutpb.is_open())
    {
        cout << "Error: Cannot open writing files" << endl;
        return false;
    }

    // 4-bit DAC_t ([0..3])
    for (int ch = 0; ch < 32; ch++)
    {
        for (int bit = 0; bit < 4; bit++)
            foutsc << GetBit(DAC_t + ch * 4 + bit);
        foutsc << " % Ch" << ch << " 4-bit DAC_t ([0..3])" << endl;
    }
    // 4-bit DAC ([0..3])
    for (int ch = 0; ch < 32; ch++)
    {
        for (int bit = 0; bit < 4; bit++)
            foutsc << GetBit(DAC + ch * 4 + bit);
        foutsc << " % Ch" << ch << " 4-bit DAC ([0..3])" << endl;
    }

    int nBit = Enable_Disc;
    foutsc << GetBit(nBit++) << " %% Enable discriminator" << endl;
    foutsc << GetBit(nBit++) << " %% Disable trigger discriminator power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Select latched (RS : 1) or direct output (trigger : 0)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable Discriminator Two" << endl;
    foutsc << GetBit(nBit++) << " %% Disable trigger discriminator power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% EN_4b_dac" << endl;
    foutsc << GetBit(nBit++) << " %% PP: 4b_dac" << endl;
    foutsc << GetBit(nBit++) << " %% EN_4b_dac_t" << endl;
    foutsc << GetBit(nBit++) << " %% PP: 4b_dac_t" << endl;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 8; j++)
            foutsc << GetBit(nBit++);
        foutsc << " ";
    }
    foutsc << " %% Allows to Mask Discriminator (channel 0 to 31) [active low]" << endl;

    foutsc << GetBit(nBit++) << " %% Disable High Gain Track & Hold power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable High Gain Track & Hold" << endl;
    foutsc << GetBit(nBit++) << " %% Disable Low Gain Track & Hold power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable Low Gain Track & Hold" << endl;
    foutsc << GetBit(nBit++) << " %% SCA bias ( 1 = weak bias, 0 = high bias 5MHz ReadOut Speed)" << endl;
    foutsc << GetBit(nBit++) << " %% PP: HG Pdet" << endl;
    foutsc << GetBit(nBit++) << " %% EN_HG_Pdet" << endl;
    foutsc << GetBit(nBit++) << " %% PP: LG Pdet" << endl;
    foutsc << GetBit(nBit++) << " %% EN_LG_Pdet" << endl;
    foutsc << GetBit(nBit++) << " %% Sel SCA or PeakD HG" << endl;
    foutsc << GetBit(nBit++) << " %% Sel SCA or PeakD LG" << endl;
    foutsc << GetBit(nBit++) << " %% Bypass Peak Sensing Cell" << endl;
    foutsc << GetBit(nBit++) << " %% Sel Trig Ext PSC" << endl;
    foutsc << GetBit(nBit++) << " %% Disable fast shaper follower power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable fast shaper" << endl;
    foutsc << GetBit(nBit++) << " %% Disable fast shaper power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Disable low gain slow shaper power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable Low Gain Slow Shaper" << endl;

    for (int i = 0; i < 3; i++)
        foutsc << GetBit(nBit++);
    foutsc << " %% Low gain shaper time constant commands (0…2)  [active low] 100" << endl;

    foutsc << GetBit(nBit++) << " %% Disable high gain slow shaper power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable high gain Slow Shaper" << endl;

    for (int i = 0; i < 3; i++)
        foutsc << GetBit(nBit++);
    foutsc << " %% High gain shaper time constant commands (0…2)  [active low] 100" << endl;

    foutsc << GetBit(nBit++) << " %% Low Gain PreAmp bias ( 1 = weak bias, 0 = normal bias)" << endl;
    foutsc << GetBit(nBit++) << " %% Disable High Gain preamp power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable High Gain preamp" << endl;
    foutsc << GetBit(nBit++) << " %% Disable Low Gain preamp power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable Low Gain preamp" << endl;
    foutsc << GetBit(nBit++) << " %% Select LG PA to send to Fast Shaper" << endl;
    foutsc << GetBit(nBit++) << " %% Enable 32 input 8-bit DACs" << endl;
    foutsc << GetBit(nBit++) << " %% 8-bit input DAC Voltage Reference (1 = external 4,5V , 0 = internal 2,5V)" << endl;

    for (int ch = 0; ch < 32; ch++)
    {
        for (int i = 0; i < 8; i++)
            foutsc << GetBit(nBit++);
        foutsc << " " << GetBit(nBit++) << " %% Input 8-bit DAC Data channel " << ch << " - (DAC7…DAC0 + DAC ON), higher-higher bias" << endl;
    }

    for (int ch = 0; ch < 32; ch++)
    {
        for (int i = 0; i < 6; i++)
            foutsc << GetBit(nBit++);
        foutsc << " ";
        for (int i = 0; i < 6; i++)
            foutsc << GetBit(nBit++);
        foutsc << " ";
        for (int i = 0; i < 3; i++)
            foutsc << GetBit(nBit++);
        foutsc << " %% Ch" << ch << "   PreAmp config (HG gain[5..0], LG gain [5..0], CtestHG, CtestLG, PA disabled)" << endl;
    }

    foutsc << GetBit(nBit++) << " %% Disable Temperature Sensor power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable Temperature Sensor" << endl;
    foutsc << GetBit(nBit++) << " %% Disable BandGap power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable BandGap" << endl;
    foutsc << GetBit(nBit++) << " %% Enable DAC1" << endl;
    foutsc << GetBit(nBit++) << " %% Disable DAC1 power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable DAC2" << endl;
    foutsc << GetBit(nBit++) << " %% Disable DAC2 power pulsing mode (force ON)" << endl;

    for (int i = 0; i < 10; i++)
        foutsc << GetBit(nBit++);
    foutsc << " %% 10-bit DAC1 (MSB-LSB): 00 1100 0000 for 0.5 p.e. charge discriminator threshold" << endl;
    for (int i = 0; i < 10; i++)
        foutsc << GetBit(nBit++);
    foutsc << " %% 10-bit DAC2 (MSB-LSB): 00 1100 0000 for 0.5 p.e. time discriminator threshold" << endl;

    foutsc << GetBit(nBit++) << " %% Enable High Gain OTA  -- start byte " << endl;
    foutsc << GetBit(nBit++) << " %% Disable High Gain OTA power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable Low Gain OTA" << endl;
    foutsc << GetBit(nBit++) << " %% Disable Low Gain OTA power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable Probe OTA" << endl;
    foutsc << GetBit(nBit++) << " %% Disable Probe OTA power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Otaq test bit" << endl;
    foutsc << GetBit(nBit++) << " %% Enable Val_Evt receiver" << endl;
    foutsc << GetBit(nBit++) << " %% Disable Val_Evt receiver power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable Raz_Chn receiver" << endl;
    foutsc << GetBit(nBit++) << " %% Disable Raz Chn receiver power pulsing mode (force ON)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable digital multiplexed output (hit mux out)" << endl;
    foutsc << GetBit(nBit++) << " %% Enable digital OR32 output" << endl;
    foutsc << GetBit(nBit++) << " %% Enable digital OR32 Open Collector output" << endl;
    foutsc << GetBit(nBit++) << " %% Trigger Polarity" << endl;
    foutsc << GetBit(nBit++) << " %% Enable digital OR32_T Open Collector output" << endl;
    foutsc << GetBit(nBit++) << " %% Enable 32 channels triggers outputs" << endl;

    foutsc.close();

    // Probe file:
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 8; j++)
            foutpb << GetBit(nBit++);
        foutpb << " ";
    }
    foutpb << " %% Out_fs   From channel 0 to 31 Analog" << endl;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 8; j++)
            foutpb << GetBit(nBit++);
        foutpb << " ";
    }
    foutpb << " %% Out_ssh_LG   From channel 0 to 31 Analog" << endl;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 8; j++)
            foutpb << GetBit(nBit++);
        foutpb << " ";
    }
    foutpb << " %% PeakSensing_modeb_LG	From channel 0 to 31 Digital" << endl;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 8; j++)
            foutpb << GetBit(nBit++);
        foutpb << " ";
    }
    foutpb << " %% Out_ssh_HG From channel 0 to 31 Analog" << endl;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 8; j++)
            foutpb << GetBit(nBit++);
        foutpb << " ";
    }
    foutpb << " %% PeakSensing_modeb_HG	32 From channel 0 to 31 Digital" << endl;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 2; j++)
            foutpb << GetBit(nBit++);
        foutpb << " ";
    }
    foutpb << "     ";
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 2; j++)
            foutpb << GetBit(nBit++);
        foutpb << " ";
    }
    foutpb << " %% Out_PA_HG/Out_PA_LG	64 From channel 0 to 15 Analog" << endl;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 2; j++)
            foutpb << GetBit(nBit++);
        foutpb << " ";
    }
    foutpb << "     ";
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 2; j++)
            foutpb << GetBit(nBit++);
        foutpb << " ";
    }
    foutpb << " %% Out_PA_HG/Out_PA_LG	64 From channel 16 to 31 Analog" << endl;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 8; j++)
            foutpb << GetBit(nBit++);
        foutpb << " ";
    }
    foutpb << " %% Input DAC output channel 0 to 31 Analog" << endl;

    foutpb.close();

    return true;
}

bool ConfigFileParser::EnableMainBiasDAC(bool flag)
{
    if (!sConfigValidate())
        return false;

    sConfig.at(ConfigStartIndex::Enable_32_input_8_bit_DAC) = ConvertFlag(flag);
    return true;
}

int ConfigFileParser::GetMainBiasDACSwitch()
{
    if (!sConfigValidate())
        return -1;

    return sConfig.at(ConfigStartIndex::Enable_32_input_8_bit_DAC);
}

bool ConfigFileParser::SetBiasDAC(int ch, int bias)
{
    if (!sConfigValidate() || !chValidate(ch))
        return false;

    int startIndex = ConfigStartIndex::Bias_Voltage_DAC + ch * 9;
    string biasString = ConvertBiasDAC(bias);
    for (int i = 0; i < 8; i++)
    {
        sConfig.at(startIndex + i) = biasString.at(i);
    }
    return true;
}

int ConfigFileParser::GetBiasDAC(int ch)
{
    if (!sConfigValidate() || !chValidate(ch))
        return -1;

    int startIndex = ConfigStartIndex::Bias_Voltage_DAC + ch * 9;
    string biasString = sConfig.substr(startIndex, 8);
    return ConvertBiasDAC(biasString);
}

bool ConfigFileParser::EnableBiasDAC(int ch, bool flag) // DAC switch for single channels
{
    if (!sConfigValidate() || !chValidate(ch))
        return false;

    int startIndex = ConfigStartIndex::Bias_Voltage_DAC + ch * 9;
    sConfig.at(startIndex + 8) = ConvertFlag(flag);
    return true;
}

int ConfigFileParser::GetBiasDACSwitch(int ch)
{
    if (!sConfigValidate() || !chValidate(ch))
        return -1;
    int startIndex = ConfigStartIndex::Bias_Voltage_DAC + ch * 9;
    return ConvertFlag(sConfig.data() + (startIndex + 8));
}

bool ConfigFileParser::SetDACRef(bool ref)
{
    if (!sConfigValidate())
        return false;
    int startIndex = ConfigStartIndex::VRef_8_bit_input_DAC;
    sConfig.at(startIndex) = ConvertFlag(ref);
    return true;
}

int ConfigFileParser::GetDACRef() // 1 = external 4.5V, 0 = internal 2.5V, -1 = Error
{
    if (!sConfigValidate())
        return -1;
    int startIndex = ConfigStartIndex::VRef_8_bit_input_DAC;
    return ConvertFlag(sConfig.data() + (startIndex));
}

bool ConfigFileParser::Set_AMP_HG_DAC(int ch, int amp) // [5, ..., 0] in sConfig String
{
    if (!sConfigValidate() || !chValidate(ch))
        return false;

    int startIndex = ConfigStartIndex::Amp_DAC + ch * 15;
    string ampString = ConvertAmp(amp);
    for (int i = 0; i < 6; i++)
    {
        sConfig.at(startIndex + i) = ampString.at(i);
    }
    return true;
}

bool ConfigFileParser::Set_AMP_LG_DAC(int ch, int amp) // [5, ..., 0] in sConfig String
{
    if (!sConfigValidate() || !chValidate(ch))
        return false;

    int startIndex = ConfigStartIndex::Amp_DAC + ch * 15;
    string ampString = ConvertAmp(amp);
    for (int i = 0; i < 6; i++)
    {
        sConfig.at(startIndex + i + 6) = ampString.at(i);
    }

    return true;
}

int ConfigFileParser::Get_AMP_HG_DAC(int ch) // Give DAC value
{
    if (!sConfigValidate() || !chValidate(ch))
        return -1;

    int startIndex = ConfigStartIndex::Amp_DAC + ch * 15;
    string ampString = sConfig.substr(startIndex, 6);
    return ConvertAmp(ampString);
}

int ConfigFileParser::Get_AMP_LG_DAC(int ch) // Give DAC value
{
    if (!sConfigValidate() || !chValidate(ch))
        return -1;

    int startIndex = ConfigStartIndex::Amp_DAC + ch * 15;
    string ampString = sConfig.substr(startIndex + 6, 6);
    return ConvertAmp(ampString);
}

int ConfigFileParser::Get_PA_Switcher(int ch) // Give DAC value
{
    if (!sConfigValidate() || !chValidate(ch))
        return -1;

    int startIndex = ConfigStartIndex::Amp_DAC + ch * 15;
    return ConvertFlag(sConfig.data() + startIndex + 14);
}

bool ConfigFileParser::DisablePA(int ch, bool flag) // [5, ..., 0] in sConfig String
{
    if (!sConfigValidate() || !chValidate(ch))
        return false;

    int startIndex = ConfigStartIndex::Amp_DAC + ch * 15;
    sConfig.at(startIndex + 14) = ConvertFlag(flag);

    return true;
}

bool ConfigFileParser::SetDiscDAC1(int dac) // Set Threshold DAC1 value
{
    if (!sConfigValidate())
        return false;

    int startIndex = ConfigStartIndex::Disc_DAC1;
    string sDAC = ConvertDiscDAC(dac);
    for (int i = 0; i < 10; i++)
    {
        sConfig.at(startIndex + i) = sDAC.at(i);
    }
    return true;
}

bool ConfigFileParser::SetDiscDAC2(int dac) // Set Threshold DAC2 value
{
    if (!sConfigValidate())
        return false;

    int startIndex = ConfigStartIndex::Disc_DAC2;
    string sDAC = ConvertDiscDAC(dac);
    for (int i = 0; i < 10; i++)
    {
        sConfig.at(startIndex + i) = sDAC.at(i);
    }
    return true;
}

int ConfigFileParser::GetDiscDAC1() // Get Threshold DAC1 value
{
    if (!sConfigValidate())
        return -1;

    int startIndex = ConfigStartIndex::Disc_DAC1;
    string sDAC = sConfig.substr(startIndex, 10);
    return ConvertDiscDAC(sDAC);
}

int ConfigFileParser::GetDiscDAC2() // Get Threshold DAC2 value
{
    if (!sConfigValidate())
        return -1;

    int startIndex = ConfigStartIndex::Disc_DAC2;
    string sDAC = sConfig.substr(startIndex, 10);
    return ConvertDiscDAC(sDAC);
}

int ConfigFileParser::GetBit(int nPos)
{
    if (!sConfigValidate() || !bitValidate(nPos))
        return -1;

    return ConvertFlag(sConfig.data() + (nPos));
}

bool ConfigFileParser::SetBit(int nPos, bool flag)
{
    if (!sConfigValidate() || !bitValidate(nPos))
        return false;

    sConfig.at(nPos) = ConvertFlag(flag);
    return true;
}

char ConfigFileParser::ConvertFlag(bool flag)
{
    if (flag)
        return '1';
    else
        return '0';
}

std::string ConfigFileParser::ConvertBiasDAC(unsigned char bias)
{
    string sBias;
    for (int i = 0; i < 8; i++)
    {
        sBias.push_back(ConvertFlag(bias << i & 0x80));
        // sBias.
    }
    return sBias;
}

int ConfigFileParser::ConvertBiasDAC(std::string sBias) //  8 bit Bias DAC, from 0 to 255, (DAC7…DAC0)
{
    unsigned char bias = 0;
    for (int i = 0; i < 8; i++)
    {
        bias |= ((0x80 & (ConvertFlag(sBias.data() + i) << 7)) >> i);
    }
    return bias;
}

string ConfigFileParser::ConvertAmp(unsigned char amp)
{
    string sAmp;
    for (int i = 0; i < 6; i++)
    {
        sAmp.push_back(ConvertFlag(amp << i & 0x20));
    }
    return sAmp;
}

int ConfigFileParser::ConvertAmp(std::string sAmp)
{
    unsigned char amp = 0;
    for (int i = 0; i < 6; i++)
    {
        amp |= ((0x20 & (ConvertFlag(sAmp.data() + i) << 5)) >> i);
    }
    return amp;
}

string ConfigFileParser::ConvertDiscDAC(unsigned short dac)
{
    string sDAC;
    for (int i = 0; i < 10; i++)
    {
        sDAC.push_back(ConvertFlag(dac << i & 0x200));
    }
    return sDAC;
}

int ConfigFileParser::ConvertDiscDAC(std::string sDAC)
{
    unsigned short dac = 0;
    for (int i = 0; i < 10; i++)
    {
        dac |= ((0x200 & (ConvertFlag(sDAC.data() + i) << 9)) >> i);
    }
    return dac;
}

bool ConfigFileParser::CheckOneLineConfig(const string &s)
{
    for (int i = 0; i < s.size(); i++)
    {
        if (s.at(i) != '0' && s.at(i) != '1')
        {
            return false;
        }
    }
    return true;
}

void ConfigFileParser::str_process(const char *in_str, char *out_str)
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

void ConfigFileParser::str_process(const string &in_str, string &out_str)
{
    out_str.clear();
    if (out_str.size() < in_str.length())
    {
        out_str.resize(in_str.length());
    }
    str_process(in_str.data(), out_str.data());
    out_str = out_str.c_str();
}
