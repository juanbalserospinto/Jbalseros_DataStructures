stadium_locator: main.cpp StadiumLocator.cpp StadiumLocator.h
	g++ -std=c++17 main.cpp StadiumLocator.cpp -o stadium_locator

run: stadium_locator
	./stadium_locator stadiums.csv

clean:
	rm -f stadium_locator
