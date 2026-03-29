// ─── Bus Reservation System - JavaScript Logic ───

let routes = [];
let tickets = [];
let ticketCounter = 0;

// Load route config from index.json
async function loadConfig() {
  try {
    const res = await fetch("index.json");
    const data = await res.json();
    routes = data.routes.map(r => ({
      ...r,
      availableSeats: r.totalSeats
    }));
  } catch (e) {
    // Fallback if fetch fails
    routes = [
      { id: "R1", name: "Chennai - Coimbatore Express", stops: ["Chennai", "Salem", "Coimbatore"], fare: 450, totalSeats: 30, availableSeats: 30 },
      { id: "R2", name: "Chennai - Nagapattinam Express", stops: ["Chennai", "Mayiladuthurai", "Nagapattinam"], fare: 350, totalSeats: 30, availableSeats: 30 }
    ];
  }
  loadTicketsFromStorage();
  renderRoutes();
}

function generateTicketId() {
  ticketCounter++;
  return "TKT" + (1000 + ticketCounter);
}

// ─── Persistence (localStorage) ───
function saveTicketsToStorage() {
  localStorage.setItem("busTickets", JSON.stringify(tickets));
  localStorage.setItem("busTicketCounter", ticketCounter);
}

function loadTicketsFromStorage() {
  const saved = localStorage.getItem("busTickets");
  const counter = localStorage.getItem("busTicketCounter");
  if (saved) {
    tickets = JSON.parse(saved);
    ticketCounter = parseInt(counter) || 0;
    // Recalculate available seats
    tickets.forEach(t => {
      if (t.active) {
        const route = routes.find(r => r.id === t.routeId);
        if (route) route.availableSeats--;
      }
    });
  }
}

// ─── UI Rendering ───
function showSection(id) {
  document.querySelectorAll(".section").forEach(s => s.classList.add("hidden"));
  document.getElementById(id).classList.remove("hidden");
}

function renderRoutes() {
  const container = document.getElementById("routesList");
  container.innerHTML = routes.map(r => `
    <div class="route-card">
      <div class="route-header">
        <span class="route-id">${r.id}</span>
        <span class="route-name">${r.name}</span>
      </div>
      <div class="route-stops">${r.stops.join(" → ")}</div>
      <div class="route-info">
        <span>Fare: <strong>₹${r.fare}</strong></span>
        <span class="seat-badge ${r.availableSeats === 0 ? 'full' : ''}">
          ${r.availableSeats}/${r.totalSeats} seats
        </span>
      </div>
    </div>
  `).join("");
}

// ─── View Routes ───
function viewRoutes() {
  renderRoutes();
  showSection("routesSection");
}

// ─── Book Ticket ───
function showBookForm() {
  const sel = document.getElementById("bookRoute");
  sel.innerHTML = routes.map(r =>
    `<option value="${r.id}" ${r.availableSeats === 0 ? 'disabled' : ''}>
      ${r.id}: ${r.name} (${r.availableSeats} seats) - ₹${r.fare}
    </option>`
  ).join("");
  document.getElementById("bookName").value = "";
  document.getElementById("bookResult").innerHTML = "";
  showSection("bookSection");
}

function bookTicket() {
  const rid = document.getElementById("bookRoute").value;
  const name = document.getElementById("bookName").value.trim();
  const resultDiv = document.getElementById("bookResult");

  if (!name) {
    resultDiv.innerHTML = `<div class="msg error">Please enter passenger name.</div>`;
    return;
  }

  const route = routes.find(r => r.id === rid);
  if (!route || route.availableSeats <= 0) {
    resultDiv.innerHTML = `<div class="msg error">No seats available on this route.</div>`;
    return;
  }

  const seat = route.totalSeats - route.availableSeats + 1;
  route.availableSeats--;

  const tid = generateTicketId();
  const ticket = { ticketId: tid, passengerName: name, routeId: rid, seatNumber: seat, fare: route.fare, active: true };
  tickets.push(ticket);
  saveTicketsToStorage();

  resultDiv.innerHTML = `
    <div class="msg success">✓ Booking Confirmed!</div>
    <div class="ticket-card">
      <div class="ticket-row"><span>Ticket ID:</span><strong>${tid}</strong></div>
      <div class="ticket-row"><span>Passenger:</span><strong>${name}</strong></div>
      <div class="ticket-row"><span>Route:</span><strong>${route.name}</strong></div>
      <div class="ticket-row"><span>Seat:</span><strong>#${seat}</strong></div>
      <div class="ticket-row"><span>Fare:</span><strong>₹${route.fare}</strong></div>
    </div>
  `;
  renderRoutes();
}

// ─── Cancel Ticket ───
function showCancelForm() {
  document.getElementById("cancelId").value = "";
  document.getElementById("cancelResult").innerHTML = "";
  showSection("cancelSection");
}

function cancelTicket() {
  const tid = document.getElementById("cancelId").value.trim().toUpperCase();
  const resultDiv = document.getElementById("cancelResult");

  const ticket = tickets.find(t => t.ticketId === tid && t.active);
  if (!ticket) {
    resultDiv.innerHTML = `<div class="msg error">Ticket not found or already cancelled.</div>`;
    return;
  }

  ticket.active = false;
  const route = routes.find(r => r.id === ticket.routeId);
  if (route) route.availableSeats++;
  saveTicketsToStorage();

  resultDiv.innerHTML = `<div class="msg success">✓ Ticket ${tid} cancelled successfully.</div>`;
  renderRoutes();
}

// ─── Search Ticket ───
function showSearchForm() {
  document.getElementById("searchId").value = "";
  document.getElementById("searchResult").innerHTML = "";
  showSection("searchSection");
}

function searchTicket() {
  const tid = document.getElementById("searchId").value.trim().toUpperCase();
  const resultDiv = document.getElementById("searchResult");

  const ticket = tickets.find(t => t.ticketId === tid);
  if (!ticket) {
    resultDiv.innerHTML = `<div class="msg error">Ticket not found.</div>`;
    return;
  }

  const route = routes.find(r => r.id === ticket.routeId);
  resultDiv.innerHTML = `
    <div class="ticket-card">
      <div class="ticket-row"><span>Ticket ID:</span><strong>${ticket.ticketId}</strong></div>
      <div class="ticket-row"><span>Passenger:</span><strong>${ticket.passengerName}</strong></div>
      <div class="ticket-row"><span>Route:</span><strong>${route ? route.name : ticket.routeId}</strong></div>
      <div class="ticket-row"><span>Seat:</span><strong>#${ticket.seatNumber}</strong></div>
      <div class="ticket-row"><span>Fare:</span><strong>₹${ticket.fare}</strong></div>
      <div class="ticket-row"><span>Status:</span>
        <strong class="${ticket.active ? 'active-status' : 'cancelled-status'}">
          ${ticket.active ? '● Active' : '● Cancelled'}
        </strong>
      </div>
    </div>
  `;
}

// ─── Revenue Report ───
function showRevenue() {
  const container = document.getElementById("revenueContent");
  let totalRevenue = 0;
  const routeData = {};

  routes.forEach(r => { routeData[r.id] = { name: r.name, bookings: 0, revenue: 0 }; });

  tickets.forEach(t => {
    if (t.active && routeData[t.routeId]) {
      routeData[t.routeId].bookings++;
      routeData[t.routeId].revenue += t.fare;
      totalRevenue += t.fare;
    }
  });

  container.innerHTML = Object.keys(routeData).map(id => `
    <div class="revenue-row">
      <div><strong>${id}</strong> — ${routeData[id].name}</div>
      <div>Bookings: <strong>${routeData[id].bookings}</strong> &nbsp;|&nbsp; Revenue: <strong>₹${routeData[id].revenue.toFixed(2)}</strong></div>
    </div>
  `).join("") + `
    <div class="revenue-total">Total Revenue: <strong>₹${totalRevenue.toFixed(2)}</strong></div>
  `;

  showSection("revenueSection");
}

// ─── Init ───
window.addEventListener("DOMContentLoaded", loadConfig);
