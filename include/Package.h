#pragma once
#include <string>
#include <vector>
#include <ctime>

enum class DeliveryStatus {
    PENDING,
    PICKED_UP,
    IN_TRANSIT,
    OUT_FOR_DELIVERY,
    DELIVERED,
    FAILED,
    RETURNED
};

struct TrackingEvent {
    std::string timestamp;
    DeliveryStatus status;
    std::string location;
    std::string note;
};

class Package {
public:
    Package() = default;
    Package(const std::string& id,
            const std::string& sender,
            const std::string& receiver,
            const std::string& origin,
            const std::string& destination,
            double weightKg);

    // Getters
    const std::string& getId()          const { return id_; }
    const std::string& getSender()      const { return sender_; }
    const std::string& getReceiver()    const { return receiver_; }
    const std::string& getOrigin()      const { return origin_; }
    const std::string& getDestination() const { return destination_; }
    double             getWeight()      const { return weightKg_; }
    DeliveryStatus     getStatus()      const { return status_; }
    const std::string& getCreatedAt()   const { return createdAt_; }
    const std::vector<TrackingEvent>& getHistory() const { return history_; }

    // Mutators
    void updateStatus(DeliveryStatus newStatus,
                      const std::string& location,
                      const std::string& note = "");

    // Used only by DeliveryTracker when loading from file
    void restoreFromHistory(const std::vector<TrackingEvent>& history,
                            const std::string& createdAt);

    // Utilities
    static std::string statusToString(DeliveryStatus s);
    static DeliveryStatus stringToStatus(const std::string& s);
    static std::string currentTimestamp();

private:
    std::string    id_;
    std::string    sender_;
    std::string    receiver_;
    std::string    origin_;
    std::string    destination_;
    double         weightKg_ = 0.0;
    DeliveryStatus status_   = DeliveryStatus::PENDING;
    std::string    createdAt_;
    std::vector<TrackingEvent> history_;
};
