
#include "General.h"

// C++
#include <iostream>
#include <string>
#include <sstream>

// Socket and other APIs
#include <WinSock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")

namespace UserDefine
{
    std::stringstream gss;

    bool ParseLineForVector(std::string sInput, std::vector<double> &vOutput)
    {
        gss.clear();
        gss.str(sInput);
        if (!gss.good())
        {
            std::cout << "Error while parsing: stringstream is bad. " << std::endl;
            return false;
        }
        double j_start = 0, i_step = 0, k_end = 0;
        char c;
        gss >> c;
        if (c != '[')
        {
            std::cout << "Error while parsing: char format error: No left bracket \'[\'" << std::endl;
            return false;
        }
        gss >> j_start;
        if (!gss.good())
        {
            std::cout << "Error while parsing: stringstream is bad. " << std::endl;
            return false;
        }
        gss >> c;
        if (c != ':')
        {
            std::cout << "Error while parsing: char format error: " << c << " Should be \':\'" << std::endl;
            return false;
        }
        gss >> i_step;
        if (!gss.good())
        {
            std::cout << "Error while parsing: stringstream is bad. " << std::endl;
            return false;
        }
        gss >> c;
        if (c != ':')
        {
            std::cout << "Error while parsing: char format error: " << c << " Should be \':\'" << std::endl;
            return false;
        }
        gss >> k_end;
        if (!gss.good())
        {
            std::cout << "Error while parsing: stringstream is bad. " << std::endl;
            return false;
        }
        gss >> c;
        if (c != ']')
        {
            std::cout << "Error while parsing: char format error: No right bracket \']\' " << std::endl;
            return false;
        }

        // Judge whether this three values are valid. If i_step == 0, or k_end-j_start have different signs, return false
        if ((k_end - j_start) * (i_step) < 0)
            return false;
        // If step is 0, return false;
        if (i_step == 0)
            return false;
        vOutput.clear();
        for (double iter = j_start; (iter - j_start) * (iter - k_end) <= 0; iter += i_step)
        {
            vOutput.push_back(iter);
        }
        gss.clear();

        if (vOutput.size() == 0)
            return false;
        return true;
    }

    bool ParseLineForArray(std::string sInput, std::vector<double> &vOutput)
    {
        gss.clear();
        gss.str(sInput);
        double testPoint = 0;
        char c;
        vOutput.clear();
        for (;;)
        {
            gss >> testPoint;
            if (!bool(gss))
                break;
            vOutput.push_back(testPoint);
            if (gss.eof())
                break;
            gss >> c;
            if (c != ',' && c != ':')
            {
                std::cout << "Warning: should use \',\' as delimiter. You are using: \'" << c << "\'. May cause error" << std::endl;
            }
            else if (c == ':')
            {
                std::cout << "Error: Detected \':\', may reveal wrong vector format, pleas check." << std::endl;
                gss.clear();
                vOutput.clear();
                return false;
            }
        }
        gss.clear();
        if (vOutput.size() == 0)
            return false;
        return true;
    }

    bool ParseLine(std::string sInput, std::vector<double> &vOutput)
    {
        gss.clear();
        gss.str(sInput);
        char cFirst;
        gss >> cFirst;
        if (cFirst == '[')
            return ParseLineForVector(sInput, vOutput);
        else if ((cFirst >= '0' && cFirst <= '9') || cFirst == '-')
            return ParseLineForArray(sInput, vOutput);
        return false;
    }

    void ConvertUInt32ToUInt16s(uint32_t *src_data, int src_counts, uint16_t *dst_data, int *dst_counts)
    {
        for (int i = 0; i < src_counts; i++)
        {
            dst_data[2 * i] = (src_data[i] >> 16);
            dst_data[2 * i + 1] = (src_data[i] << 16) >> 16;
        }
        *dst_counts = 2 * src_counts;
    }

    int GetGateIPList(std::vector<std::pair<std::string, std::string>> &gateIPList)
    {
        gateIPList.clear();
        // WSADATA wsaData = {0};
        // if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
        //     return kErrorWSAStartup;
        // char buf[256];
        // gethostname(buf, sizeof(buf));
        // auto info = gethostbyname(buf);

        // std::string localIP;
        // localIP = inet_ntoa(*(struct in_addr *)*info->h_addr_list);

        IP_ADAPTER_INFO pIpAdapterInfo[10];
        unsigned long stSize = sizeof(IP_ADAPTER_INFO) * 10;
        int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
        if (nRel > 0)
            return nRel;

        auto iter = pIpAdapterInfo;
        while (iter)
        {
            std::cout << "Ip Adapter: " << iter->Description << std::endl;
            std::string sHostIP = iter->IpAddressList.IpAddress.String;
            std::string sGateIP = iter->GatewayList.IpAddress.String;
            std::cout << "Ip Address: " << sHostIP << std::endl;
            std::cout << "Network Gate: " << sGateIP << std::endl;

            if (sGateIP != "" && sGateIP != "0.0.0.0")
            {
                gateIPList.push_back({sGateIP, sHostIP});
            }

            std::cout << std::endl;
            iter = iter->Next;
        }

        return 1;
    }

    std::string GetIPPrefix()
    {
        std::vector<std::pair<std::string, std::string>> list1;
        auto rtn = GetGateIPList(list1);
        if (rtn < 1)
            return "";
        if (list1.size() < 1)
            return "";

        auto sIP = list1[0].first;
        std::stringstream ss(sIP);
        std::string sParsed, sTotal;
        std::getline(ss, sParsed, '.');
        sTotal += sParsed + ".";
        std::getline(ss, sParsed, '.');
        sTotal += sParsed + ".";
        std::getline(ss, sParsed, '.');
        sTotal += sParsed + ".";
        return sTotal;
    }
}
