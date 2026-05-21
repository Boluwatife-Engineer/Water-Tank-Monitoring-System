import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.0/firebase-app.js";

import {
  getDatabase,
  ref,
  onValue
} from "https://www.gstatic.com/firebasejs/10.12.0/firebase-database.js";

import { firebaseConfig } from "./Secrets";

let socket =
  new WebSocket("ws://" + location.host + "/ws");

let chart;

let allLogs = [];

let currentFilter = "all";



socket.onmessage = function(event)
{
  let data = JSON.parse(event.data);

  let level = data.level;

  document.getElementById("fill").style.height =
    level + "%";

  document.getElementById("percent").innerText =
    level + "%";

  let status = "EMPTY";

  let cls = "";

  if (level == 25)
  {
    status = "LOW";
    cls = "low";
  }

  if (level == 50)
  {
    status = "MEDIUM";
    cls = "mid";
  }

  if (level == 75)
  {
    status = "HIGH";
    cls = "high";
  }

  if (level == 100)
  {
    status = "FULL";
    cls = "high";
  }

  let statusEl =
    document.getElementById("status");

  statusEl.innerText = status;

  statusEl.className = cls;
};



const app =
  initializeApp(firebaseConfig);

const db =
  getDatabase(app);

const logsRef =
  ref(db, "/tank/logs");



onValue(logsRef, (snapshot) =>
{
  const data = snapshot.val();

  if (!data) return;

  allLogs = Object.keys(data).map((key) =>
  {
    return {
      timestamp: Number(key),
      level: data[key].level
    };
  });

  allLogs.sort((a, b) =>
    a.timestamp - b.timestamp);

  applyFilter(currentFilter);
});



function renderLogs(logs)
{
  const logsBody =
    document.getElementById("logs-body");

  logsBody.innerHTML = "";

  const latestLogs =
    [...logs].reverse().slice(0, 20);

  latestLogs.forEach((log) =>
  {
    const dateObj =
      new Date(log.timestamp * 1000);

    const date =
      dateObj.toLocaleDateString();

    const time =
      dateObj.toLocaleTimeString();

    const row =
      document.createElement("div");

    row.className = "log-item";

    row.innerHTML = `
      <span>${date}</span>
      <span>${time}</span>
      <span>${log.level}%</span>
    `;

    logsBody.appendChild(row);
  });
}



function renderChart(logs)
{
  const latestLogs =
    logs.slice(-20);

  const labels =
    latestLogs.map((log) =>
    {
      const date =
        new Date(log.timestamp * 1000);

      return date.toLocaleTimeString();
    });

  const levels =
    latestLogs.map((log) =>
      log.level);

  const ctx =
    document.getElementById("levelChart");

  if (chart)
  {
    chart.destroy();
  }

  chart = new Chart(ctx,
  {
    type: "line",

    data:
    {
      labels: labels,

      datasets:
      [
        {
          label: "Water Level",

          data: levels,

          borderColor: "#38bdf8",

          backgroundColor:
            "rgba(56,189,248,0.15)",

          fill: true,

          tension: 0.45,

          cubicInterpolationMode:
            "monotone",

          borderWidth: 4,

          pointRadius: 5,

          pointHoverRadius: 8,

          pointBackgroundColor:
            "#38bdf8",

          pointBorderColor:
            "#ffffff",

          pointBorderWidth: 2,

          pointStyle: "circle"
        }
      ]
    },

    options:
    {
      responsive: true,

      maintainAspectRatio: false,

      animation:
      {
        duration: 700
      },

      plugins:
      {
        legend:
        {
          labels:
          {
            color: "white"
          }
        }
      },

      scales:
      {
        x:
        {
          ticks:
          {
            color: "white"
          },

          grid:
          {
            color:
              "rgba(255,255,255,0.08)"
          }
        },

        y:
        {
          min: 0,

          max: 100,

          ticks:
          {
            color: "white"
          },

          grid:
          {
            color:
              "rgba(255,255,255,0.08)"
          }
        }
      }
    }
  });
}



function applyFilter(filter)
{
  currentFilter = filter;

  let filtered = allLogs;

  if (filter === "low")
  {
    filtered =
      allLogs.filter((log) =>
        log.level <= 25);
  }

  if (filter === "high")
  {
    filtered =
      allLogs.filter((log) =>
        log.level >= 75);
  }

  if (filter === "today")
  {
    const today =
      new Date().toLocaleDateString();

    filtered =
      allLogs.filter((log) =>
      {
        const logDate =
          new Date(log.timestamp * 1000)
          .toLocaleDateString();

        return logDate === today;
      });
  }

  renderLogs(filtered);

  renderChart(filtered);
}



const buttons =
  document.querySelectorAll(".filters button");

buttons.forEach((button) =>
{
  button.addEventListener("click", () =>
  {
    const filter =
      button.dataset.filter;

    applyFilter(filter);
  });
});