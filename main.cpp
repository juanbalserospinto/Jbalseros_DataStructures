#include "StadiumLocator.h"

#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

static bool ReadDouble(const string& prompt, double* out) {
  if (!out) return false;
  cout << prompt;
  string s;
  if (!getline(cin, s)) return false;
  try {
    size_t idx = 0;
    double v = stod(s, &idx);
    (void)idx;
    *out = v;
    return true;
  } catch (...) {
    return false;
  }
}

static bool ReadSizeT(const string& prompt, size_t* out) {
  if (!out) return false;
  cout << prompt;
  string s;
  if (!getline(cin, s)) return false;
  try {
    size_t idx = 0;
    unsigned long long v = stoull(s, &idx);
    (void)idx;
    *out = static_cast<size_t>(v);
    return true;
  } catch (...) {
    return false;
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " stadiums.csv\n";
    return 1;
  }

  string csv_path = argv[1];
  StadiumLocator locator(csv_path);

  if (!locator.Load()) {
    return 1;
  }

  cout << "Loaded " << locator.Size() << " stadiums.\n";


  // Helpful coordinate examples before asking for user input
  cout << "\nIf you don't know your coordinates, you can use one of these examples:\n";
  cout << "City\t\tCountry\t\t\tLatitude\tLongitude\n";
  cout << "London\t\tUnited Kingdom\t\t51.5074\t\t-0.1278\n";
  cout << "Paris\t\tFrance\t\t\t48.8566\t\t 2.3522\n";
  cout << "Rome\t\tItaly\t\t\t41.9028\t\t12.4964\n";
  cout << "Madrid\t\tSpain\t\t\t40.4168\t\t-3.7038\n";
  cout << "Berlin\t\tGermany\t\t\t52.5200\t\t13.4050\n";
  cout << "Amsterdam\tNetherlands\t\t52.3676\t\t 4.9041\n";
  cout << "Vienna\t\tAustria\t\t\t48.2082\t\t16.3738\n";
  cout << "Athens\t\tGreece\t\t\t37.9838\t\t23.7275\n";
  cout << "Prague\t\tCzech Republic\t\t50.0755\t\t14.4378\n";
  cout << "Lisbon\t\tPortugal\t\t38.7169\t\t-9.1399\n\n";
  
  double user_lat = 0.0;
  double user_lon = 0.0;

  if (!ReadDouble("Please enter your latitude: ", &user_lat)) {
    cout << "Invalid latitude input.\n";
    return 1;
  }
  if (!ReadDouble("Please enter your longitude: ", &user_lon)) {
    cout << "Invalid longitude input.\n";
    return 1;
  }

  size_t k = 0;
  if (!ReadSizeT("To show other nearest stadiums, please enter a number for the desired number of stadium to be shown: ", &k) || k == 0) {
    cout << "Invalid K input.\n";
    return 1;
  }

  vector<StadiumResult> nearest = locator.FindKNearest(user_lat, user_lon, k);

  cout << "\nNearest " << nearest.size() << " stadium(s):\n";
  cout << fixed << setprecision(2);

  for (size_t i = 0; i < nearest.size(); i++) {
    const StadiumResult& r = nearest[i];
    cout << (i + 1) << ") " << r.record.stadium
         << " (" << r.record.team << ", " << r.record.city << ", " << r.record.country << ")\n";
    cout << "   Capacity: " << r.record.capacity << "\n";
    cout << "   Distance: " << r.distance_km << " km\n";
  }

  return 0;
}
