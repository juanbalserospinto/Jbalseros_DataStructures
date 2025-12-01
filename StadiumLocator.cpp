#include "StadiumLocator.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <utility>

std::size_t StadiumLocator::Count() const {
  return stadiums_.size();
}

std::string StadiumLocator::Trim(const std::string& s) {
  std::size_t i = 0;
  while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])) != 0) {
    ++i;
  }
  std::size_t j = s.size();
  while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1])) != 0) {
    --j;
  }
  return s.substr(i, j - i);
}

std::vector<std::string> StadiumLocator::SplitCSVLine(const std::string& line) {
  // Supports quoted fields and commas inside quotes.
  std::vector<std::string> out;
  std::string cur;
  bool in_quotes = false;

  for (std::size_t i = 0; i < line.size(); ++i) {
    char c = line[i];
    if (c == '"') {
      // If we see "" while in quotes, that's an escaped quote.
      if (in_quotes && (i + 1) < line.size() && line[i + 1] == '"') {
        cur.push_back('"');
        ++i;
      } else {
        in_quotes = !in_quotes;
      }
    } else if (c == ',' && !in_quotes) {
      out.push_back(Trim(cur));
      cur.clear();
    } else {
      cur.push_back(c);
    }
  }

  out.push_back(Trim(cur));

  // Strip surrounding quotes if present.
  for (std::size_t k = 0; k < out.size(); ++k) {
    std::string f = out[k];
    if (f.size() >= 2 && f.front() == '"' && f.back() == '"') {
      out[k] = f.substr(1, f.size() - 2);
    }
  }

  return out;
}

bool StadiumLocator::ToLongLong(const std::string& s, long long& out) {
  std::string t = Trim(s);
  if (t.empty()) return false;

  // Remove common formatting like commas in capacity.
  std::string cleaned;
  cleaned.reserve(t.size());
  for (std::size_t i = 0; i < t.size(); ++i) {
    if (t[i] != ',') cleaned.push_back(t[i]);
  }

  std::istringstream iss(cleaned);
  long long v = 0;
  iss >> v;
  if (!iss || (iss.peek() != EOF && !iss.eof())) return false;
  out = v;
  return true;
}

bool StadiumLocator::ToDouble(const std::string& s, double& out) {
  std::string t = Trim(s);
  if (t.empty()) return false;

  std::istringstream iss(t);
  double v = 0.0;
  iss >> v;
  if (!iss || (iss.peek() != EOF && !iss.eof())) return false;
  out = v;
  return true;
}

double StadiumLocator::HaversineKm(double lat1, double lon1, double lat2, double lon2) {
  constexpr double kPi = 3.14159265358979323846;
  constexpr double kEarthRadiusKm = 6371.0;

  auto deg2rad = [](double deg) -> double {
    constexpr double kPiLocal = 3.14159265358979323846;
    return deg * (kPiLocal / 180.0);
  };

  double dlat = deg2rad(lat2 - lat1);
  double dlon = deg2rad(lon2 - lon1);

  double a = std::sin(dlat / 2.0) * std::sin(dlat / 2.0) +
             std::cos(deg2rad(lat1)) * std::cos(deg2rad(lat2)) *
             std::sin(dlon / 2.0) * std::sin(dlon / 2.0);

  double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
  (void)kPi; // keep compilers happy if they warn about unused constants in some builds

  return kEarthRadiusKm * c;
}

bool StadiumLocator::LoadFromCSV(const std::string& csv_path) {
  stadiums_.clear();

  std::ifstream fin(csv_path);
  if (!fin.is_open()) {
    std::cerr << "Error: Could not open CSV file: " << csv_path << "\n";
    return false;
  }

  std::string line;
  bool header_checked = false;
  std::size_t line_no = 0;
  std::size_t skipped = 0;

  while (std::getline(fin, line)) {
    ++line_no;
    if (Trim(line).empty()) continue;

    std::vector<std::string> cols = SplitCSVLine(line);

    // Expect 7 columns: Team,City,Stadium,Capacity,Latitude,Longitude,Country
    if (cols.size() < 7) {
      // If this is likely a header or malformed line, skip it.
      ++skipped;
      continue;
    }

    // If first non-empty line looks like header, skip it once.
    if (!header_checked) {
      header_checked = true;
      std::string c0 = cols[0];
      std::string c1 = cols[1];
      std::string c2 = cols[2];
      std::string c3 = cols[3];
      std::string c4 = cols[4];
      std::string c5 = cols[5];
      std::string c6 = cols[6];
      auto lower = [](std::string s) -> std::string {
        for (std::size_t i = 0; i < s.size(); ++i) {
          s[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(s[i])));
        }
        return s;
      };
      if (lower(c0) == "team" && lower(c1) == "city" && lower(c2) == "stadium" &&
          lower(c3) == "capacity" && lower(c4) == "latitude" && lower(c5) == "longitude" &&
          lower(c6) == "country") {
        continue;
      }
    }

    Stadium s;
    s.team = cols[0];
    s.city = cols[1];
    s.stadium = cols[2];

    long long cap = 0;
    double lat = 0.0;
    double lon = 0.0;

    if (!ToLongLong(cols[3], cap) || !ToDouble(cols[4], lat) || !ToDouble(cols[5], lon)) {
      ++skipped;
      continue;
    }

    s.capacity = cap;
    s.latitude = lat;
    s.longitude = lon;
    s.country = cols[6];

    stadiums_.push_back(s);
  }

  if (stadiums_.empty()) {
    std::cerr << "Error: No valid stadium rows were loaded from: " << csv_path << "\n";
    return false;
  }

  if (skipped > 0) {
    std::cerr << "Note: Skipped " << skipped << " malformed row(s).\n";
  }

  return true;
}

bool StadiumLocator::FindNearest(double user_lat, double user_lon, StadiumDistance& out) const {
  if (stadiums_.empty()) return false;

  double best = std::numeric_limits<double>::infinity();
  std::size_t best_i = 0;

  for (std::size_t i = 0; i < stadiums_.size(); ++i) {
    double d = HaversineKm(user_lat, user_lon, stadiums_[i].latitude, stadiums_[i].longitude);
    if (d < best) {
      best = d;
      best_i = i;
    }
  }

  out.stadium = stadiums_[best_i];
  out.distance_km = best;
  return true;
}

std::vector<StadiumDistance> StadiumLocator::FindKNearest(double user_lat, double user_lon, std::size_t k) const {
  std::vector<StadiumDistance> result;
  if (stadiums_.empty() || k == 0) return result;

  std::vector<std::pair<double, std::size_t> > dist_idx;
  dist_idx.reserve(stadiums_.size());

  for (std::size_t i = 0; i < stadiums_.size(); ++i) {
    double d = HaversineKm(user_lat, user_lon, stadiums_[i].latitude, stadiums_[i].longitude);
    dist_idx.push_back(std::make_pair(d, i));
  }

  std::sort(dist_idx.begin(), dist_idx.end(),
            [](const std::pair<double, std::size_t>& a, const std::pair<double, std::size_t>& b) {
              return a.first < b.first;
            });

  if (k > dist_idx.size()) k = dist_idx.size();
  result.reserve(k);

  for (std::size_t j = 0; j < k; ++j) {
    StadiumDistance sd;
    sd.stadium = stadiums_[dist_idx[j].second];
    sd.distance_km = dist_idx[j].first;
    result.push_back(sd);
  }

  return result;
}