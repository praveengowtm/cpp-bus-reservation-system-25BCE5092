// Bus Reservation System - JavaScript Logic

// Route data embedded directly (no fetch needed)
var ROUTE_DATA = [
  { id: "R1", name: "Chennai-Coimbatore Express", stops: ["Chennai", "Salem", "Coimbatore"], totalSeats: 30, fare: 450 },
  { id: "R2", name: "Chennai-Nagapattinam Express", stops: ["Chennai", "Mayiladuthurai", "Nagapattinam"], totalSeats: 30, fare: 350 },
  { id: "R3", name: "Chennai-Kanyakumari Express", stops: ["Chennai", "Madurai", "Kanyakumari"], totalSeats: 30, fare: 550 }
];

class Route {
  constructor(id, name, stops, totalSeats, fare) {
    this.id = id;
    this.name = name;
    this.stops = stops;
    this.totalSeats = totalSeats;
    this.availableSeats = totalSeats;
    this.fare = fare;
    this.seatMap = new Array(totalSeats).fill(false);
  }
  getStopsString() { return this.stops.join(' → '); }
  bookSeat() {
    for (var i = 0; i < this.totalSeats; i++) {
      if (!this.seatMap[i]) { this.seatMap[i] = true; this.availableSeats--; return i + 1; }
    }
    return -1;
  }
  cancelSeat(seatNumber) {
    var idx = seatNumber - 1;
    if (idx >= 0 && idx < this.totalSeats && this.seatMap[idx]) { this.seatMap[idx] = false; this.availableSeats++; return true; }
    return false;
  }
}

class Ticket {
  constructor(ticketId, passengerName, routeId, seatNumber, fare) {
    this.ticketId = ticketId;
    this.passengerName = passengerName;
    this.routeId = routeId;
    this.seatNumber = seatNumber;
    this.fare = fare;
    this.isActive = true;
  }
  cancel() { this.isActive = false; }
}

class ReservationSystem {
  constructor(routeData) {
    this.routes = {};
    this.tickets = [];
    this.ticketCounter = 0;
    routeData.forEach(function(r) {
      this.routes[r.id] = new Route(r.id, r.name, r.stops, r.totalSeats, r.fare);
    }.bind(this));
    this.load();
  }

  generateTicketId() {
    this.ticketCounter++;
    return 'TKT' + this.ticketCounter;
  }

  bookTicket(routeId, passengerName) {
    var route = this.routes[routeId];
    if (!route) return { success: false, message: 'Route not found!' };
    var seatNum = route.bookSeat();
    if (seatNum === -1) return { success: false, message: 'No seats available on this route!' };
    var ticketId = this.generateTicketId();
    var ticket = new Ticket(ticketId, passengerName, routeId, seatNum, route.fare);
    this.tickets.push(ticket);
    this.save();
    return { success: true, ticket: ticket };
  }

  cancelTicket(ticketId) {
    var ticket = null;
    for (var i = 0; i < this.tickets.length; i++) {
      if (this.tickets[i].ticketId === ticketId && this.tickets[i].isActive) { ticket = this.tickets[i]; break; }
    }
    if (!ticket) return { success: false, message: 'Ticket not found or already cancelled.' };
    var route = this.routes[ticket.routeId];
    if (route) route.cancelSeat(ticket.seatNumber);
    ticket.cancel();
    this.save();
    return { success: true, message: 'Ticket ' + ticketId + ' cancelled successfully. Seat ' + ticket.seatNumber + ' released.' };
  }

  searchTicket(ticketId) {
    for (var i = 0; i < this.tickets.length; i++) {
      if (this.tickets[i].ticketId === ticketId) return { success: true, ticket: this.tickets[i] };
    }
    return { success: false, message: 'Ticket not found.' };
  }

  getReport() {
    var totalRevenue = 0, totalBooked = 0, totalCancelled = 0;
    this.tickets.forEach(function(t) {
      if (t.isActive) { totalRevenue += t.fare; totalBooked++; } else totalCancelled++;
    });
    var routeStats = [];
    var keys = Object.keys(this.routes);
    for (var i = 0; i < keys.length; i++) {
      var r = this.routes[keys[i]];
      routeStats.push({ id: r.id, name: r.getStopsString(), booked: r.totalSeats - r.availableSeats, total: r.totalSeats, available: r.availableSeats });
    }
    var mostPopular = routeStats[0];
    for (var j = 1; j < routeStats.length; j++) {
      if (routeStats[j].booked > mostPopular.booked) mostPopular = routeStats[j];
    }
    return { totalRevenue: totalRevenue, totalBooked: totalBooked, totalCancelled: totalCancelled, routeStats: routeStats, mostPopular: mostPopular };
  }

  save() {
    var data = { ticketCounter: this.ticketCounter, tickets: this.tickets, seatMaps: {} };
    var keys = Object.keys(this.routes);
    for (var i = 0; i < keys.length; i++) { data.seatMaps[keys[i]] = this.routes[keys[i]].seatMap; }
    localStorage.setItem('busReservationData', JSON.stringify(data));
  }

  load() {
    var raw = localStorage.getItem('busReservationData');
    if (!raw) return;
    try {
      var data = JSON.parse(raw);
      this.ticketCounter = data.ticketCounter || 0;
      var self = this;
      this.tickets = (data.tickets || []).map(function(t) {
        var ticket = new Ticket(t.ticketId, t.passengerName, t.routeId, t.seatNumber, t.fare);
        if (!t.isActive) ticket.cancel();
        return ticket;
      });
      if (data.seatMaps) {
        Object.keys(data.seatMaps).forEach(function(id) {
          if (self.routes[id]) {
            self.routes[id].seatMap = data.seatMaps[id];
            self.routes[id].availableSeats = self.routes[id].seatMap.filter(function(s) { return !s; }).length;
          }
        });
      }
    } catch (e) { console.error('Failed to load saved data', e); }
  }
}

// ==================== UI Logic ====================
var system;

function init() {
  system = new ReservationSystem(ROUTE_DATA);
  showSection('routes');
}

function showSection(id) {
  document.querySelectorAll('.section').forEach(function(s) { s.classList.add('hidden'); });
  document.getElementById('section-' + id).classList.remove('hidden');
  document.querySelectorAll('.nav-btn').forEach(function(b) { b.classList.remove('active'); });
  var btn = document.querySelector('[data-section="' + id + '"]');
  if (btn) btn.classList.add('active');
  if (id === 'routes') renderRoutes();
  if (id === 'report') renderReport();
  clearOutput();
}

function renderRoutes() {
  var container = document.getElementById('routes-list');
  container.innerHTML = '';
  var keys = Object.keys(system.routes);
  for (var i = 0; i < keys.length; i++) {
    var route = system.routes[keys[i]];
    var card = document.createElement('div');
    card.className = 'route-card';
    card.innerHTML =
      '<div class="route-header"><span class="route-id">' + route.id + '</span><span class="route-name">' + route.name + '</span></div>' +
      '<div class="route-stops">' + route.getStopsString() + '</div>' +
      '<div class="route-meta"><span>🪑 ' + route.availableSeats + '/' + route.totalSeats + ' seats</span><span>💰 ₹' + route.fare + '</span></div>';
    container.appendChild(card);
  }
}

function handleBook(e) {
  e.preventDefault();
  var routeId = document.getElementById('book-route').value;
  var name = document.getElementById('book-name').value.trim();
  if (!name) { showOutput('Please enter passenger name.', 'error'); return; }
  var result = system.bookTicket(routeId, name);
  if (result.success) {
    var t = result.ticket;
    showOutput(
      '✅ Booking Confirmed!\nTicket ID: ' + t.ticketId + '\nPassenger: ' + t.passengerName +
      '\nRoute: ' + t.routeId + ' (' + system.routes[t.routeId].getStopsString() + ')' +
      '\nSeat: ' + t.seatNumber + '\nFare: ₹' + t.fare, 'success'
    );
    document.getElementById('book-name').value = '';
    renderRoutes();
  } else {
    showOutput('❌ ' + result.message, 'error');
  }
}

function handleCancel(e) {
  e.preventDefault();
  var ticketId = document.getElementById('cancel-id').value.trim().toUpperCase();
  if (!ticketId) { showOutput('Please enter a Ticket ID.', 'error'); return; }
  var result = system.cancelTicket(ticketId);
  showOutput(result.success ? '✅ ' + result.message : '❌ ' + result.message, result.success ? 'success' : 'error');
  if (result.success) { document.getElementById('cancel-id').value = ''; renderRoutes(); }
}

function handleSearch(e) {
  e.preventDefault();
  var ticketId = document.getElementById('search-id').value.trim().toUpperCase();
  if (!ticketId) { showOutput('Please enter a Ticket ID.', 'error'); return; }
  var result = system.searchTicket(ticketId);
  if (result.success) {
    var t = result.ticket;
    showOutput(
      '🔍 Ticket Found\nTicket ID: ' + t.ticketId + '\nPassenger: ' + t.passengerName +
      '\nRoute: ' + t.routeId + ' (' + system.routes[t.routeId].getStopsString() + ')' +
      '\nSeat: ' + t.seatNumber + '\nFare: ₹' + t.fare +
      '\nStatus: ' + (t.isActive ? '✅ Active' : '❌ Cancelled'),
      t.isActive ? 'success' : 'warning'
    );
  } else {
    showOutput('❌ ' + result.message, 'error');
  }
}

function renderReport() {
  var report = system.getReport();
  var container = document.getElementById('report-content');
  var html = '<div class="report-grid">' +
    '<div class="stat-card"><div class="stat-value">' + report.totalBooked + '</div><div class="stat-label">Active Bookings</div></div>' +
    '<div class="stat-card"><div class="stat-value">' + report.totalCancelled + '</div><div class="stat-label">Cancellations</div></div>' +
    '<div class="stat-card revenue"><div class="stat-value">₹' + report.totalRevenue + '</div><div class="stat-label">Total Revenue</div></div>' +
    '</div>';

  if (report.mostPopular) {
    html += '<h3>Most Popular Route</h3>' +
      '<div class="route-card" style="border-color:#38bdf8;margin-bottom:1.5rem;">' +
      '<div class="route-header"><span class="route-id">⭐ MOST POPULAR</span>' +
      '<span class="route-name">' + report.mostPopular.id + ' — ' + report.mostPopular.name + '</span></div>' +
      '<div class="route-stops">' + report.mostPopular.booked + ' bookings out of ' + report.mostPopular.total + ' seats</div></div>';
  }

  html += '<h3>Route-wise Occupancy</h3>';
  for (var i = 0; i < report.routeStats.length; i++) {
    var r = report.routeStats[i];
    var pct = (r.booked / r.total) * 100;
    html += '<div class="route-stat"><div class="route-stat-header"><strong>' + r.id + '</strong> — ' + r.name + '</div>' +
      '<div class="progress-bar"><div class="progress-fill" style="width:' + pct + '%"></div></div>' +
      '<small>' + r.booked + '/' + r.total + ' seats booked (' + r.available + ' available)</small></div>';
  }
  container.innerHTML = html;
}

function showOutput(msg, type) {
  var el = document.getElementById('output');
  el.className = 'output ' + type;
  el.textContent = msg;
  el.classList.remove('hidden');
}

function clearOutput() {
  document.getElementById('output').classList.add('hidden');
}

document.addEventListener('DOMContentLoaded', init);
