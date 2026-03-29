#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

class Route {
private:
    string routeID;
    string from;
    string to;
    double fare;
    int seatCapacity;
    vector<bool> seats;

public:
    Route() : fare(0), seatCapacity(0) {}

    Route(string id, string f, string t, double fr, int cap)
        : routeID(id), from(f), to(t), fare(fr), seatCapacity(cap) {
        seats.resize(cap, false);
    }

    string getRouteID() const { return routeID; }
    string getFrom() const { return from; }
    string getTo() const { return to; }
    double getFare() const { return fare; }
    int getSeatCapacity() const { return seatCapacity; }

    bool isSeatAvailable(int seatNum) const {
        if (seatNum < 1 || seatNum > seatCapacity) return false;
        return !seats[seatNum - 1];
    }

    bool bookSeat(int seatNum) {
        if (!isSeatAvailable(seatNum)) return false;
        seats[seatNum - 1] = true;
        return true;
    }

    void releaseSeat(int seatNum) {
        if (seatNum >= 1 && seatNum <= seatCapacity)
            seats[seatNum - 1] = false;
    }

    int getAvailableSeats() const {
        int count = 0;
        for (int i = 0; i < seatCapacity; i++)
            if (!seats[i]) count++;
        return count;
    }

    int getBookedSeats() const {
        return seatCapacity - getAvailableSeats();
    }

    void display() const {
        cout << "Route ID: " << routeID
             << " | " << from << " -> " << to
             << " | Fare: Rs." << fare
             << " | Available Seats: " << getAvailableSeats()
             << "/" << seatCapacity << endl;
    }
};

class Ticket {
private:
    string ticketID;
    string passengerName;
    string routeID;
    vector<int> seatNumbers;
    double totalFare;
    bool cancelled;

public:
    Ticket() : totalFare(0), cancelled(false) {}

    Ticket(string tid, string name, string rid, vector<int> seats, double fare)
        : ticketID(tid), passengerName(name), routeID(rid),
          seatNumbers(seats), totalFare(fare), cancelled(false) {}

    string getTicketID() const { return ticketID; }
    string getPassengerName() const { return passengerName; }
    string getRouteID() const { return routeID; }
    vector<int> getSeatNumbers() const { return seatNumbers; }
    double getTotalFare() const { return totalFare; }
    bool isCancelled() const { return cancelled; }

    void cancelTicket() { cancelled = true; }

    void display() const {
        cout << "---------------------------------------" << endl;
        cout << "Ticket ID    : " << ticketID << endl;
        cout << "Passenger    : " << passengerName << endl;
        cout << "Route ID     : " << routeID << endl;
        cout << "Seat(s)      : ";
        for (size_t i = 0; i < seatNumbers.size(); i++) {
            cout << seatNumbers[i];
            if (i < seatNumbers.size() - 1) cout << ", ";
        }
        cout << endl;
        cout << "Total Fare   : Rs." << totalFare << endl;
        cout << "Status       : " << (cancelled ? "CANCELLED" : "CONFIRMED") << endl;
        cout << "---------------------------------------" << endl;
    }

    string serialize() const {
        stringstream ss;
        ss << ticketID << "|" << passengerName << "|" << routeID << "|";
        for (size_t i = 0; i < seatNumbers.size(); i++) {
            ss << seatNumbers[i];
            if (i < seatNumbers.size() - 1) ss << ",";
        }
        ss << "|" << totalFare << "|" << (cancelled ? 1 : 0);
        return ss.str();
    }

    static Ticket deserialize(const string& line) {
        stringstream ss(line);
        string tid, name, rid, seatsStr, fareStr, cancelStr;
        getline(ss, tid, '|');
        getline(ss, name, '|');
        getline(ss, rid, '|');
        getline(ss, seatsStr, '|');
        getline(ss, fareStr, '|');
        getline(ss, cancelStr, '|');

        vector<int> seats;
        stringstream seatStream(seatsStr);
        string seatNum;
        while (getline(seatStream, seatNum, ',')) {
            seats.push_back(stoi(seatNum));
        }

        Ticket t(tid, name, rid, seats, stod(fareStr));
        if (cancelStr == "1") t.cancelTicket();
        return t;
    }
};

class ReservationSystem {
private:
    vector<Route> routes;
    vector<Ticket> tickets;
    int ticketCounter;
    string dataFile;

    string generateTicketID() {
        ticketCounter++;
        return "TKT" + to_string(ticketCounter);
    }

    void saveToFile() {
        ofstream fout(dataFile.c_str());
        if (!fout) {
            cout << "Error: Could not save data." << endl;
            return;
        }
        fout << ticketCounter << endl;
        fout << tickets.size() << endl;
        for (size_t i = 0; i < tickets.size(); i++) {
            fout << tickets[i].serialize() << endl;
        }
        fout.close();
    }

    void loadFromFile() {
        ifstream fin(dataFile.c_str());
        if (!fin) return;

        fin >> ticketCounter;
        int count;
        fin >> count;
        fin.ignore();

        tickets.clear();
        for (int i = 0; i < count; i++) {
            string line;
            getline(fin, line);
            if (!line.empty()) {
                Ticket t = Ticket::deserialize(line);
                tickets.push_back(t);

                if (!t.isCancelled()) {
                    for (size_t j = 0; j < routes.size(); j++) {
                        if (routes[j].getRouteID() == t.getRouteID()) {
                            vector<int> sns = t.getSeatNumbers();
                            for (size_t k = 0; k < sns.size(); k++) {
                                routes[j].bookSeat(sns[k]);
                            }
                        }
                    }
                }
            }
        }
        fin.close();
    }

public:
    ReservationSystem(string file = "tickets.dat")
        : ticketCounter(0), dataFile(file) {}

    void addRoute(const Route& r) {
        routes.push_back(r);
    }

    void initialize() {
        loadFromFile();
    }

    void displayRoutes() {
        cout << "\n===== AVAILABLE BUS ROUTES =====" << endl;
        for (size_t i = 0; i < routes.size(); i++) {
            routes[i].display();
        }
        cout << "================================" << endl;
    }

    void bookTicket() {
        string name, routeID;
        int numSeats;

        cout << "\nEnter Passenger Name: ";
        cin.ignore();
        getline(cin, name);

        displayRoutes();
        cout << "Enter Route ID (e.g., R1, R2): ";
        cin >> routeID;

        Route* selectedRoute = NULL;
        for (size_t i = 0; i < routes.size(); i++) {
            if (routes[i].getRouteID() == routeID) {
                selectedRoute = &routes[i];
                break;
            }
        }

        if (!selectedRoute) {
            cout << "Error: Invalid Route ID!" << endl;
            return;
        }

        cout << "How many seats to book? ";
        cin >> numSeats;

        if (numSeats <= 0 || numSeats > selectedRoute->getAvailableSeats()) {
            cout << "Error: Invalid number of seats or not enough available!" << endl;
            return;
        }

        vector<int> bookedSeats;
        for (int i = 0; i < numSeats; i++) {
            int seatNum;
            cout << "Enter seat number " << (i + 1) << " (1-" << selectedRoute->getSeatCapacity() << "): ";
            cin >> seatNum;

            if (seatNum < 1 || seatNum > selectedRoute->getSeatCapacity()) {
                cout << "Error: Seat " << seatNum << " is out of range!" << endl;
                return;
            }

            if (!selectedRoute->isSeatAvailable(seatNum)) {
                cout << "Error: Seat " << seatNum << " is already booked!" << endl;
                return;
            }

            bookedSeats.push_back(seatNum);
        }

        for (size_t i = 0; i < bookedSeats.size(); i++) {
            selectedRoute->bookSeat(bookedSeats[i]);
        }

        double totalFare = selectedRoute->getFare() * numSeats;
        string tid = generateTicketID();
        Ticket ticket(tid, name, routeID, bookedSeats, totalFare);
        tickets.push_back(ticket);

        saveToFile();

        cout << "\n*** Booking Successful! ***" << endl;
        ticket.display();
    }

    void cancelTicket() {
        string tid;
        cout << "\nEnter Ticket ID to cancel: ";
        cin >> tid;

        for (size_t i = 0; i < tickets.size(); i++) {
            if (tickets[i].getTicketID() == tid) {
                if (tickets[i].isCancelled()) {
                    cout << "Error: Ticket is already cancelled!" << endl;
                    return;
                }

                tickets[i].cancelTicket();

                for (size_t j = 0; j < routes.size(); j++) {
                    if (routes[j].getRouteID() == tickets[i].getRouteID()) {
                        vector<int> sns = tickets[i].getSeatNumbers();
                        for (size_t k = 0; k < sns.size(); k++) {
                            routes[j].releaseSeat(sns[k]);
                        }
                        break;
                    }
                }

                saveToFile();
                cout << "Ticket " << tid << " has been cancelled. Seats released." << endl;
                return;
            }
        }
        cout << "Error: Ticket ID not found!" << endl;
    }

    void searchTicket() {
        string tid;
        cout << "\nEnter Ticket ID to search: ";
        cin >> tid;

        for (size_t i = 0; i < tickets.size(); i++) {
            if (tickets[i].getTicketID() == tid) {
                tickets[i].display();
                return;
            }
        }
        cout << "Error: Ticket ID not found!" << endl;
    }

    void showReports() {
        cout << "\n===== REPORTS =====" << endl;

        map<string, double> revenueMap;
        map<string, int> bookingCount;

        for (size_t i = 0; i < tickets.size(); i++) {
            if (!tickets[i].isCancelled()) {
                revenueMap[tickets[i].getRouteID()] += tickets[i].getTotalFare();
                bookingCount[tickets[i].getRouteID()] += (int)tickets[i].getSeatNumbers().size();
            }
        }

        cout << "\n-- Revenue Per Route --" << endl;
        for (size_t i = 0; i < routes.size(); i++) {
            string rid = routes[i].getRouteID();
            double rev = revenueMap.count(rid) ? revenueMap[rid] : 0;
            cout << rid << " (" << routes[i].getFrom() << " -> " << routes[i].getTo() << "): Rs." << rev << endl;
        }

        cout << "\n-- Total Tickets Booked Per Route --" << endl;
        for (size_t i = 0; i < routes.size(); i++) {
            string rid = routes[i].getRouteID();
            int cnt = bookingCount.count(rid) ? bookingCount[rid] : 0;
            cout << rid << " (" << routes[i].getFrom() << " -> " << routes[i].getTo() << "): " << cnt << " seat(s)" << endl;
        }

        string popularRoute = "";
        int maxBookings = 0;
        for (map<string,int>::iterator it = bookingCount.begin(); it != bookingCount.end(); ++it) {
            if (it->second > maxBookings) {
                maxBookings = it->second;
                popularRoute = it->first;
            }
        }

        cout << "\n-- Most Popular Route --" << endl;
        if (!popularRoute.empty()) {
            for (size_t i = 0; i < routes.size(); i++) {
                if (routes[i].getRouteID() == popularRoute) {
                    cout << routes[i].getRouteID() << " (" << routes[i].getFrom() << " -> " << routes[i].getTo()
                         << ") with " << maxBookings << " booking(s)" << endl;
                    break;
                }
            }
        } else {
            cout << "No bookings yet." << endl;
        }
        cout << "===================" << endl;
    }

    void run() {
        int choice;
        do {
            cout << "\n========================================" << endl;
            cout << "   BUS RESERVATION SYSTEM" << endl;
            cout << "========================================" << endl;
            cout << "1. Display Available Routes" << endl;
            cout << "2. Book Ticket" << endl;
            cout << "3. Cancel Ticket" << endl;
            cout << "4. Search Ticket by ID" << endl;
            cout << "5. View Reports" << endl;
            cout << "6. Exit" << endl;
            cout << "========================================" << endl;
            cout << "Enter your choice: ";
            cin >> choice;

            switch (choice) {
                case 1: displayRoutes(); break;
                case 2: bookTicket(); break;
                case 3: cancelTicket(); break;
                case 4: searchTicket(); break;
                case 5: showReports(); break;
                case 6: cout << "Thank you! Exiting..." << endl; break;
                default: cout << "Invalid choice! Try again." << endl;
            }
        } while (choice != 6);
    }
};

int main() {
    ReservationSystem system("tickets.dat");

    // Route 1: Chennai -> Salem -> Coimbatore
    system.addRoute(Route("R1", "Chennai", "Coimbatore (via Salem)", 450.0, 30));
    // Route 2: Chennai -> Mayiladuthurai -> Nagapattinam
    system.addRoute(Route("R2", "Chennai", "Nagapattinam (via Mayiladuthurai)", 350.0, 30));

    system.initialize();
    system.run();

    return 0;
}
