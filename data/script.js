import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.0/firebase-app.js";

import {
  getDatabase,
  ref,
  onValue
} from "https://www.gstatic.com/firebasejs/10.12.0/firebase-database.js";



let socket = new WebSocket("ws://" + location.host + "/ws");

socket.onmessage = function(event)
{
  let data = JSON.parse(event.data);

  let level = data.level;

  document.getElementById("fill").style.height = level + "%";

  document.getElementById("percent").innerText = level + "%";

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

  let statusEl = document.getElementById("status");

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

const app = initializeApp(firebaseConfig);

const db = getDatabase(app);

const logsRef = ref(db, "/tank/logs");

onValue(logsRef, (snapshot) =>
{
  console.log("Firebase data:", snapshot.val());

  const data = snapshot.val();

  const logContainer = document.getElementById("logs");

  logContainer.innerHTML = "<h1>Data Logs</h1>";

  if (!data)
  {
    console.log("No data found");
    return;
  }

  const keys = Object.keys(data).reverse();

  keys.forEach((key) =>
  {
    const item = data[key];

    const date = new Date(Number(key) * 1000);

    const readableTime = date.toLocaleString();

    const div = document.createElement("div");

    div.className = "log-item";

    div.innerText =
      `${readableTime} → ${item.level}%`;

    logContainer.appendChild(div);
  });
});