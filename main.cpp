/*
 * Bus Reservation System - C++ Source
 * Routes:
 *   R1: Chennai -> Salem -> Coimbatore (Fare: 450, 30 seats)
 *   R2: Chennai -> Mayiladuthurai -> Nagapattinam (Fare: 350, 30 seats)
 *
 * Features: Book, Cancel, Search, Revenue Report
 * This file is the reference C++ implementation.
 * The web UI (index.html + index.js) mirrors this logic in JavaScript.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iomanip>

using namespace std;

// ─── Route Class ───
class Route {
public:
    string id;
    string name;
    vector<string> stops;
    double fare;
    int totalSeats;
    int availableSeats;

    Route() : fare(0), totalSeats(0), availableSeats(0) {}

    Route(string id, string name, vector<string> stops, double fare, int seats)
        : id(id), name(name), stops(stops), fare(fare),
          totalSeats(seats), availableSeats(seats) {}

    bool bookSeat() {
        if (availableSeats > 0) { availableSeats--; return true; }
        return false;
    }

    void cancelSeat() {
        if (availableSeats < totalSeats) availableSeats++;
    }

    void display() const {
        cout << "  Route " << id << ": " << name << endl;
        cout << "  Stops: ";
        for (size_t i = 0; i < stops.size(); i++) {
            cout << stops[i];
            if (i < stops.size() - 1) cout << " -> ";
        }
        cout << endl;
        cout << "  Fare: Rs." << fare
             << " | Available: " << availableSeats << "/" << totalSeats << endl;
    }
};

// ─── Ticket Class ───
class Ticket {
public:
    string ticketId;
    string passengerName;
    string routeId;
    int seatNumber;
    double fare;
    bool active;

    Ticket() : seatNumber(0), fare(0), active(false) {}

    Ticket(string tid, string pname, string rid, int seat, double f)
        : ticketId(tid), passengerName(pname), routeId(rid),
          seatNumber(seat), fare(f), active(true) {}

    void display() const {
        cout << "  Ticket ID   : " << ticketId << endl;
        cout << "  Passenger   : " << passengerName << endl;
        cout << "  Route       : " << routeId << endl;
        cout << "  Seat Number : " << seatNumber << endl;
        cout << "  Fare        : Rs." << fare << endl;
        cout << "  Status      : " << (active ? "Active" : "Cancelled") << endl;
    }
};

// ─── Reservation System Class ───
class ReservationSystem {
private:
    map<string, Route> routes;
    vector<Ticket> tickets;
    int ticketCounter;

    string generateTicketId() {
        ticketCounter++;
        return "TKT" + to_string(1000 + ticketCounter);
    }

public:
    ReservationSystem() : ticketCounter(0) {
        // Route 1: Chennai -> Salem -> Coimbatore
        routes["R1"] = Route("R1", "Chennai - Coimbatore Express",
            {"Chennai", "Salem", "Coimbatore"}, 450.0, 30);

        // Route 2: Chennai -> Mayiladuthurai -> Nagapattinam
        routes["R2"] = Route("R2", "Chennai - Nagapattinam Express",
            {"Chennai", "Mayiladuthurai", "Nagapattinam"}, 350.0, 30);

        loadTickets();
    }

    ~ReservationSystem() { saveTickets(); }

    void displayRoutes() {
        cout << "\n╔══════════════════════════════════╗" << endl;
        cout << "║       AVAILABLE ROUTES           ║" << endl;
        cout << "╚══════════════════════════════════╝" << endl;
        for (auto& p : routes) {
            p.second.display();
            cout << "  ────────────────────────────" << endl;
        }
    }

    void bookTicket() {
        displayRoutes();
        cout << "\nEnter Route ID (R1/R2): ";
        string rid; cin >> rid;
        transform(rid.begin(), rid.end(), rid.begin(), ::toupper);

        if (routes.find(rid) == routes.end()) {
            cout << "  ✗ Invalid route ID.\n"; return;
        }

        Route& route = routes[rid];
        if (route.availableSeats <= 0) {
            cout << "  ✗ No seats available on this route.\n"; return;
        }

        cout << "Enter Passenger Name: ";
        cin.ignore();
        string name; getline(cin, name);

        int seat = route.totalSeats - route.availableSeats + 1;
        route.bookSeat();

        string tid = generateTicketId();
        Ticket t(tid, name, rid, seat, route.fare);
        tickets.push_back(t);
        saveTickets();

        cout << "\n  ✓ Booking Confirmed!\n";
        t.display();
    }

    void cancelTicket() {
        cout << "\nEnter Ticket ID to cancel: ";
        string tid; cin >> tid;

        for (auto& t : tickets) {
            if (t.ticketId == tid && t.active) {
                t.active = false;
                routes[t.routeId].cancelSeat();
                saveTickets();
                cout << "  ✓ Ticket " << tid << " cancelled successfully.\n";
                return;
            }
        }
        cout << "  ✗ Ticket not found or already cancelled.\n";
    }

    void searchTicket() {
        cout << "\nEnter Ticket ID to search: ";
        string tid; cin >> tid;

        for (auto& t : tickets) {
            if (t.ticketId == tid) {
                cout << "\n  Ticket Found:\n";
                t.display();
                return;
            }
        }
        cout << "  ✗ Ticket not found.\n";
    }

    void revenueReport() {
        cout << "\n╔══════════════════════════════════╗" << endl;
        cout << "║        REVENUE REPORT            ║" << endl;
        cout << "╚══════════════════════════════════╝" << endl;

        double totalRevenue = 0;
        map<string, double> routeRevenue;
        map<string, int> routeBookings;

        for (auto& t : tickets) {
            if (t.active) {
                routeRevenue[t.routeId] += t.fare;
                routeBookings[t.routeId]++;
                totalRevenue += t.fare;
            }
        }

        for (auto& p : routes) {
            cout << "  Route " << p.first << " (" << p.second.name << "):" << endl;
            cout << "    Bookings: " << routeBookings[p.first]
                 << " | Revenue: Rs." << fixed << setprecision(2)
                 << routeRevenue[p.first] << endl;
        }

        cout << "\n  Total Revenue: Rs." << fixed << setprecision(2)
             << totalRevenue << endl;
    }

    void saveTickets() {
        ofstream f("tickets.dat");
        for (auto& t : tickets) {
            f << t.ticketId << "|" << t.passengerName << "|"
              << t.routeId << "|" << t.seatNumber << "|"
              << t.fare << "|" << t.active << endl;
        }
        f.close();
    }

    void loadTickets() {
        ifstream f("tickets.dat");
        if (!f.is_open()) return;
        string line;
        while (getline(f, line)) {
            size_t p1 = line.find('|');
            size_t p2 = line.find('|', p1 + 1);
            size_t p3 = line.find('|', p2 + 1);
            size_t p4 = line.find('|', p3 + 1);
            size_t p5 = line.find('|', p4 + 1);
            if (p5 == string::npos) continue;

            Ticket t;
            t.ticketId      = line.substr(0, p1);
            t.passengerName = line.substr(p1+1, p2-p1-1);
            t.routeId       = line.substr(p2+1, p3-p2-1);
            t.seatNumber    = stoi(line.substr(p3+1, p4-p3-1));
            t.fare          = stod(line.substr(p4+1, p5-p4-1));
            t.active        = line.substr(p5+1) == "1";
            tickets.push_back(t);

            int num = stoi(t.ticketId.substr(3)) - 1000;
            if (num > ticketCounter) ticketCounter = num;

            if (t.active && routes.count(t.routeId))
                routes[t.routeId].bookSeat();
        }
        f.close();
    }
};

// ─── Main ───
int main() {
    ReservationSystem sys;
    int choice;

    do {
        cout << "\n╔══════════════════════════════════╗" << endl;
        cout << "║   BUS RESERVATION SYSTEM         ║" << endl;
        cout << "╠══════════════════════════════════╣" << endl;
        cout << "║  1. View Routes                  ║" << endl;
        cout << "║  2. Book Ticket                  ║" << endl;
        cout << "║  3. Cancel Ticket                ║" << endl;
        cout << "║  4. Search Ticket                ║" << endl;
        cout << "║  5. Revenue Report               ║" << endl;
        cout << "║  6. Exit                         ║" << endl;
        cout << "╚══════════════════════════════════╝" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: sys.displayRoutes();  break;
            case 2: sys.bookTicket();     break;
            case 3: sys.cancelTicket();   break;
            case 4: sys.searchTicket();   break;
            case 5: sys.revenueReport();  break;
            case 6: cout << "  Goodbye!\n"; break;
            default: cout << "  ✗ Invalid choice.\n";
        }
    } while (choice != 6);

    return 0;
}
