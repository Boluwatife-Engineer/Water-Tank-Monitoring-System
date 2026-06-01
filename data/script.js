import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.0/firebase-app.js";
import {
  getDatabase,
  ref,
  onValue
} from "https://www.gstatic.com/firebasejs/10.12.0/firebase-database.js";

const firebaseConfig = {
  apiKey: "AIzaSyC6OE3-jl73z41bRWEO699fBBaqJPQPRI",
  authDomain: "water-tank-monitoring-sy-9fbf3.firebaseapp.com",
  databaseURL: "https://water-tank-monitoring-sy-9fbf3-default-rtdb.firebaseio.com",
  projectId: "water-tank-monitoring-sy-9fbf3",
  storageBucket: "water-tank-monitoring-sy-9fbf3.appspot.com",
  messagingSenderId: "250565556537",
  appId: "1:250565556537:web:428aec758e7c0155b7d0e8"
};

const app = initializeApp(firebaseConfig);
const db = getDatabase(app);

let chart;
let allLogs = [];
let currentFilter = "all";

/* FIXED PATH */
const levelRef = ref(db, "/tank/current");

onValue(levelRef, (snap) => {
  const level = snap.val() || 0;

  document.getElementById("fill").style.height = level + "%";
  document.getElementById("percent").innerText = level + "%";

  let status = "EMPTY";
  let cls = "";

  if (level <= 25) { status = "LOW"; cls = "low"; }
  else if (level <= 50) { status = "MEDIUM"; cls = "mid"; }
  else { status = "HIGH"; cls = "high"; }

  document.getElementById("status").innerText = status;
  document.getElementById("status").className = cls;
});

/* ANALYTICS */
const analyticsRef = ref(db, "/tank/analytics");

onValue(analyticsRef, (snap) => {
  const d = snap.val();
  if (!d) return;

  document.getElementById("highest").innerText = (d.highest ?? 0) + "%";
  document.getElementById("lowest").innerText = (d.lowest ?? 0) + "%";
  document.getElementById("average").innerText = (d.average ?? 0).toFixed(1) + "%";
  document.getElementById("changes").innerText = d.changes ?? 0;
});

/* LOGS */
const logsRef = ref(db, "/tank/logs");

onValue(logsRef, (snap) => {
  const data = snap.val();
  if (!data) return;

  allLogs = Object.keys(data).map(k => ({
    timestamp: Number(k),
    level: data[k].level
  }));

  allLogs.sort((a, b) => a.timestamp - b.timestamp);

  applyFilter(currentFilter);
});

function renderLogs(logs) {
  const body = document.getElementById("logs-body");
  body.innerHTML = "";

  logs.slice(-20).reverse().forEach(l => {
    const d = new Date(l.timestamp * 1000);

    const div = document.createElement("div");
    div.className = "log-item";

    div.innerHTML = `
      <span>${d.toLocaleTimeString()}</span>
      <span>${l.level}%</span>
    `;

    body.appendChild(div);
  });
}

function renderChart(logs) {
  const ctx = document.getElementById("levelChart");

  const last = logs.slice(-20);

  const labels = last.map(l =>
    new Date(l.timestamp * 1000).toLocaleTimeString()
  );

  const values = last.map(l => l.level);

  if (chart) chart.destroy();

  chart = new Chart(ctx, {
    type: "line",
    data: {
      labels,
      datasets: [{
        label: "Level",
        data: values,
        borderColor: "#38bdf8",
        fill: true
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: { min: 0, max: 100 }
      }
    }
  });
}

function applyFilter(f) {
  currentFilter = f;

  let filtered = allLogs;

  if (f === "low") filtered = allLogs.filter(x => x.level <= 25);
  if (f === "high") filtered = allLogs.filter(x => x.level >= 75);

  if (f === "today") {
    const today = new Date().toDateString();
    filtered = allLogs.filter(x =>
      new Date(x.timestamp * 1000).toDateString() === today
    );
  }

  renderLogs(filtered);
  renderChart(filtered);
}

document.querySelectorAll(".filters button")
  .forEach(btn => {
    btn.onclick = () => applyFilter(btn.dataset.filter);
  });