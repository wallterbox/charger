// Get current sensor readings when the page loads  
window.addEventListener('load', getReadings);

// Create chargingpower Gauge
var gaugeHum = new RadialGauge({
  renderTo: 'gauge-chargingpower',
  width: 300,
  height: 300,
  units: "Ladeleistung (Watt)",
  minValue: 0,
  maxValue: 11000,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueDec: 0,
  valueInt: 1,
  majorTicks: [
      "0",
      "1000",
      "2000",
      "3000",
      "4000",
      "5000",
      "6000",
      "7000",
      "8000",
      "9000",
      "10000",
      "11000"

  ],
  minorTicks: 12,
  strokeTicks: true,
  highlights: [
      {
          "from": 4140,
          "to": 11000,
          "color": "#dc7633"
      }
,
      {
          "from": 2740,
          "to": 7360,
          "color": "#f5b041"
      }
,
      {
          "from": 1380,
          "to": 3690,
          "color": "#f7dc6f "
      }
,
      {
          "from": 10000,
          "to": 11000,
          "color": "#03C0C1"
      }
  ],
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#007F80",
  colorNeedleEnd: "#007F80",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#007F80",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1000,
  animationRule: "linear"
}).draw();


				setInterval(function ( ) {
					  var xhr = new XMLHttpRequest();
					  xhr.onreadystatechange = function() {
						if (this.readyState == 4 && this.status == 200) {
						  var myObj = JSON.parse(this.responseText);
						  console.log(myObj);
						  //var temp = myObj.temperature;
						  var hum = myObj.chargingpower;
						  //gaugeTemp.value = temp;
						  gaugeHum.value = hum;
						}
					  }; 
					  xhr.open("GET", "/readings", true);
					  xhr.send();
				
				}, 3000 ) ;


// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      //var temp = myObj.temperature;
      var hum = myObj.chargingpower;
      //gaugeTemp.value = temp;
      gaugeHum.value = hum;
    }
  }; 
  xhr.open("GET", "/readings", true);
  xhr.send();
}

if (!!window.EventSource) {
  var source = new EventSource('/events');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  
  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    //gaugeTemp.value = myObj.temperature;
    gaugeHum.value = myObj.chargingpower;
  }, false);
}