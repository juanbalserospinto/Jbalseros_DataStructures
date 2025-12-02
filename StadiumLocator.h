#ifndef STADIUMLOCATOR_H
#define STADIUMLOCATOR_H

#include <string>
#include <vector>

using namespace std;

struct StadiumRecord {
  string team;
  string city;
  string stadium;
  long long capacity;
  double latitude;
  double longitude;
  string country;
};

struct StadiumResult {
  StadiumRecord record;
  double distance_km;
};

class StadiumLocator {
 public:
  explicit StadiumLocator(const string& csv_path);

  bool Load();
  size_t Size() const;

  vector<StadiumResult> FindKNearest(double user_lat, double user_lon, size_t k) const;

 private:
  static bool ParseLineCSV(const string& line, vector<string>* fields);
  static string Trim(const string& s);
  static bool ToLongLong(const string& s, long long* out);
  static bool ToDouble(const string& s, double* out);

  static double DegToRad(double deg);
  static double HaversineKm(double lat1, double lon1, double lat2, double lon2);

  string csv_path_;
  vector<StadiumRecord> records_;
};

#endif
