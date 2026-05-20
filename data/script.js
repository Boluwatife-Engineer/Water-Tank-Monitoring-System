import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.0/firebase-app.js";

import {
  getDatabase,
  ref,
  onValue
} from "https://www.gstatic.com/firebasejs/10.12.0/firebase-database.js";



let socket =
  new WebSocket("ws://" + location.host + "/ws");

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

const app =
  initializeApp(firebaseConfig);

const db =
  getDatabase(app);

const logsRef =
  ref(db, "/tank/logs");



let allLogs = [];



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

  allLogs.reverse();

  renderLogs(allLogs);
});



function renderLogs(logs)
{
  const logsBody =
    document.getElementById("logs-body");

  logsBody.innerHTML = "";

  logs.forEach((log) =>
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



const buttons =
  document.querySelectorAll(".filters button");

buttons.forEach((button) =>
{
  button.addEventListener("click", () =>
  {
    const filter =
      button.dataset.filter;

    if (filter === "all")
    {
      renderLogs(allLogs);
    }
    if (filter === "low")
    {
      const filtered =
        allLogs.filter(log => log.level <= 25);

      renderLogs(filtered);
    }

    if (filter === "high")
    {
      const filtered =
        allLogs.filter(log => log.level >= 75);

      renderLogs(filtered);
    }

    if (filter === "today")
    {
      const today =
        new Date().toLocaleDateString();

      const filtered =
        allLogs.filter((log) =>
        {
          const logDate =
            new Date(log.timestamp * 1000)
            .toLocaleDateString();

          return logDate === today;
        });

      renderLogs(filtered);
    }
  });
});