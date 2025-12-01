#ifndef STADIUM_LOCATOR_H
#define STADIUM_LOCATOR_H

#include <string>
#include <vector>

struct Stadium {
  std::string team;
  std::string city;
  std::string stadium;
  long long capacity;
  double latitude;
  double longitude;
  std::string country;
};

struct StadiumDistance {
  Stadium stadium;
  double distance_km;
};

class StadiumLocator {
public:
  bool LoadFromCSV(const std::string& csv_path);
  std::size_t Count() const;

  // Finds the single nearest stadium (distance in km).
  bool FindNearest(double user_lat, double user_lon, StadiumDistance& out) const;

  // Finds the K nearest stadiums (sorted ascending by distance).
  // If k > number of stadiums, it returns all.
  std::vector<StadiumDistance> FindKNearest(double user_lat, double user_lon, std::size_t k) const;

private:
  std::vector<Stadium> stadiums_;

  static std::vector<std::string> SplitCSVLine(const std::string& line);
  static std::string Trim(const std::string& s);
  static bool ToLongLong(const std::string& s, long long& out);
  static bool ToDouble(const std::string& s, double& out);

  static double HaversineKm(double lat1, double lon1, double lat2, double lon2);
};

#endif