#include "StadiumLocator.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

using namespace std;

StadiumLocator::StadiumLocator(const string& csv_path) : csv_path_(csv_path), kd_root_(nullptr) {}

StadiumLocator::~StadiumLocator() {
  ClearKD(kd_root_);
  kd_root_ = nullptr;
}

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
  ClearKD(kd_root_);
  kd_root_ = nullptr;

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
    return true;
  }

  BuildKD();
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

void StadiumLocator::ClearKD(KDNode* node) {
  if (!node) return;
  ClearKD(node->left);
  ClearKD(node->right);
  delete node;
}

void StadiumLocator::BuildKD() {
  kd_indices_.clear();
  kd_indices_.reserve(records_.size());
  for (size_t i = 0; i < records_.size(); i++) {
    kd_indices_.push_back(i);
  }
  kd_root_ = BuildKDRec(0, kd_indices_.size(), 0);
}

StadiumLocator::KDNode* StadiumLocator::BuildKDRec(size_t lo, size_t hi, int depth) {
  if (lo >= hi) return nullptr;

  size_t mid = lo + (hi - lo) / 2;
  int axis = depth % 2;

  auto comp = [this, axis](size_t a, size_t b) {
    if (axis == 0) return records_[a].latitude < records_[b].latitude;
    return records_[a].longitude < records_[b].longitude;
  };

  nth_element(kd_indices_.begin() + lo, kd_indices_.begin() + mid, kd_indices_.begin() + hi, comp);

  KDNode* node = new KDNode(kd_indices_[mid], axis);
  node->left = BuildKDRec(lo, mid, depth + 1);
  node->right = BuildKDRec(mid + 1, hi, depth + 1);
  return node;
}

void StadiumLocator::HeapPush(vector<pair<double, size_t> >* max_heap, const pair<double, size_t>& item) {
  max_heap->push_back(item);
  push_heap(max_heap->begin(), max_heap->end(),
            [](const pair<double, size_t>& a, const pair<double, size_t>& b) {
              return a.first < b.first;
            });
}

void StadiumLocator::HeapPop(vector<pair<double, size_t> >* max_heap) {
  pop_heap(max_heap->begin(), max_heap->end(),
           [](const pair<double, size_t>& a, const pair<double, size_t>& b) {
             return a.first < b.first;
           });
  max_heap->pop_back();
}

pair<double, size_t> StadiumLocator::HeapTop(const vector<pair<double, size_t> >& max_heap) {
  return max_heap.front();
}

double StadiumLocator::PlaneLowerBoundKm(int axis, double user_lat, double node_lat, double node_lon) {
  (void)node_lon;
  if (axis == 0) {
    double dlat_rad = DegToRad(user_lat - node_lat);
    return 6371.0 * fabs(dlat_rad);
  }
  return 0.0;
}

void StadiumLocator::KNearestRec(KDNode* node, double user_lat, double user_lon, size_t k, vector<pair<double, size_t> >* max_heap) const {
  if (!node) return;

  const StadiumRecord& r = records_[node->idx];
  double d = HaversineKm(user_lat, user_lon, r.latitude, r.longitude);

  if (max_heap->size() < k) {
    HeapPush(max_heap, make_pair(d, node->idx));
  } else {
    pair<double, size_t> worst = HeapTop(*max_heap);
    if (d < worst.first) {
      HeapPop(max_heap);
      HeapPush(max_heap, make_pair(d, node->idx));
    }
  }

  KDNode* near_child = nullptr;
  KDNode* far_child = nullptr;

  if (node->axis == 0) {
    if (user_lat < r.latitude) {
      near_child = node->left;
      far_child = node->right;
    } else {
      near_child = node->right;
      far_child = node->left;
    }
  } else {
    if (user_lon < r.longitude) {
      near_child = node->left;
      far_child = node->right;
    } else {
      near_child = node->right;
      far_child = node->left;
    }
  }

  KNearestRec(near_child, user_lat, user_lon, k, max_heap);

  double worst_km = numeric_limits<double>::infinity();
  if (max_heap->size() == k) {
    worst_km = HeapTop(*max_heap).first;
  }

  double plane_lb = PlaneLowerBoundKm(node->axis, user_lat, r.latitude, r.longitude);

  if (max_heap->size() < k || plane_lb < worst_km) {
    KNearestRec(far_child, user_lat, user_lon, k, max_heap);
  }
}

vector<StadiumResult> StadiumLocator::FindKNearest(double user_lat, double user_lon, size_t k) const {
  vector<StadiumResult> out;

  if (k == 0) return out;
  if (records_.empty()) return out;
  if (!kd_root_) return out;
  if (k > records_.size()) k = records_.size();

  vector<pair<double, size_t> > max_heap;
  max_heap.reserve(k);

  KNearestRec(kd_root_, user_lat, user_lon, k, &max_heap);

  out.reserve(max_heap.size());
  for (size_t i = 0; i < max_heap.size(); i++) {
    StadiumResult res;
    res.distance_km = max_heap[i].first;
    res.record = records_[max_heap[i].second];
    out.push_back(res);
  }

  sort(out.begin(), out.end(),
       [](const StadiumResult& a, const StadiumResult& b) {
         return a.distance_km < b.distance_km;
       });

  return out;
}