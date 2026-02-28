var Clay = require('pebble-clay');

// Define config directly in index.js to avoid loading issues in PKJS
var clayConfig = [
  { "type": "heading", "defaultValue": "HamBands Settings" },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Language Override" },
      {
        "type": "select",
        "messageKey": "LANG",
        "defaultValue": "system",
        "label": "Select Language",
        "options": [
          { "label": "System Default", "value": "system" },
          { "label": "English", "value": "en" },
          { "label": "Español", "value": "es" },
          { "label": "Français", "value": "fr" },
          { "label": "Italiano", "value": "it" },
          { "label": "Português", "value": "pt" }
        ]
      }
    ]
  },
  { "type": "submit", "defaultValue": "Save Settings" }
];

var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

function fetchSolarData() {
  console.log('Fetching solar data...');
  var url = 'https://www.hamqsl.com/solarxml.php';
  var xhr = new XMLHttpRequest();
  xhr.timeout = 10000; // 10s timeout

  xhr.onload = function () {
    console.log('XHR Success');
    var xml = this.responseText;

    var sfiMatch = xml.match(/<solarflux>([^<]+)<\/solarflux>/);
    var kindexMatch = xml.match(/<kindex>([^<]+)<\/kindex>/);
    var updatedMatch = xml.match(/<updated>([^<]+)<\/updated>/);

    if (!sfiMatch || !kindexMatch || !updatedMatch) {
      console.log('Parsing failed - unexpected XML format');
      return;
    }

    function getBand(name, time) {
      var regex = new RegExp('<band name="' + name + '" time="' + time + '">([^<]+)<\\/band>');
      var match = xml.match(regex);
      return match ? match[1].trim() : 'Poor';
    }

    var dict = {
      'SFI': parseInt(sfiMatch[1]),
      'K_INDEX': parseInt(kindexMatch[1]),
      'BAND_80_40_D': getBand('80m-40m', 'day'),
      'BAND_30_20_D': getBand('30m-20m', 'day'),
      'BAND_17_15_D': getBand('17m-15m', 'day'),
      'BAND_12_10_D': getBand('12m-10m', 'day'),
      'BAND_80_40_N': getBand('80m-40m', 'night'),
      'BAND_30_20_N': getBand('30m-20m', 'night'),
      'BAND_17_15_N': getBand('17m-15m', 'night'),
      'BAND_12_10_N': getBand('12m-10m', 'night'),
      'UPDATED': updatedMatch[1].trim().split(' ').slice(0, 3).join('|')
    };

    console.log('Sending dict to watch...');
    Pebble.sendAppMessage(dict, function () {
      console.log('Dict sent successfully');
    }, function (e) {
      console.log('Error sending stats to watch: ' + JSON.stringify(e));
    });
  };

  xhr.onerror = function () { console.log('XHR Error'); };
  xhr.ontimeout = function () { console.log('XHR Timeout'); };

  xhr.open('GET', url);
  xhr.send();
}

Pebble.addEventListener('ready', function (e) {
  console.log('PKJS Ready event');
  fetchSolarData();
});

Pebble.addEventListener('appmessage', function (e) {
  console.log('AppMessage event');
  fetchSolarData();
});

Pebble.addEventListener('showConfiguration', function (e) {
  console.log('Showing configuration');
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function (e) {
  console.log('Webview closed');
  if (e && !e.response) {
    console.log('No response from webview');
    return;
  }
  var dict = clay.getSettings(e.response);
  console.log('Settings extracted: ' + JSON.stringify(dict));
  Pebble.sendAppMessage(dict, function () {
    console.log('Settings sent to watch');
    fetchSolarData();
  }, function (err) {
    console.log('Error sending settings to watch');
  });
});
