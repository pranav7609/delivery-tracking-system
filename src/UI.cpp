#include "UI.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <random>

#ifdef _WIN32
#include <windows.h>
#endif

// ──────────────────────────────────────────────
//  Construction
// ──────────────────────────────────────────────
UI::UI(DeliveryTracker& tracker) : tracker_(tracker) {}

// ──────────────────────────────────────────────
//  Main loop
// ──────────────────────────────────────────────
void UI::run() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::cout << "\n========================================\n";
    std::cout << "   DELIVERY TRACKING SYSTEM\n";
    std::cout << "========================================\n";

    bool running = true;
    while (running) {
        showMainMenu();
        int choice = promptInt("Choice", 0, 7);
        switch (choice) {
            case 1: handleAddPackage();     break;
            case 2: handleUpdateStatus();   break;
            case 3: handleTrackPackage();   break;
            case 4: handleSearchPackages(); break;
            case 5: handleListByStatus();   break;
            case 6: handleListAll();        break;
            case 7: handleSummary();        break;
            case 0:
                std::cout << "\nGoodbye!\n";
                running = false;
                break;
        }
    }
}

// ──────────────────────────────────────────────
//  Menu
// ──────────────────────────────────────────────
void UI::showMainMenu() {
    std::cout << "\n";
    printDivider('=');
    std::cout << "  MAIN MENU\n";
    printDivider('=');
    std::cout << "  1. Add new package\n";
    std::cout << "  2. Update package status\n";
    std::cout << "  3. Track a package\n";
    std::cout << "  4. Search packages\n";
    std::cout << "  5. List by status\n";
    std::cout << "  6. List all packages\n";
    std::cout << "  7. Summary / statistics\n";
    std::cout << "  0. Exit\n";
    printDivider();
}

// ──────────────────────────────────────────────
//  Handlers
// ──────────────────────────────────────────────
void UI::handleAddPackage() {
    printDivider('=');
    std::cout << "  ADD NEW PACKAGE\n";
    printDivider();

    // Auto-generate a tracking ID
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(100000, 999999);
    std::string id = "PKG-" + std::to_string(dist(rng));
    std::cout << "  Generated Tracking ID: " << id << "\n\n";

    std::string sender      = prompt("Sender name");
    std::string receiver    = prompt("Receiver name");
    std::string origin      = prompt("Origin city");
    std::string destination = prompt("Destination city");
    double      weight      = promptDouble("Weight (kg)");

    Package pkg(id, sender, receiver, origin, destination, weight);
    if (tracker_.addPackage(pkg)) {
        std::cout << "\n  [OK] Package added successfully.\n";
        std::cout << "  Tracking ID: " << id << "\n";
    } else {
        std::cout << "\n  [ERROR] A package with that ID already exists.\n";
    }
    pauseScreen();
}

void UI::handleUpdateStatus() {
    printDivider('=');
    std::cout << "  UPDATE PACKAGE STATUS\n";
    printDivider();

    // Admin key check
    std::cout << "  Enter admin access key: ";
    std::string key;
    std::getline(std::cin, key);
    if (key != "12345") {
        std::cout << "\n  [DENIED] Incorrect admin key.\n";
        pauseScreen();
        return;
    }
    std::cout << "  [OK] Access granted.\n\n";

    std::string id       = prompt("Tracking ID");
    Package* pkg         = tracker_.findPackage(id);
    if (!pkg) {
        std::cout << "\n  [ERROR] Package not found.\n";
        pauseScreen();
        return;
    }

    std::cout << "\n  Current status: "
              << Package::statusToString(pkg->getStatus()) << "\n\n";

    DeliveryStatus newStatus = pickStatus("New status");
    std::string location     = prompt("Current location");
    std::string note         = prompt("Note (optional, press Enter to skip)");

    tracker_.updateStatus(id, newStatus, location, note);
    std::cout << "\n  [OK] Status updated.\n";
    pauseScreen();
}

void UI::handleTrackPackage() {
    printDivider('=');
    std::cout << "  TRACK PACKAGE\n";
    printDivider();

    std::string id = prompt("Tracking ID");
    Package* pkg   = tracker_.findPackage(id);
    if (!pkg) {
        std::cout << "\n  [ERROR] Package not found.\n";
    } else {
        printPackageDetail(*pkg);
    }
    pauseScreen();
}

void UI::handleSearchPackages() {
    printDivider('=');
    std::cout << "  SEARCH PACKAGES\n";
    printDivider();
    std::cout << "  1. Search by sender\n";
    std::cout << "  2. Search by receiver\n";
    int choice = promptInt("Option", 1, 2);

    std::string query = prompt("Search term");
    std::vector<Package*> results;
    if (choice == 1) results = tracker_.searchBySender(query);
    else             results = tracker_.searchByReceiver(query);

    std::cout << "\n  Found " << results.size() << " result(s):\n";
    printDivider();
    for (Package* p : results) printPackageBrief(*p);
    pauseScreen();
}

void UI::handleListByStatus() {
    printDivider('=');
    std::cout << "  LIST BY STATUS\n";
    printDivider();
    DeliveryStatus status = pickStatus("Filter status");
    auto results = tracker_.filterByStatus(status);
    std::cout << "\n  Packages with status ["
              << Package::statusToString(status) << "]: "
              << results.size() << "\n";
    printDivider();
    for (Package* p : results) printPackageBrief(*p);
    pauseScreen();
}

void UI::handleListAll() {
    printDivider('=');
    std::cout << "  ALL PACKAGES\n";
    printDivider();
    auto all = tracker_.getAllPackages();
    if (all.empty()) {
        std::cout << "  No packages on record.\n";
    } else {
        std::cout << "  " << std::left
                  << std::setw(14) << "ID"
                  << std::setw(18) << "Sender"
                  << std::setw(18) << "Receiver"
                  << std::setw(20) << "Status"
                  << "Weight(kg)\n";
        printDivider();
        for (Package* p : all) {
            std::cout << "  " << std::left
                      << std::setw(14) << p->getId()
                      << std::setw(18) << p->getSender().substr(0, 16)
                      << std::setw(18) << p->getReceiver().substr(0, 16)
                      << std::setw(20) << Package::statusToString(p->getStatus())
                      << std::fixed << std::setprecision(2) << p->getWeight()
                      << "\n";
        }
    }
    pauseScreen();
}

void UI::handleSummary() {
    printDivider('=');
    std::cout << "  SUMMARY\n";
    printDivider();
    tracker_.printSummary();
    pauseScreen();
}

// ──────────────────────────────────────────────
//  Print helpers
// ──────────────────────────────────────────────
void UI::printPackageBrief(const Package& pkg) {
    std::cout << "  " << std::left
              << std::setw(14) << pkg.getId()
              << " | " << std::setw(14) << pkg.getSender().substr(0, 12)
              << " -> " << std::setw(14) << pkg.getReceiver().substr(0, 12)
              << " | " << Package::statusToString(pkg.getStatus()) << "\n";
}

void UI::printPackageDetail(const Package& pkg) {
    printDivider('=');
    std::cout << "  TRACKING DETAILS\n";
    printDivider();
    std::cout << "  Tracking ID   : " << pkg.getId()          << "\n";
    std::cout << "  Sender        : " << pkg.getSender()       << "\n";
    std::cout << "  Receiver      : " << pkg.getReceiver()     << "\n";
    std::cout << "  Origin        : " << pkg.getOrigin()       << "\n";
    std::cout << "  Destination   : " << pkg.getDestination()  << "\n";
    std::cout << "  Weight        : " << std::fixed << std::setprecision(2)
              << pkg.getWeight() << " kg\n";
    std::cout << "  Created At    : " << pkg.getCreatedAt()    << "\n";
    std::cout << "  Status        : " << Package::statusToString(pkg.getStatus()) << "\n";

    std::cout << "\n  --- Tracking History ---\n";
    const auto& hist = pkg.getHistory();
    for (size_t i = 0; i < hist.size(); ++i) {
        const auto& ev = hist[i];
        std::cout << "  [" << (i + 1) << "] "
                  << ev.timestamp << "  "
                  << std::left << std::setw(20)
                  << Package::statusToString(ev.status)
                  << " @ " << ev.location;
        if (!ev.note.empty() && ev.note != Package::statusToString(ev.status))
            std::cout << "  (" << ev.note << ")";
        std::cout << "\n";
    }
    printDivider();
}

// ──────────────────────────────────────────────
//  Input helpers
// ──────────────────────────────────────────────
void UI::printDivider(char c, int width) {
    std::cout << "  " << std::string(width, c) << "\n";
}

std::string UI::prompt(const std::string& label) {
    std::string input;
    std::cout << "  " << label << ": ";
    std::getline(std::cin, input);
    return input;
}

int UI::promptInt(const std::string& label, int minVal, int maxVal) {
    int val = 0;
    while (true) {
        std::cout << "  " << label << " [" << minVal << "-" << maxVal << "]: ";
        std::string line;
        std::getline(std::cin, line);
        try {
            val = std::stoi(line);
            if (val >= minVal && val <= maxVal) return val;
        } catch (...) {}
        std::cout << "  Please enter a number between "
                  << minVal << " and " << maxVal << ".\n";
    }
}

double UI::promptDouble(const std::string& label) {
    while (true) {
        std::cout << "  " << label << ": ";
        std::string line;
        std::getline(std::cin, line);
        try {
            double v = std::stod(line);
            if (v > 0) return v;
        } catch (...) {}
        std::cout << "  Please enter a positive number.\n";
    }
}

DeliveryStatus UI::pickStatus(const std::string& label) {
    std::cout << "\n  " << label << ":\n";
    std::cout << "    1. Pending\n";
    std::cout << "    2. Picked Up\n";
    std::cout << "    3. In Transit\n";
    std::cout << "    4. Out for Delivery\n";
    std::cout << "    5. Delivered\n";
    std::cout << "    6. Failed\n";
    std::cout << "    7. Returned\n";
    int choice = promptInt("  Select", 1, 7);
    switch (choice) {
        case 1: return DeliveryStatus::PENDING;
        case 2: return DeliveryStatus::PICKED_UP;
        case 3: return DeliveryStatus::IN_TRANSIT;
        case 4: return DeliveryStatus::OUT_FOR_DELIVERY;
        case 5: return DeliveryStatus::DELIVERED;
        case 6: return DeliveryStatus::FAILED;
        case 7: return DeliveryStatus::RETURNED;
        default: return DeliveryStatus::PENDING;
    }
}

void UI::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void UI::pauseScreen() {
    std::cout << "\n  Press Enter to continue...";
    std::string dummy;
    std::getline(std::cin, dummy);
}
