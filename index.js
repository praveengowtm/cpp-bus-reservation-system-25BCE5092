// Bus Reservation System - JavaScript Logic
// Mirrors the C++ OOP structure using classes and localStorage for persistence

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

  getStopsString() {
    return this.stops.join(' → ');
  }

  bookSeat() {
    for (let i = 0; i < this.totalSeats; i++) {
      if (!this.seatMap[i]) {
        this.seatMap[i] = true;
        this.availableSeats--;
        return i + 1;
      }
    }
    return -1;
  }

  cancelSeat(seatNumber) {
    const idx = seatNumber - 1;
    if (idx >= 0 && idx < this.totalSeats && this.seatMap[idx]) {
      this.seatMap[idx] = false;
      this.availableSeats++;
      return true;
    }
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

  cancel() {
    this.isActive = false;
  }
}

class ReservationSystem {
  constructor(routeData) {
    this.routes = {};
    this.tickets = [];
    this.ticketCounter = 0;

    routeData.forEach(r => {
      this.routes[r.id] = new Route(r.id, r.name, r.stops, r.totalSeats, r.fare);
    });

    this.load();
  }

  generateTicketId() {
    this.ticketCounter++;
    return 'TKT' + this.ticketCounter;
  }

  bookTicket(routeId, passengerName) {
    const route = this.routes[routeId];
    if (!route) return { success: false, message: 'Route not found!' };

    const seatNum = route.bookSeat();
    if (seatNum === -1) return { success: false, message: 'No seats available on this route!' };

    const ticketId = this.generateTicketId();
    const ticket = new Ticket(ticketId, passengerName, routeId, seatNum, route.fare);
    this.tickets.push(ticket);
    this.save();

    return { success: true, ticket };
  }

  cancelTicket(ticketId) {
    const ticket = this.tickets.find(t => t.ticketId === ticketId && t.isActive);
    if (!ticket) return { success: false, message: 'Ticket not found or already cancelled.' };

    const route = this.routes[ticket.routeId];
    if (route) route.cancelSeat(ticket.seatNumber);
    ticket.cancel();
    this.save();

    return { success: true, message: `Ticket ${ticketId} cancelled successfully. Seat ${ticket.seatNumber} released.` };
  }

  searchTicket(ticketId) {
    const ticket = this.tickets.find(t => t.ticketId === ticketId);
    if (!ticket) return { success: false, message: 'Ticket not found.' };
    return { success: true, ticket };
  }

  getReport() {
    let totalRevenue = 0, totalBooked = 0, totalCancelled = 0;

    this.tickets.forEach(t => {
      if (t.isActive) { totalRevenue += t.fare; totalBooked++; }
      else totalCancelled++;
    });

    const routeStats = Object.values(this.routes).map(r => ({
      id: r.id,
      name: r.getStopsString(),
      booked: r.totalSeats - r.availableSeats,
      total: r.totalSeats,
      available: r.availableSeats
    }));

    // Find most popular route
    let mostPopular = routeStats[0];
    routeStats.forEach(r => { if (r.booked > mostPopular.booked) mostPopular = r; });

    return { totalRevenue, totalBooked, totalCancelled, routeStats, mostPopular };
  }

  save() {
    const data = {
      ticketCounter: this.ticketCounter,
      tickets: this.tickets,
      seatMaps: {}
    };
    Object.keys(this.routes).forEach(id => {
      data.seatMaps[id] = this.routes[id].seatMap;
    });
    localStorage.setItem('busReservationData', JSON.stringify(data));
  }

  load() {
    const raw = localStorage.getItem('busReservationData');
    if (!raw) return;

    try {
      const data = JSON.parse(raw);
      this.ticketCounter = data.ticketCounter || 0;
      this.tickets = (data.tickets || []).map(t => {
        const ticket = new Ticket(t.ticketId, t.passengerName, t.routeId, t.seatNumber, t.fare);
        if (!t.isActive) ticket.cancel();
        return ticket;
      });
      if (data.seatMaps) {
        Object.keys(data.seatMaps).forEach(id => {
          if (this.routes[id]) {
            this.routes[id].seatMap = data.seatMaps[id];
            this.routes[id].availableSeats = this.routes[id].seatMap.filter(s => !s).length;
          }
        });
      }
    } catch (e) {
      console.error('Failed to load saved data', e);
    }
  }
}

// ==================== UI Logic ====================
let system;

async function init() {
  const resp = await fetch('index.json');
  const config = await resp.json();
  system = new ReservationSystem(config.routes);
  showSection('routes');
  renderRoutes();
}

function showSection(id) {
  document.querySelectorAll('.section').forEach(s => s.classList.add('hidden'));
  document.getElementById('section-' + id).classList.remove('hidden');
  document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));
  const btn = document.querySelector(`[data-section="${id}"]`);
  if (btn) btn.classList.add('active');

  if (id === 'routes') renderRoutes();
  if (id === 'report') renderReport();
  clearOutput();
}

function renderRoutes() {
  const container = document.getElementById('routes-list');
  container.innerHTML = '';
  Object.values(system.routes).forEach(route => {
    const card = document.createElement('div');
    card.className = 'route-card';
    card.innerHTML = `
      <div class="route-header">
        <span class="route-id">${route.id}</span>
        <span class="route-name">${route.name}</span>
      </div>
      <div class="route-stops">${route.getStopsString()}</div>
      <div class="route-meta">
        <span>🪑 ${route.availableSeats}/${route.totalSeats} seats</span>
        <span>💰 ₹${route.fare}</span>
      </div>
    `;
    container.appendChild(card);
  });
}

function handleBook(e) {
  e.preventDefault();
  const routeId = document.getElementById('book-route').value;
  const name = document.getElementById('book-name').value.trim();

  if (!name) { showOutput('Please enter passenger name.', 'error'); return; }

  const result = system.bookTicket(routeId, name);
  if (result.success) {
    const t = result.ticket;
    showOutput(
      `✅ Booking Confirmed!\nTicket ID: ${t.ticketId}\nPassenger: ${t.passengerName}\nRoute: ${t.routeId} (${system.routes[t.routeId].getStopsString()})\nSeat: ${t.seatNumber}\nFare: ₹${t.fare}`,
      'success'
    );
    document.getElementById('book-name').value = '';
    renderRoutes();
  } else {
    showOutput('❌ ' + result.message, 'error');
  }
}

function handleCancel(e) {
  e.preventDefault();
  const ticketId = document.getElementById('cancel-id').value.trim().toUpperCase();
  if (!ticketId) { showOutput('Please enter a Ticket ID.', 'error'); return; }

  const result = system.cancelTicket(ticketId);
  showOutput(result.success ? '✅ ' + result.message : '❌ ' + result.message, result.success ? 'success' : 'error');
  if (result.success) {
    document.getElementById('cancel-id').value = '';
    renderRoutes();
  }
}

function handleSearch(e) {
  e.preventDefault();
  const ticketId = document.getElementById('search-id').value.trim().toUpperCase();
  if (!ticketId) { showOutput('Please enter a Ticket ID.', 'error'); return; }

  const result = system.searchTicket(ticketId);
  if (result.success) {
    const t = result.ticket;
    showOutput(
      `🔍 Ticket Found\nTicket ID: ${t.ticketId}\nPassenger: ${t.passengerName}\nRoute: ${t.routeId} (${system.routes[t.routeId].getStopsString()})\nSeat: ${t.seatNumber}\nFare: ₹${t.fare}\nStatus: ${t.isActive ? '✅ Active' : '❌ Cancelled'}`,
      t.isActive ? 'success' : 'warning'
    );
  } else {
    showOutput('❌ ' + result.message, 'error');
  }
}

function renderReport() {
  const report = system.getReport();
  const container = document.getElementById('report-content');
  container.innerHTML = `
    <div class="report-grid">
      <div class="stat-card"><div class="stat-value">${report.totalBooked}</div><div class="stat-label">Active Bookings</div></div>
      <div class="stat-card"><div class="stat-value">${report.totalCancelled}</div><div class="stat-label">Cancellations</div></div>
      <div class="stat-card revenue"><div class="stat-value">₹${report.totalRevenue}</div><div class="stat-label">Total Revenue</div></div>
    </div>
    <h3>Route-wise Occupancy</h3>
    <div class="route-card" style="border-color:#38bdf8;margin-bottom:1.5rem;">
      <div class="route-header">
        <span class="route-id">⭐ MOST POPULAR</span>
        <span class="route-name">${report.mostPopular.id} — ${report.mostPopular.name}</span>
      </div>
      <div class="route-stops">${report.mostPopular.booked} bookings out of ${report.mostPopular.total} seats</div>
    </div>
    <h3>Route-wise Occupancy</h3>
    ${report.routeStats.map(r => `
      <div class="route-stat">
        <div class="route-stat-header">
          <strong>${r.id}</strong> — ${r.name}
        </div>
        <div class="progress-bar">
          <div class="progress-fill" style="width:${(r.booked/r.total)*100}%"></div>
        </div>
        <small>${r.booked}/${r.total} seats booked (${r.available} available)</small>
      </div>
    `).join('')}
  `;
}

function showOutput(msg, type) {
  const el = document.getElementById('output');
  el.className = 'output ' + type;
  el.textContent = msg;
  el.classList.remove('hidden');
}

function clearOutput() {
  const el = document.getElementById('output');
  el.classList.add('hidden');
}

document.addEventListener('DOMContentLoaded', init);
