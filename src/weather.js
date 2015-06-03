var xhrRequest = function(url, type, callback){
  var xhr = new XMLHttpRequest();
  xhr.onload = function(){
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

// request weather
function locationSuccess(pos){
  // build url
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + '&lon=' + pos.coords.longitude;
  // send request to openweathermap
  xhrRequest(url, 'GET',
            function(responseText){
              var json = json.parse(responseText);
              // temperature, kelvin to celsius
              var temperature = Math.round(json.main.temp - 273.15);
              console.log('Temperature: ', temperature);
              
              // conditions
              var conditions = json.weather[0].main;
              console.log('Condition: ', conditions);
              
              // dictionary using keys
              var dictionary = {"KEY_TEMPERATURE": temperature,
                                "KEY_CONDITIONS": conditions};

              Pebble.sendAppMessage(dictionary,
                     function(e){
                       console.log("Weather data sent to Pebble successfully");
                     }, function(e){
                       console.log("Error sending weather data to Pebble");
                     });
            });
}

function locationError(err){
  console.log('Error requesting location');
}

function getWeather(){
  navigator.geoLocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// listener for when watchface is opened
Pebble.addEventListener('ready',
                       function(e){
                         console.log('PebbleJS ready');
                         getWeather();
                       });

// listener for AppMessage
Pebble.addEventListener('appmessage',
                       function(e){
                         console.log('AppMessage received');
                         getWeather();
                       });








