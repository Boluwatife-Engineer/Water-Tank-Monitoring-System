import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.0/firebase-app.js";

import {
  getDatabase,
  ref,
  onValue
} from "https://www.gstatic.com/firebasejs/10.12.0/firebase-database.js";

const firebaseConfig = {
  apiKey: "AIzaSyC6OE3-jl73z41B4rWEO699fBBaqJPQPRI",
  authDomain: "water-tank-monitoring-sy-9fbf3.firebaseapp.com",
  databaseURL: "https://water-tank-monitoring-sy-9fbf3-default-rtdb.firebaseio.com",
  projectId: "water-tank-monitoring-sy-9fbf3",
  storageBucket: "water-tank-monitoring-sy-9fbf3.firebasestorage.app",
  messagingSenderId: "250565556537",
  appId: "1:250565556537:web:428aec758e7c0155b7d0e8",
  measurementId: "G-XQVNN1GPRM"
};

const app = initializeApp(firebaseConfig);

const db = getDatabase(app);

let chart;

let allLogs = [];

let currentFilter = "all";



const levelRef = ref(db, "/tank/current");

onValue(levelRef, (snapshot) => {

  const level = snapshot.val() || 0;

  document.getElementById("fill").style.height =
    level + "%";

  document.getElementById("percent").innerText =
    level + "%";

  let status = "EMPTY";

  let cls = "";

  if(level <= 25){
    status = "LOW";
    cls = "low";
  }

  if(level === 50){
    status = "MEDIUM";
    cls = "mid";
  }

  if(level >= 75){
    status = "HIGH";
    cls = "high";
  }

  document.getElementById("status").innerText =
    status;

  document.getElementById("status").className =
    cls;
});



const analyticsRef =
  ref(db, "/tank/analytics");

onValue(analyticsRef, (snapshot) => {

  const data = snapshot.val();

  if(!data) return;

  document.getElementById("highest").innerText =
    data.highest + "%";

  document.getElementById("lowest").innerText =
    data.lowest + "%";

  document.getElementById("average").innerText =
    Number(data.average).toFixed(1) + "%";

  document.getElementById("changes").innerText =
    data.changes;
});



const logsRef =
  ref(db, "/tank/logs");

onValue(logsRef, (snapshot) => {

  const data = snapshot.val();

  if(!data) return;

  allLogs = Object.keys(data).map((key) => {

    return {
      timestamp: Number(key),
      level: data[key].level
    };

  });

  allLogs.sort((a, b) =>
    a.timestamp - b.timestamp
  );

  applyFilter(currentFilter);
});



function renderLogs(logs){

  const body =
    document.getElementById("logs-body");

  body.innerHTML = "";

  maintainAspectRatio: false
  logs
    .slice(-20)
    .reverse()
    .forEach((log) => {

      const date =
        new Date(log.timestamp * 1000);

      const row =
        document.createElement("div");

      row.className = "log-item";

      row.innerHTML = `
        <span>${date.toLocaleDateString()}</span>
        <span>${date.toLocaleTimeString()}</span>
        <span>${log.level}%</span>
      `;

      body.appendChild(row);
    });
}



function renderChart(logs){

  const ctx =
    document.getElementById("levelChart");

  const latest =
    logs.slice(-20);

  const labels =
    latest.map((log) =>
      new Date(log.timestamp * 1000)
        .toLocaleTimeString()
    );

  const values =
    latest.map((log) =>
      log.level
    );

  if(chart){
    chart.destroy();
  }

  chart = new Chart(ctx, {

    type: "line",

    data: {

      labels: labels,

      datasets: [{
        label: "Water Level",

        data: values,

        borderColor: "#38bdf8",

        backgroundColor:
          "rgba(56,189,248,0.15)",

        fill: true,

        tension: 0.4,

        borderWidth: 3,

        pointRadius: 4
      }]
    },

    options: {

      responsive: true,

      maintainAspectRatio: false,

      scales: {

        y: {

          min: 0,

          max: 100,

          ticks: {
            stepSize: 25,
            color: "white"
          },

          grid: {
            color:
              "rgba(255,255,255,0.08)"
          }
        },

        x: {

          ticks: {
            color: "white"
          },

          grid: {
            color:
              "rgba(255,255,255,0.08)"
          }
        }
      },

      plugins: {

        legend: {

          labels: {
            color: "white"
          }
        }
      }
    }
  });
}



function applyFilter(filter){

  currentFilter = filter;

  let filtered = allLogs;

  if(filter === "low"){
    filtered =
      allLogs.filter((log) =>
        log.level <= 25
      );
  }

  if(filter === "high"){
    filtered =
      allLogs.filter((log) =>
        log.level >= 75
      );
  }

  if(filter === "today"){

    const today =
      new Date().toLocaleDateString();

    filtered =
      allLogs.filter((log) => {

        return (
          new Date(log.timestamp * 1000)
            .toLocaleDateString() === today
        );

      });
  }

  renderLogs(filtered);

  renderChart(filtered);
}



document
  .querySelectorAll(".filters button")
  .forEach((button) => {

    button.onclick = () => {

      applyFilter(
        button.dataset.filter
      );

    };

  });