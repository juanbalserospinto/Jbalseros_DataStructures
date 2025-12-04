# Jbalseros_DataStructures
Final Project for Data Structures Class

#  Stadium Locator (Distance Finder)

This program loads a CSV of soccer stadiums that includes stadiums from all over europe where most of the famoust stadiums are located.
Once you run the program, it will give you the latitude and longituted of the most famous europeans cities in case you do not know your coordinates. After this the program does the followin: 
1) Prompts for your latitude/longitude
2) Asks for a number N, which represents how many stadiums you want the program to show you 
3) Prints the N nearest stadiums with distances with respect to your coordinates



## CSV Format

The CSV included has the following columns, in case you want to upload your own data. Your data must contain such columns:

Team,City,Stadium,Capacity,Latitude,Longitude,Country


## Data Structure Used and Math Used
The program uses a two-dimensional KD-Tree to organize stadium locations by their latitude and longitude. A KD-Tree is made for efficient searching in multidimensional spaces, and since each stadium location is a 2D point (lat, lon), it fits this problem well.

When the program loads the CSV file, it builds the KD-Tree by recursively splitting the data: at each level it alternates comparing latitude and longitude. Each KD-Tree node stores a single stadium record (stadium name, club/team name, and its coordinates), and has left/right children that represent the two halves of the dataset after the split.

After the user inputs their coordinates, the program performs a nearest-neighbor search (and can also return the K nearest stadiums) by traversing the KD-Tree. It searches the most likely branch first, and then checks the other branch only when it could still contain a closer stadium. This makes the search faster than checking every stadium one-by-one for large datasets.

Distances are computed using the Haversine formula, which calculates the great-circle distance between two points on the Earth’s surface, giving an accurate distance in kilometers.


## Build (g++ / C++17)

You can run the program and test it by yourself, you need all the files in the same folder, then using terminal run the following command and follow the instructions given by the program

make

make run

