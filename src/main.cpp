#include "DeliveryTracker.h"
#include "UI.h"
#include <iostream>

int main() {
    try {
        DeliveryTracker tracker("packages.csv");
        UI ui(tracker);
        ui.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}