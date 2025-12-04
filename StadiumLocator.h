#ifndef STADIUMLOCATOR_H
#define STADIUMLOCATOR_H

#include <cstddef>
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
  ~StadiumLocator();

  StadiumLocator(const StadiumLocator&) = delete;
  StadiumLocator& operator=(const StadiumLocator&) = delete;

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

  struct KDNode {
    size_t idx;
    int axis;
    KDNode* left;
    KDNode* right;
    KDNode(size_t i, int a) : idx(i), axis(a), left(nullptr), right(nullptr) {}
  };

  void ClearKD(KDNode* node);
  void BuildKD();
  KDNode* BuildKDRec(size_t lo, size_t hi, int depth);

  void KNearestRec(KDNode* node, double user_lat, double user_lon, size_t k, vector<pair<double, size_t> >* max_heap) const;

  static void HeapPush(vector<pair<double, size_t> >* max_heap, const pair<double, size_t>& item);
  static void HeapPop(vector<pair<double, size_t> >* max_heap);
  static pair<double, size_t> HeapTop(const vector<pair<double, size_t> >& max_heap);

  static double PlaneLowerBoundKm(int axis, double user_lat, double node_lat, double node_lon);

  string csv_path_;
  vector<StadiumRecord> records_;

  vector<size_t> kd_indices_;
  KDNode* kd_root_;
};

#endif