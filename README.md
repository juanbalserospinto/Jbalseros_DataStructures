# Jbalseros_DataStructures
Final Project for Data Structures Class

# Soccer/Football Stadium Locator (Distance Finder)

This program loads a CSV of soccer stadiums and:
1) Prompts for your latitude/longitude
2) Shows the single nearest stadium
3) Asks for **K**
4) Prints the **K nearest** stadiums with distances

## CSV Format

Your file must contain these columns (in this order):

Team,City,Stadium,Capacity,Latitude,Longitude,Country

Example header:
Team,City,Stadium,Capacity,Latitude,Longitude,Country

Distances are computed using the Haversine formula (great-circle distance).

## Build (g++ / C++17)

From the project folder:

make
make run
make clean
