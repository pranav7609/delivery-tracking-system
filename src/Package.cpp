#include "Package.h"
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <ctime>

Package::Package(const std::string& id,
                 const std::string& sender,
                 const std::string& receiver,
                 const std::string& origin,
                 const std::string& destination,
                 double weightKg)
    : id_(id), sender_(sender), receiver_(receiver),
      origin_(origin), destination_(destination),
      weightKg_(weightKg), status_(DeliveryStatus::PENDING),
      createdAt_(currentTimestamp())
{
    // Record the initial creation event
    TrackingEvent ev;
    ev.timestamp = createdAt_;
    ev.status    = DeliveryStatus::PENDING;
    ev.location  = origin_;
    ev.note      = "Package registered.";
    history_.push_back(ev);
}

void Package::updateStatus(DeliveryStatus newStatus,
                           const std::string& location,
                           const std::string& note)
{
    status_ = newStatus;
    TrackingEvent ev;
    ev.timestamp = currentTimestamp();
    ev.status    = newStatus;
    ev.location  = location;
    ev.note      = note.empty() ? statusToString(newStatus) : note;
    history_.push_back(ev);
}

void Package::restoreFromHistory(const std::vector<TrackingEvent>& history,
                                 const std::string& createdAt)
{
    history_   = history;
    createdAt_ = createdAt;
    if (!history.empty()) {
        status_ = history.back().status;
    }
}

std::string Package::statusToString(DeliveryStatus s) {
    switch (s) {
        case DeliveryStatus::PENDING:            return "Pending";
        case DeliveryStatus::PICKED_UP:          return "Picked Up";
        case DeliveryStatus::IN_TRANSIT:         return "In Transit";
        case DeliveryStatus::OUT_FOR_DELIVERY:   return "Out for Delivery";
        case DeliveryStatus::DELIVERED:          return "Delivered";
        case DeliveryStatus::FAILED:             return "Failed";
        case DeliveryStatus::RETURNED:           return "Returned";
        default:                                 return "Unknown";
    }
}

DeliveryStatus Package::stringToStatus(const std::string& s) {
    if (s == "Pending")            return DeliveryStatus::PENDING;
    if (s == "Picked Up")          return DeliveryStatus::PICKED_UP;
    if (s == "In Transit")         return DeliveryStatus::IN_TRANSIT;
    if (s == "Out for Delivery")   return DeliveryStatus::OUT_FOR_DELIVERY;
    if (s == "Delivered")          return DeliveryStatus::DELIVERED;
    if (s == "Failed")             return DeliveryStatus::FAILED;
    if (s == "Returned")           return DeliveryStatus::RETURNED;
    throw std::invalid_argument("Unknown status: " + s);
}

std::string Package::currentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* localTm = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(localTm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
