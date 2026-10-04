#pragma once
#include "DeliveryTracker.h"

class UI {
public:
    explicit UI(DeliveryTracker& tracker);
    void run();

private:
    DeliveryTracker& tracker_;

    void showMainMenu();
    void handleAddPackage();
    void handleUpdateStatus();
    void handleTrackPackage();
    void handleSearchPackages();
    void handleListByStatus();
    void handleListAll();
    void handleSummary();

    static void printPackageBrief(const Package& pkg);
    static void printPackageDetail(const Package& pkg);
    static void printDivider(char c = '-', int width = 60);
    static std::string prompt(const std::string& label);
    static int         promptInt(const std::string& label, int min, int max);
    static double      promptDouble(const std::string& label);
    static DeliveryStatus pickStatus(const std::string& label);
    static void clearScreen();
    static void pauseScreen();
};
