#include "ZaberConnectionManager.h"

using namespace zaber::motion;
using namespace zaber::motion::binary;
using namespace std;

// Connection ZaberConnectionManager::fcon;

ZaberConnectionManager::ZaberConnectionManager() : sPortNameNow(UserDefine::NO_DEVICE_FOUND), fIsOpen(false)
{
    zaber::motion::Library::enableDeviceDbStore();
}

vector<Device> ZaberConnectionManager::getDeviceList()
{
    return fDeviceList;
}

void ZaberConnectionManager::Init()
{
    fDeviceList.clear();
    fcon.close();
    fIsOpen = false;
    sPortNameNow = UserDefine::NO_DEVICE_FOUND;
}

string ZaberConnectionManager::ScanDevice()
{
    Init();

    sPortNameNow = TryFindDevice(fcon);
    if (sPortNameNow == UserDefine::NO_DEVICE_FOUND)
    {
        Init();
        return sPortNameNow;
    }

    fDeviceList = fcon.detectDevices();
    fIsOpen = true;
    return sPortNameNow;
}

bool ZaberConnectionManager::ScanPort(string portName)
{
    Init();
    bool rtn = TryFindDevice(fcon, portName, fDeviceList);
    if (!rtn)
    {
        Init();
        return rtn;
    }
    fIsOpen = true;
    sPortNameNow = portName;

    return rtn;
}

ZaberConnectionManager *&ZaberConnectionManager::Instance()
{
    static ZaberConnectionManager *currentConMan = new ZaberConnectionManager;
    return currentConMan;
}

std::vector<std::string> getComPort()
{
    HKEY hKey;
    wchar_t w_commName[256];
    LPSTR portName2[256];
    std::vector<std::string> comName;

    if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Hardware\\DeviceMap\\SerialComm"), NULL, KEY_READ, &hKey))
    {
        int i = 0;
        DWORD dwLong, dwSize;
        while (1)
        {
            dwLong = dwSize = sizeof(portName2);
            //            Enum all serial ports
            auto returnv = ::RegEnumValueA(hKey, i, LPSTR(portName2), &dwLong, NULL, NULL, (PUCHAR)w_commName, &dwSize);
            if (ERROR_NO_MORE_ITEMS == returnv)
            {
                break;
            }

            comName.push_back((char *)w_commName);
            i++;
        }
        //        Close Reg
        RegCloseKey(hKey);
    }
    else
    {
        std::cout << "您的计算机的注册表上没有HKEY_LOCAL_MACHINE:Hardware\\DeviceMap\\SerialComm项, warning"
                  << std::endl;
    }
    return comName;
}

bool TryFindDevice(Connection &con, string portName, vector<Device> &deviceList) noexcept
{
    deviceList.clear();
    try
    {
        con = Connection::openSerialPort(portName);
        cout << "Inside: Connection ID: " << con.getInterfaceId() << endl;

        deviceList = con.detectDevices();
        auto nDevice = deviceList.size();
        cout << "nDevice: " << nDevice << endl;
        if (nDevice > 0)
            return true;
    }
    catch (MotionLibException s)
    {
        std::cout << "Failed to open & find in " << portName << " :" << s.getMessage() << std::endl;
        return false;
    }

    return false;
}

bool TryFindDevice(Connection &con, string portName) noexcept
{
    vector<Device> deviceList;
    return TryFindDevice(con, portName, deviceList);
}

string TryFindDevice(Connection &con) noexcept
{
    auto portList = getComPort();
    string portNameNow = UserDefine::NO_DEVICE_FOUND;
    for (int i = 0; i < portList.size(); i++)
    {
        cout << "Try to Open: " << portList[i] << endl;
        if (TryFindDevice(con, portList[i]))
        {
            portNameNow = portList[i];
            break;
        }
    }

    return portNameNow;
}

namespace UserDefine
{
    string NO_DEVICE_FOUND = "NoDeviceFound";
}
