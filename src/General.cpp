
#include "General.h"


// C++
#include <iostream>
#include <string>
#include <sstream>

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
}
