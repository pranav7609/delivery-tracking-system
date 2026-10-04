#include "DeliveryTracker.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

// ──────────────────────────────────────────────
//  Construction
// ──────────────────────────────────────────────
DeliveryTracker::DeliveryTracker(const std::string& dataFile)
    : dataFile_(dataFile)
{
    loadFromFile();
}

// ──────────────────────────────────────────────
//  Core operations
// ──────────────────────────────────────────────
bool DeliveryTracker::addPackage(const Package& pkg) {
    if (packages_.count(pkg.getId())) return false;   // duplicate
    packages_.emplace(pkg.getId(), pkg);
    saveToFile();
    return true;
}

bool DeliveryTracker::updateStatus(const std::string& id,
                                   DeliveryStatus newStatus,
                                   const std::string& location,
                                   const std::string& note)
{
    auto it = packages_.find(id);
    if (it == packages_.end()) return false;
    it->second.updateStatus(newStatus, location, note);
    saveToFile();
    return true;
}

Package* DeliveryTracker::findPackage(const std::string& id) {
    auto it = packages_.find(id);
    return (it == packages_.end()) ? nullptr : &it->second;
}

// ──────────────────────────────────────────────
//  Queries
// ──────────────────────────────────────────────
std::vector<Package*> DeliveryTracker::searchBySender(const std::string& sender) {
    std::vector<Package*> result;
    for (auto& kv : packages_) {
        std::string s = kv.second.getSender(), q = sender;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        std::transform(q.begin(), q.end(), q.begin(), ::tolower);
        if (s.find(q) != std::string::npos) result.push_back(&kv.second);
    }
    return result;
}

std::vector<Package*> DeliveryTracker::searchByReceiver(const std::string& receiver) {
    std::vector<Package*> result;
    for (auto& kv : packages_) {
        std::string s = kv.second.getReceiver(), q = receiver;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        std::transform(q.begin(), q.end(), q.begin(), ::tolower);
        if (s.find(q) != std::string::npos) result.push_back(&kv.second);
    }
    return result;
}

std::vector<Package*> DeliveryTracker::filterByStatus(DeliveryStatus status) {
    std::vector<Package*> result;
    for (auto& kv : packages_)
        if (kv.second.getStatus() == status) result.push_back(&kv.second);
    return result;
}

std::vector<Package*> DeliveryTracker::getAllPackages() {
    std::vector<Package*> result;
    result.reserve(packages_.size());
    for (auto& kv : packages_) result.push_back(&kv.second);
    return result;
}

// ──────────────────────────────────────────────
//  Statistics
// ──────────────────────────────────────────────
void DeliveryTracker::printSummary() const {
    std::cout << "\n  Total packages  : " << packages_.size() << "\n";

    int counts[7] = {};
    double totalWeight = 0.0;
    for (auto& kv : packages_) {
        counts[static_cast<int>(kv.second.getStatus())]++;
        totalWeight += kv.second.getWeight();
    }

    std::cout << "  Total weight    : " << std::fixed << std::setprecision(2)
              << totalWeight << " kg\n\n";

    const char* labels[] = {
        "Pending", "Picked Up", "In Transit",
        "Out for Delivery", "Delivered", "Failed", "Returned"
    };
    std::cout << "  Status breakdown:\n";
    for (int i = 0; i < 7; ++i) {
        if (counts[i])
            std::cout << "    " << std::left << std::setw(20)
                      << labels[i] << ": " << counts[i] << "\n";
    }
}

// ──────────────────────────────────────────────
//  CSV helpers
// ──────────────────────────────────────────────
std::string DeliveryTracker::escapeCsv(const std::string& s) {
    // Wrap in quotes and double any internal quotes
    std::string out = "\"";
    for (char c : s) {
        if (c == '"') out += '"';
        out += c;
    }
    out += '"';
    return out;
}

std::string DeliveryTracker::unescapeCsv(const std::string& s) {
    if (s.size() < 2 || s.front() != '"' || s.back() != '"') return s;
    std::string out;
    for (size_t i = 1; i + 1 < s.size(); ++i) {
        if (s[i] == '"' && s[i + 1] == '"') { out += '"'; ++i; }
        else out += s[i];
    }
    return out;
}

// History encoded as pipe-separated tuples:
// timestamp|status|location|note ; ...
std::string DeliveryTracker::encodeHistory(const std::vector<TrackingEvent>& history) {
    std::string out;
    for (size_t i = 0; i < history.size(); ++i) {
        const auto& ev = history[i];
        if (i) out += ";";
        out += ev.timestamp + "|"
             + Package::statusToString(ev.status) + "|"
             + ev.location + "|"
             + ev.note;
    }
    return out;
}

std::vector<TrackingEvent> DeliveryTracker::decodeHistory(const std::string& encoded) {
    std::vector<TrackingEvent> history;
    if (encoded.empty()) return history;
    std::istringstream ss(encoded);
    std::string chunk;
    while (std::getline(ss, chunk, ';')) {
        std::istringstream cs(chunk);
        std::string ts, st, loc, note;
        if (!std::getline(cs, ts,  '|')) continue;
        if (!std::getline(cs, st,  '|')) continue;
        if (!std::getline(cs, loc, '|')) continue;
        std::getline(cs, note);   // rest is the note
        TrackingEvent ev;
        ev.timestamp = ts;
        try { ev.status = Package::stringToStatus(st); }
        catch (...) { ev.status = DeliveryStatus::PENDING; }
        ev.location  = loc;
        ev.note      = note;
        history.push_back(ev);
    }
    return history;
}

// ──────────────────────────────────────────────
//  Persistence  (CSV format)
//  Columns: id, sender, receiver, origin, destination,
//           weight, status, createdAt, history
// ──────────────────────────────────────────────
bool DeliveryTracker::saveToFile() const {
    std::ofstream ofs(dataFile_);
    if (!ofs) { std::cerr << "Warning: cannot write " << dataFile_ << "\n"; return false; }

    // Header
    ofs << "id,sender,receiver,origin,destination,weight,status,createdAt,history\n";

    for (auto& kv : packages_) {
        const Package& pkg = kv.second;
        ofs << escapeCsv(pkg.getId())          << ","
            << escapeCsv(pkg.getSender())       << ","
            << escapeCsv(pkg.getReceiver())     << ","
            << escapeCsv(pkg.getOrigin())       << ","
            << escapeCsv(pkg.getDestination())  << ","
            << pkg.getWeight()                  << ","
            << escapeCsv(Package::statusToString(pkg.getStatus())) << ","
            << escapeCsv(pkg.getCreatedAt())    << ","
            << escapeCsv(encodeHistory(pkg.getHistory()))
            << "\n";
    }
    return true;
}

// Simple CSV tokenizer respecting quoted fields
static std::vector<std::string> parseCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') { field += '"'; ++i; }
                else inQuotes = false;
            } else field += c;
        } else {
            if (c == '"') { inQuotes = true; }
            else if (c == ',') { fields.push_back(field); field.clear(); }
            else field += c;
        }
    }
    fields.push_back(field);
    return fields;
}

bool DeliveryTracker::loadFromFile() {
    std::ifstream ifs(dataFile_);
    if (!ifs) return false;   // file may not exist yet

    std::string line;
    bool firstLine = true;
    while (std::getline(ifs, line)) {
        if (firstLine) { firstLine = false; continue; }   // skip header
        if (line.empty()) continue;

        auto f = parseCsvLine(line);
        if (f.size() < 9) continue;

        // Rebuild Package manually (bypasses constructor side-effects)
        // We use a private-style reconstruction via update calls
        try {
            Package pkg(f[0], f[1], f[2], f[3], f[4], std::stod(f[5]));
            auto savedHistory = decodeHistory(f[8]);
            // Restore the exact history (with original timestamps) and
            // set createdAt from the saved field rather than re-calling
            // updateStatus (which would generate new timestamps).
            if (!savedHistory.empty())
                pkg.restoreFromHistory(savedHistory, f[7]);
            packages_.emplace(pkg.getId(), std::move(pkg));
        } catch (...) {
            // Skip malformed lines
        }
    }
    return true;
}
