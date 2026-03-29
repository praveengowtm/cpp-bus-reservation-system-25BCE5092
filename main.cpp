// Bus Reservation System - C++ OOP Implementation
// Routes: R1 (Chennai->Salem->Coimbatore), R2 (Chennai->Mayiladuthurai->Nagapattinam), R3 (Chennai->Madurai->Kanyakumari)

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

// ==================== Route Class ====================
class Route {
private:
    string routeId;
    string routeName;
    vector<string> stops;
    int totalSeats;
    int availableSeats;
    double fare;
    vector<bool> seatMap;

public:
    Route() : totalSeats(0), availableSeats(0), fare(0) {}

    Route(string id, string name, vector<string> stopList, int seats, double f)
        : routeId(id), routeName(name), stops(stopList),
          totalSeats(seats), availableSeats(seats), fare(f) {
        seatMap.resize(seats, false);
    }

    string getRouteId() const { return routeId; }
    string getRouteName() const { return routeName; }
    int getTotalSeats() const { return totalSeats; }
    int getAvailableSeats() const { return availableSeats; }
    double getFare() const { return fare; }

    string getStopsString() const {
        string result;
        for (size_t i = 0; i < stops.size(); i++) {
            result += stops[i];
            if (i < stops.size() - 1) result += " -> ";
        }
        return result;
    }

    int bookSeat() {
        for (int i = 0; i < totalSeats; i++) {
            if (!seatMap[i]) {
                seatMap[i] = true;
                availableSeats--;
                return i + 1;
            }
        }
        return -1;
    }

    bool cancelSeat(int seatNumber) {
        int idx = seatNumber - 1;
        if (idx >= 0 && idx < totalSeats && seatMap[idx]) {
            seatMap[idx] = false;
            availableSeats++;
            return true;
        }
        return false;
    }

    void displayInfo() const {
        cout << "Route ID: " << routeId << endl;
        cout << "Route: " << getStopsString() << endl;
        cout << "Total Seats: " << totalSeats << endl;
        cout << "Available Seats: " << availableSeats << endl;
        cout << "Fare: Rs." << fare << endl;
    }
};

// ==================== Ticket Class ====================
class Ticket {
private:
    string ticketId;
    string passengerName;
    string routeId;
    int seatNumber;
    double fare;
    bool isActive;

public:
    Ticket() : seatNumber(0), fare(0), isActive(false) {}

    Ticket(string tId, string name, string rId, int seat, double f)
        : ticketId(tId), passengerName(name), routeId(rId),
          seatNumber(seat), fare(f), isActive(true) {}

    string getTicketId() const { return ticketId; }
    string getPassengerName() const { return passengerName; }
    string getRouteId() const { return routeId; }
    int getSeatNumber() const { return seatNumber; }
    double getFare() const { return fare; }
    bool getIsActive() const { return isActive; }

    void cancel() { isActive = false; }

    void display() const {
        cout << "Ticket ID: " << ticketId << endl;
        cout << "Passenger: " << passengerName << endl;
        cout << "Route: " << routeId << endl;
        cout << "Seat: " << seatNumber << endl;
        cout << "Fare: Rs." << fare << endl;
        cout << "Status: " << (isActive ? "Active" : "Cancelled") << endl;
    }

    string serialize() const {
        stringstream ss;
        ss << ticketId << "|" << passengerName << "|" << routeId << "|"
           << seatNumber << "|" << fare << "|" << isActive;
        return ss.str();
    }

    static Ticket deserialize(const string& data) {
        stringstream ss(data);
        string tId, name, rId, seatStr, fareStr, activeStr;
        getline(ss, tId, '|');
        getline(ss, name, '|');
        getline(ss, rId, '|');
        getline(ss, seatStr, '|');
        getline(ss, fareStr, '|');
        getline(ss, activeStr, '|');
        Ticket t(tId, name, rId, stoi(seatStr), stod(fareStr));
        if (activeStr == "0") t.cancel();
        return t;
    }
};

// ==================== ReservationSystem Class ====================
class ReservationSystem {
private:
    map<string, Route> routes;
    vector<Ticket> tickets;
    int ticketCounter;

    string generateTicketId() {
        ticketCounter++;
        stringstream ss;
        ss << "TKT" << ticketCounter;
        return ss.str();
    }

public:
    ReservationSystem() : ticketCounter(0) {
        vector<string> stops1 = {"Chennai", "Salem", "Coimbatore"};
        routes["R1"] = Route("R1", "Chennai-Coimbatore Express", stops1, 30, 450.0);

        vector<string> stops2 = {"Chennai", "Mayiladuthurai", "Nagapattinam"};
        routes["R2"] = Route("R2", "Chennai-Nagapattinam Express", stops2, 30, 350.0);

        vector<string> stops3 = {"Chennai", "Madurai", "Kanyakumari"};
        routes["R3"] = Route("R3", "Chennai-Kanyakumari Express", stops3, 30, 550.0);

        loadTickets();
    }

    void displayRoutes() {
        cout << "\n========== Available Routes ==========\n";
        for (auto& pair : routes) {
            pair.second.displayInfo();
            cout << "--------------------------------------\n";
        }
    }

    string bookTicket(const string& routeId, const string& passengerName) {
        if (routes.find(routeId) == routes.end()) {
            cout << "Error: Route not found!\n";
            return "";
        }
        Route& route = routes[routeId];
        int seatNum = route.bookSeat();
        if (seatNum == -1) {
            cout << "Error: No seats available on this route!\n";
            return "";
        }
        string ticketId = generateTicketId();
        Ticket ticket(ticketId, passengerName, routeId, seatNum, route.getFare());
        tickets.push_back(ticket);
        saveTickets();
        cout << "\n===== Booking Confirmed =====\n";
        ticket.display();
        return ticketId;
    }

    bool cancelTicket(const string& ticketId) {
        for (auto& ticket : tickets) {
            if (ticket.getTicketId() == ticketId && ticket.getIsActive()) {
                string rId = ticket.getRouteId();
                if (routes.find(rId) != routes.end()) {
                    routes[rId].cancelSeat(ticket.getSeatNumber());
                }
                ticket.cancel();
                saveTickets();
                cout << "Ticket " << ticketId << " cancelled successfully.\n";
                return true;
            }
        }
        cout << "Error: Ticket not found or already cancelled.\n";
        return false;
    }

    void searchTicket(const string& ticketId) {
        for (const auto& ticket : tickets) {
            if (ticket.getTicketId() == ticketId) {
                cout << "\n===== Ticket Details =====\n";
                ticket.display();
                return;
            }
        }
        cout << "Ticket not found.\n";
    }

    void generateReport() {
        double totalRevenue = 0;
        int totalBooked = 0, totalCancelled = 0;
        for (const auto& ticket : tickets) {
            if (ticket.getIsActive()) { totalRevenue += ticket.getFare(); totalBooked++; }
            else totalCancelled++;
        }

        cout << "\n========== Revenue Report ==========\n";
        cout << "Total Bookings: " << totalBooked << endl;
        cout << "Total Cancellations: " << totalCancelled << endl;
        cout << "Total Revenue: Rs." << totalRevenue << endl;

        // Most popular route
        string popularId;
        int maxBooked = -1;
        for (auto& pair : routes) {
            int booked = pair.second.getTotalSeats() - pair.second.getAvailableSeats();
            if (booked > maxBooked) { maxBooked = booked; popularId = pair.first; }
        }
        if (!popularId.empty()) {
            cout << "\nMost Popular Route: " << popularId << " ("
                 << routes[popularId].getStopsString() << ") with "
                 << maxBooked << " bookings\n";
        }

        cout << "\n--- Route Wise Occupancy ---\n";
        for (auto& pair : routes) {
            Route& r = pair.second;
            int booked = r.getTotalSeats() - r.getAvailableSeats();
            cout << r.getRouteId() << " (" << r.getStopsString() << "): "
                 << booked << "/" << r.getTotalSeats() << " seats booked\n";
        }
    }

    void saveTickets() {
        ofstream file("tickets.dat");
        if (file.is_open()) {
            file << ticketCounter << "\n";
            for (const auto& t : tickets) file << t.serialize() << "\n";
            file.close();
        }
    }

    void loadTickets() {
        ifstream file("tickets.dat");
        if (file.is_open()) {
            string line;
            if (getline(file, line)) ticketCounter = stoi(line);
            while (getline(file, line)) {
                if (!line.empty()) {
                    Ticket t = Ticket::deserialize(line);
                    tickets.push_back(t);
                    if (t.getIsActive() && routes.find(t.getRouteId()) != routes.end()) {
                        routes[t.getRouteId()].bookSeat();
                    }
                }
            }
            file.close();
        }
    }
};

// ==================== Main Function ====================
int main() {
    ReservationSystem system;
    int choice;

    while (true) {
        cout << "\n====================================\n";
        cout << "   BUS RESERVATION SYSTEM\n";
        cout << "====================================\n";
        cout << "1. View Routes\n";
        cout << "2. Book Ticket\n";
        cout << "3. Cancel Ticket\n";
        cout << "4. Search Ticket\n";
        cout << "5. Revenue Report\n";
        cout << "6. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: system.displayRoutes(); break;
            case 2: {
                string routeId, name;
                cout << "Enter Route ID (R1/R2/R3): ";
                getline(cin, routeId);
                cout << "Enter Passenger Name: ";
                getline(cin, name);
                system.bookTicket(routeId, name);
                break;
            }
            case 3: {
                string ticketId;
                cout << "Enter Ticket ID: ";
                getline(cin, ticketId);
                system.cancelTicket(ticketId);
                break;
            }
            case 4: {
                string ticketId;
                cout << "Enter Ticket ID: ";
                getline(cin, ticketId);
                system.searchTicket(ticketId);
                break;
            }
            case 5: system.generateReport(); break;
            case 6: cout << "Thank you for using Bus Reservation System!\n"; return 0;
            default: cout << "Invalid choice. Try again.\n";
        }
    }
    return 0;
}
