#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <algorithm>

// --- GEOGRAPHIC UTILITIES ---
struct Location {
    double latitude;
    double longitude;

    // Haversine formula to compute ground distance in kilometers
    double distanceTo(const Location& other) const {
        constexpr double EARTH_RADIUS_KM = 6371.0;
        double lat1Rad = latitude * M_PI / 180.0;
        double lat2Rad = other.latitude * M_PI / 180.0;
        double deltaLat = (other.latitude - latitude) * M_PI / 180.0;
        double deltaLon = (other.longitude - longitude) * M_PI / 180.0;

        double a = std::sin(deltaLat / 2.0) * std::sin(deltaLat / 2.0) +
                   std::cos(lat1Rad) * std::cos(lat2Rad) *
                   std::sin(deltaLon / 2.0) * std::sin(deltaLon / 2.0);

        double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
        return EARTH_RADIUS_KM * c;
    }
};

// --- DATA STRUCTURES ---
enum class Transmission { MANUAL, AUTOMATIC };

struct DriverLicense {
    std::string licenseNumber;
    std::string fullName;
    int yearsExperience;
    int violationPoints;
    bool automaticOnlyRestriction;
};

struct Car {
    std::string id;
    std::string makeModel;
    Transmission transmission;
    double baseHourlyRate;
    Location location;
    int fuelPercent;
    std::string inCarNavigationType; // e.g., "Built-in OEM GPS", "Apple CarPlay / Android Auto"
    bool isAvailable;
};

struct BookingRequest {
    int startHour;      // 0 to 23
    int durationHours;  // Rental duration
};

// --- CORE SYSTEM MODULES ---
class VerificationEngine {
public:
    static bool verifyDriver(const DriverLicense& license) {
        std::cout << "\n[OCR & Registry Scan] Validating License #" << license.licenseNumber << "...\n";
        
        if (license.violationPoints >= 6) {
            std::cout << " -> Result: REJECTED (High risk score: " << license.violationPoints << " points)\n";
            return false;
        }
        std::cout << " -> Result: VERIFIED successfully.\n";
        return true;
    }

    static Transmission recommendTransmission(const DriverLicense& license) {
        std::cout << "\n[Recommendation Engine] Evaluating driver profile...\n";
        
        if (license.automaticOnlyRestriction) {
            std::cout << " -> License Restriction Found: Automatic Only.\n";
            return Transmission::AUTOMATIC;
        }

        if (license.yearsExperience < 2) {
            std::cout << " -> Driver Experience: Less than 2 years (" << license.yearsExperience << " yrs).\n";
            std::cout << " -> Recommendation: AUTOMATIC (Optimized for safety and ease in urban traffic).\n";
            return Transmission::AUTOMATIC;
        }

        std::cout << " -> Driver Experience: " << license.yearsExperience << " years.\n";
        std::cout << " -> Recommendation: BOTH MANUAL & AUTOMATIC unlocked.\n";
        return Transmission::MANUAL;
    }
};

class PricingEngine {
public:
    static double calculateTotalPrice(double baseRate, int startHour, int durationHours) {
        double totalCost = 0.0;

        for (int h = 0; h < durationHours; ++h) {
            int currentHour = (startHour + h) % 24;
            double multiplier = 1.0;

            // Morning peak (7 AM - 9 AM) and Evening peak (5 PM - 8 PM)
            if ((currentHour >= 7 && currentHour <= 9) || (currentHour >= 17 && currentHour <= 20)) {
                multiplier = 1.4; // 40% peak multiplier
            } 
            // Off-peak late night (11 PM - 5 AM)
            else if (currentHour >= 23 || currentHour <= 5) {
                multiplier = 0.8; // 20% night discount
            }

            totalCost += baseRate * multiplier;
        }
        return totalCost;
    }
};

class CarSharingPlatform {
private:
    std::vector<Car> fleet;

public:
    void addCar(const Car& car) {
        fleet.push_back(car);
    }

    void displayNearbyCars(const Location& userLoc, Transmission recommendedTrans, const BookingRequest& booking) const {
        std::cout << "\n========================================================================\n";
        std::cout << "                        AVAILABLE CARS NEARBY                           \n";
        std::cout << "========================================================================\n";

        for (const auto& car : fleet) {
            if (!car.isAvailable) continue;

            double dist = car.location.distanceTo(userLoc);
            double totalPrice = PricingEngine::calculateTotalPrice(car.baseHourlyRate, booking.startHour, booking.durationHours);
            
            std::string transStr = (car.transmission == Transmission::AUTOMATIC) ? "AUTOMATIC" : "MANUAL";
            std::string recTag = (car.transmission == recommendedTrans) ? " [RECOMMENDED FOR YOU]" : "";

            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Car ID:          " << car.id << recTag << "\n";
            std::cout << "Model:           " << car.makeModel << "\n";
            std::cout << "Transmission:    " << transStr << "\n";
            std::cout << "Distance:        " << dist << " km away\n";
            std::cout << "Fuel/Battery:    " << car.fuelPercent << "%\n";
            std::cout << "In-Car Map System: " << car.inCarNavigationType << "\n";
            std::cout << "Base Rate:       $" << car.baseHourlyRate << "/hr\n";
            std::cout << "Total Price (" << booking.durationHours << " hrs @ " << booking.startHour << ":00 start): $" << totalPrice << "\n";
            std::cout << "------------------------------------------------------------------------\n";
        }
    }

    bool unlockAndStartTrip(const std::string& carId, const Location& userDestination) {
        for (auto& car : fleet) {
            if (car.id == carId && car.isAvailable) {
                car.isAvailable = false;
                std::cout << "\n[Telematics API] Authenticating BLE Keyless Unlock for " << car.makeModel << "...\n";
                std::cout << "[Telematics API] Vehicle unlocked!\n";
                std::cout << "[In-Car Sync] Syncing target route (" 
                          << userDestination.latitude << ", " << userDestination.longitude 
                          << ") to " << car.inCarNavigationType << "...\n";
                std::cout << "[Engine Control] Immobilizer deactivated. You are clear to drive!\n";
                return true;
            }
        }
        std::cout << "Vehicle unavailable or invalid ID.\n";
        return false;
    }
};

// --- MAIN EXECUTION ---
int main() {
    CarSharingPlatform app;

    // Populate Fleet
    app.addCar({"CAR-01", "Volkswagen Polo", Transmission::MANUAL, 12.0, {-1.28638, 36.81722}, 85, "Built-in OEM Touchscreen GPS", true});
    app.addCar({"CAR-02", "Toyota Yaris", Transmission::AUTOMATIC, 15.0, {-1.28800, 36.82000}, 92, "Apple CarPlay & Android Auto", true});
    app.addCar({"CAR-03", "Mazda Demio", Transmission::AUTOMATIC, 14.0, {-1.29100, 36.81500}, 45, "Android Auto Display", true});

    // User State
    Location userPos = {-1.28500, 36.82100}; // Current user position
    Location destinationPos = {-1.30000, 36.85000}; // Destination coordinates

    DriverLicense userLicense = {
        "DL-89472910",
        "Hannah M.",
        1,       // 1 year experience -> triggering Automatic recommendation
        0,       // 0 violation points -> Passed
        false    // No manual restrictions
    };

    std::cout << "========================================================================\n";
    std::cout << "              ON-DEMAND SELF-DRIVE CAR SHARING PLATFORM                 \n";
    std::cout << "========================================================================\n";

    // 1. Scan and verify license
    if (!VerificationEngine::verifyDriver(userLicense)) {
        std::cout << "Verification failed. Unable to proceed.\n";
        return 0;
    }

    // 2. Recommend transmission based on experience
    Transmission recommendedTrans = VerificationEngine::recommendTransmission(userLicense);

    // 3. Define trip details: Pickup at 8:00 AM (Peak time), Duration: 3 hours
    BookingRequest booking = {8, 3};

    // 4. Search and present nearby cars
    app.displayNearbyCars(userPos, recommendedTrans, booking);

    // 5. Select and unlock vehicle
    std::string selectedCarId = "CAR-02";
    std::cout << "\nSelecting " << selectedCarId << " for booking...\n";
    app.unlockAndStartTrip(selectedCarId, destinationPos);

    return 0;
}
