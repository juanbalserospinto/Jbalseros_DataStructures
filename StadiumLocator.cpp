#include "StadiumLocator.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

using namespace std;

StadiumLocator::StadiumLocator(const string& csv_path) : csv_path_(csv_path) {}

size_t StadiumLocator::Size() const {
  return records_.size();
}

string StadiumLocator::Trim(const string& s) {
  size_t i = 0;
  while (i < s.size() && isspace(static_cast<unsigned char>(s[i]))) {
    i++;
  }
  size_t j = s.size();
  while (j > i && isspace(static_cast<unsigned char>(s[j - 1]))) {
    j--;
  }
  return s.substr(i, j - i);
}

bool StadiumLocator::ToLongLong(const string& s, long long* out) {
  if (!out) return false;
  try {
    size_t idx = 0;
    string t = Trim(s);
    long long v = stoll(t, &idx);
    if (idx != t.size()) return false;
    *out = v;
    return true;
  } catch (...) {
    return false;
  }
}

bool StadiumLocator::ToDouble(const string& s, double* out) {
  if (!out) return false;
  try {
    size_t idx = 0;
    string t = Trim(s);
    double v = stod(t, &idx);
    if (idx != t.size()) return false;
    *out = v;
    return true;
  } catch (...) {
    return false;
  }
}

bool StadiumLocator::ParseLineCSV(const string& line, vector<string>* fields) {
  if (!fields) return false;
  fields->clear();

  string cur;
  bool in_quotes = false;

  for (size_t i = 0; i < line.size(); i++) {
    char c = line[i];
    if (c == '"') {
      if (in_quotes && i + 1 < line.size() && line[i + 1] == '"') {
        cur.push_back('"');
        i++;
      } else {
        in_quotes = !in_quotes;
      }
    } else if (c == ',' && !in_quotes) {
      fields->push_back(cur);
      cur.clear();
    } else {
      cur.push_back(c);
    }
  }
  fields->push_back(cur);
  return true;
}

bool StadiumLocator::Load() {
  records_.clear();

  ifstream fin(csv_path_);
  if (!fin.is_open()) {
    cout << "could not open file: " << csv_path_ << "\n";
    return false;
  }

  string line;
  bool header_skipped = false;

  while (getline(fin, line)) {
    if (!header_skipped) {
      header_skipped = true;  
      continue;
    }
    if (line.empty()) continue;

    vector<string> f;
    if (!ParseLineCSV(line, &f)) continue;
    if (f.size() < 7) continue;

    StadiumRecord r;
    r.team = Trim(f[0]);
    r.city = Trim(f[1]);
    r.stadium = Trim(f[2]);

    long long cap = 0;
    double lat = 0.0;
    double lon = 0.0;

    if (!ToLongLong(f[3], &cap)) continue;
    if (!ToDouble(f[4], &lat)) continue;
    if (!ToDouble(f[5], &lon)) continue;

    r.capacity = cap;
    r.latitude = lat;
    r.longitude = lon;
    r.country = Trim(f[6]);

    records_.push_back(r);
  }

  if (records_.empty()) {
    cout << "Warning: no valid stadium rows loaded from " << csv_path_ << "\n";
  }
  return true;
}

double StadiumLocator::DegToRad(double deg) {
  return deg * (3.14159265358979323846 / 180.0);
}

double StadiumLocator::HaversineKm(double lat1, double lon1, double lat2, double lon2) {
  const double R = 6371.0;  
  const double dlat = DegToRad(lat2 - lat1);
  const double dlon = DegToRad(lon2 - lon1);

  const double a =
      sin(dlat / 2.0) * sin(dlat / 2.0) +
      cos(DegToRad(lat1)) * cos(DegToRad(lat2)) *
      sin(dlon / 2.0) * sin(dlon / 2.0);

  const double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
  return R * c;
}

vector<StadiumResult> StadiumLocator::FindKNearest(double user_lat, double user_lon, size_t k) const {
  vector<StadiumResult> out;
  out.reserve(records_.size());

  for (size_t i = 0; i < records_.size(); i++) {
    const StadiumRecord& r = records_[i];
    StadiumResult res;
    res.record = r;
    res.distance_km = HaversineKm(user_lat, user_lon, r.latitude, r.longitude);
    out.push_back(res);
  }

  sort(out.begin(), out.end(),
       [](const StadiumResult& a, const StadiumResult& b) {
         return a.distance_km < b.distance_km;
       });

  if (k > out.size()) k = out.size();
  out.resize(k);
  return out;
}
