#include "StadiumLocator.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

static bool ReadDouble(const std::string& prompt, double& out) {
  while (true) {
    std::cout << prompt;
    if (std::cin >> out) return true;

    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Invalid number. Try again.\n";
  }
}

static std::size_t ReadSizeT(const std::string& prompt, std::size_t min_val, std::size_t max_val) {
  while (true) {
    std::cout << prompt;
    long long temp = 0;
    if (std::cin >> temp) {
      if (temp >= static_cast<long long>(min_val) && temp <= static_cast<long long>(max_val)) {
        return static_cast<std::size_t>(temp);
      }
      std::cout << "Please enter a number between " << min_val << " and " << max_val << ".\n";
    } else {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Invalid number. Try again.\n";
    }
  }
}

static void PrintStadium(const StadiumDistance& sd) {
  std::cout << "Team:     " << sd.stadium.team << "\n";
  std::cout << "City:     " << sd.stadium.city << "\n";
  std::cout << "Stadium:  " << sd.stadium.stadium << "\n";
  std::cout << "Country:  " << sd.stadium.country << "\n";
  std::cout << "Capacity: " << sd.stadium.capacity << "\n";
  std::cout << "Coords:   (" << sd.stadium.latitude << ", " << sd.stadium.longitude << ")\n";
  std::cout << "Distance: " << std::fixed << std::setprecision(3) << sd.distance_km << " km\n";
}

int main(int argc, char** argv) {
  std::string csv_path = "stadiums.csv";
  if (argc >= 2) {
    csv_path = argv[1];
  }

  StadiumLocator locator;
  if (!locator.LoadFromCSV(csv_path)) {
    std::cerr << "Make sure your CSV exists and has columns:\n";
    std::cerr << "Team,City,Stadium,Capacity,Latitude,Longitude,Country\n";
    return 1;
  }

  std::cout << "Loaded " << locator.Count() << " stadium(s) from: " << csv_path << "\n\n";

  while (true) {
    double user_lat = 0.0;
    double user_lon = 0.0;

    ReadDouble("Enter your latitude:  ", user_lat);
    ReadDouble("Enter your longitude: ", user_lon);

    StadiumDistance nearest;
    if (!locator.FindNearest(user_lat, user_lon, nearest)) {
      std::cerr << "Error: No stadiums loaded.\n";
      return 1;
    }

    std::cout << "\nNearest stadium:\n";
    std::cout << "----------------\n";
    PrintStadium(nearest);

    // Requirement: ask K AFTER showing the nearest stadium.
    std::size_t k_max = locator.Count();
    std::size_t k = ReadSizeT("\nEnter K (how many nearest stadiums to list): ", 1, k_max);

    std::vector<StadiumDistance> knn = locator.FindKNearest(user_lat, user_lon, k);

    std::cout << "\nTop " << knn.size() << " nearest stadium(s):\n";
    std::cout << "---------------------------\n";
    for (std::size_t i = 0; i < knn.size(); ++i) {
      const StadiumDistance& sd = knn[i];
      std::cout << (i + 1) << ") "
                << sd.stadium.stadium << " — " << sd.stadium.team
                << " (" << sd.stadium.city << ", " << sd.stadium.country << ")"
                << " -> " << std::fixed << std::setprecision(3) << sd.distance_km << " km\n";
    }

    std::cout << "\nRun again? (y/n): ";
    std::string ans;
    std::cin >> ans;
    if (ans != "y" && ans != "Y") break;

    std::cout << "\n";
  }

  std::cout << "Goodbye.\n";
  return 0;
}