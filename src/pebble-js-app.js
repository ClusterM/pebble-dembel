var initialized = false;
var options = {
  "config_start_year": 2014,
  "config_start_month": 12,
  "config_start_day": 9,
  "config_start_hour": 12,
  "config_start_min": 0,
  "config_end_year": 2015,
  "config_end_month": 12,
  "config_end_day": 9,
  "config_end_hour": 12,
  "config_end_min": 0,
  "config_text_ostalos": "До дембеля осталось",
  "config_text_uje": "Ты дембель уже",
  "config_text_otslujil": "Ты отслужил"
};

function java_mktime(hour,minute,day,month,year) 
{
    var localOffset = new Date().getTimezoneOffset() * 60;
    return new Date(year, month - 1, day, hour, minute).getTime() / 1000 - localOffset;
}  

function send_config()
{
  var config = {
    "start_time": java_mktime(options["config_start_hour"], options["config_start_min"], options["config_start_day"], options["config_start_month"], options["config_start_year"]),
    "end_time": java_mktime(options["config_end_hour"], options["config_end_min"], options["config_end_day"], options["config_end_month"], options["config_end_year"]),
    "text_ostalos": options["config_text_ostalos"],
    "text_uje": options["config_text_uje"],
    "text_otslujil": options["config_text_otslujil"]
  };
  Pebble.sendAppMessage(config);
}

Pebble.addEventListener("ready", function() {
  initialized = true;
	var json = window.localStorage.getItem('dembel-config');	
	if (typeof json === 'string') {
		try {
      options = JSON.parse(json);
			send_config();
			console.log("Loaded stored config: " + json);
    } catch(e) {
      console.log("stored config json parse error: " + json + ' - ' + e);
    }
  }
});

Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
	var cfg = '?config=' + encodeURI(JSON.stringify(options));
  Pebble.openURL("http://clusterrr.com/pebble_configs/dembel.php" + cfg);
});

Pebble.addEventListener("webviewclosed", function(e) {
	var response = decodeURIComponent(e.response);
  if (response.charAt(0) == "{" && response.slice(-1) == "}" && response.length > 5) {
		try {
			options = JSON.parse(response);
			send_config();
      window.localStorage.setItem('dembel-config', response);
    } catch(e) {
			console.log("Response config json parse error: " + response + ' - ' + e);
		}
    console.log("Options = " + response);
  }
});
