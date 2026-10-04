#pragma once
#include "Package.h"
#include <unordered_map>
#include <vector>
#include <string>

class DeliveryTracker {
public:
    explicit DeliveryTracker(const std::string& dataFile = "packages.csv");

    // Core operations
    bool        addPackage(const Package& pkg);
    bool        updateStatus(const std::string& id,
                             DeliveryStatus newStatus,
                             const std::string& location,
                             const std::string& note = "");
    Package*    findPackage(const std::string& id);

    // Queries
    std::vector<Package*> searchBySender(const std::string& sender);
    std::vector<Package*> searchByReceiver(const std::string& receiver);
    std::vector<Package*> filterByStatus(DeliveryStatus status);
    std::vector<Package*> getAllPackages();

    // Persistence
    bool loadFromFile();
    bool saveToFile() const;

    // Statistics
    void printSummary() const;

private:
    std::string dataFile_;
    std::unordered_map<std::string, Package> packages_;

    static std::string escapeCsv(const std::string& s);
    static std::string unescapeCsv(const std::string& s);
    static std::string encodeHistory(const std::vector<TrackingEvent>& history);
    static std::vector<TrackingEvent> decodeHistory(const std::string& encoded);
};
