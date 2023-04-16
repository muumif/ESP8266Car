var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

function onLoad(event) {
      initWebSocket();
}

function getMPUValues() {
      websocket.send("getMPUValues");
}

function initWebSocket() {
      console.log("Opening a WebSocket connection...");
      websocket = new WebSocket(gateway);
      websocket.onopen = onOpen;
      websocket.onclose = onClose;
      websocket.onmessage = onMessage;
}

function onOpen(event) {
      console.log("Connection opened");
      // setInterval(getMPUValues, 500);
}

function onClose(event) {
      console.log("Connection closed");
      setTimeout(initWebSocket, 2000);
}

const zeroToOneButton = document.getElementById('zeroToOneButton');

var intervalId;
var zeroToOneStartTime;
function onZeroToOneButtonClicked() {
      zeroToOneStartTime = new Date();
      zeroToOneButton.disabled = true;
      intervalId = setInterval(function() {
            websocket.send("getGPSSpeedTop");
      }, 50);
}
zeroToOneButton.onclick = onZeroToOneButtonClicked;

var lastSpeed = 0;
var topSpeed = 0;

var topSpeedZero = 0;

var zeroToOneEndTime;
function onMessage(event) {
      var data = JSON.parse(event.data);
      console.log(data["speed"]["speed"]);
      if (data["speed"]["speed"] != undefined) {
            topSpeed = data["speed"]["speed"];
            if (topSpeed > lastSpeed) {
                  document.getElementById("speedText").innerText = "Speed: " + topSpeed;
                  lastSpeed = topSpeed;
            }        
      } else if (data["speed"]["top"] != undefined) {
            topSpeedZero = data["speed"]["top"];
            console.log(topSpeedZero);
            if (topSpeedZero >= 100) {
                  zeroToOneEndTime = new Date();
                  zeroToOneButton.disabled = false;
                  clearInterval(intervalId);
                  document.getElementById("zeroToOneText").innerText = "Speed: " + topSpeedZero;
                  document.getElementById("zeroToOneTextTime").innerText = "Time: " + (zeroToOneEndTime - zeroToOneStartTime) / 1000 + "s";
            }
      }

      // document.getElementById("accelX").innerHTML = Number(data.mpu6050.accelX).toFixed(2);
      // document.getElementById("accelY").innerHTML = Number(data.mpu6050.accelY).toFixed(2);
      // document.getElementById("accelZ").innerHTML = Number(data.mpu6050.accelZ).toFixed(2);
      // document.getElementById("rotX").innerHTML = Number(data.mpu6050.rotX).toFixed(2);
      // document.getElementById("rotY").innerHTML = Number(data.mpu6050.rotY).toFixed(2);
      // document.getElementById("rotZ").innerHTML = Number(data.mpu6050.rotZ).toFixed(2);
}

const topSpeedButton = document.getElementById('topSpeedButton');

var refreshIntervalId;
var startTime;
var endTime;
function onTopSpeedButtonClicked() {
      if (topSpeedButton.innerHTML == "Start") {
            startTime = new Date();
            topSpeedButton.innerHTML = "Stop";
            refreshIntervalId = setInterval(function() {
                  websocket.send("getGPSSpeed")
            }, 200);
      } else {
            topSpeedButton.innerHTML = "Start";
            clearInterval(refreshIntervalId);
            endTime = new Date();
            document.getElementById("speedTextTime").innerText = "Time: " + (endTime - startTime) / 1000 + "s";
      }
}
topSpeedButton.onclick = onTopSpeedButtonClicked;