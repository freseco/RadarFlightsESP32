#include "web_server.h"
#include <Update.h>

// --- HTML DEL PORTAL CAUTIVO ---
const char* htmlForm = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset='utf-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>Config Radar ESP32</title>
  <style>
    body { background-color: #121212; color: #fff; font-family: sans-serif; padding: 20px; max-width: 500px; margin: 0 auto; }
    h2 { text-align: center; color: #4CAF50; margin-bottom: 5px; }
    p.sub { text-align: center; font-size: 14px; color: #888; margin-top: 0; margin-bottom: 25px; }
    label { display: block; margin-top: 15px; font-size: 14px; color: #ccc; }
    input, select { width: 100%; padding: 12px; margin-top: 5px; background: #222; color: white; border: 1px solid #444; border-radius: 6px; box-sizing: border-box; font-size: 16px; }
    input[type="checkbox"] { width: auto; display: inline-block; margin-right: 10px; transform: scale(1.5); }
    .chk-container { margin-top: 15px; margin-bottom: 5px; display: flex; align-items: center; }
    input:focus, select:focus { border-color: #4CAF50; outline: none; }
    button { width: 100%; padding: 15px; margin-top: 30px; background: #4CAF50; color: white; border: none; border-radius: 6px; font-size: 18px; cursor: pointer; font-weight: bold; }
    button:active { background: #45a049; }
    details { background: #1e1e1e; border-radius: 8px; margin-bottom: 15px; padding: 10px; border: 1px solid #333; }
    summary { font-weight: bold; cursor: pointer; outline: none; font-size: 16px; color: #4CAF50; padding: 5px 0; list-style: none; }
    summary::-webkit-details-marker { display: none; }
    summary::after { content: ' ▼'; float: right; color: #888; }
    details[open] summary::after { content: ' ▲'; }
    details[open] summary { border-bottom: 1px solid #333; margin-bottom: 10px; padding-bottom: 10px; }
  </style>
</head>
<body>
  <h2>⚙️ Ajustes del Radar</h2>
  <p class="sub">Configura tu dispositivo</p>
  <form action='/save' method='POST'>
  <div style='position: absolute; top: 20px; right: 20px; font-size: 28px; cursor: pointer;'>
    <span id='flag_es' onclick='document.getElementById("langHidden").value="es"; changeLang("es"); document.getElementById("flag_es").style.opacity="1"; document.getElementById("flag_en").style.opacity="0.3";' style='%OPACITY_ES% margin-right: 10px;'>🇪🇸</span>
    <span id='flag_en' onclick='document.getElementById("langHidden").value="en"; changeLang("en"); document.getElementById("flag_en").style.opacity="1"; document.getElementById("flag_es").style.opacity="0.3";' style='%OPACITY_EN%'>🇬🇧</span>
  </div>
  <input type='hidden' name='lang' id='langHidden' value='%LANG_VAL%'>
    <details>
      <summary>📶 Conexión WiFi</summary>
      <label>🌐 Red WiFi (Nombre):</label>
      <input list='wifi_networks' name='ssid' value='%SSID%'>
      <datalist id='wifi_networks'>
        %WIFI_OPTIONS%
      </datalist>
      <label>🔑 Contraseña WiFi:</label>
      <input type='password' name='pass' value='%PASS%'>
    </details>
    
    <details>
      <summary>🌍 Ubicación y Área</summary>
      <label>🌍 Autolocalizar por IP:</label>
      <select name='geoip'>
        <option value='1' %GEO_ON%>Sí (Ignora manuales)</option>
        <option value='0' %GEO_OFF%>No (Usar manuales)</option>
      </select>
      <label>🏢 Aeropuertos Famosos (Auto-relleno):</label>
      <select id='airportSelect' onchange='if(this.value){var p=this.value.split(",");document.getElementById("lat").value=p[0];document.getElementById("lon").value=p[1];document.getElementById("airport_id").value=p[2];}'>
        <option value=''>-- Selecciona un aeropuerto --</option>
        <option value='40.4722,-3.5609,MAD'>Madrid-Barajas (MAD) 🇪🇸</option>
        <option value='51.4700,-0.4543,LHR'>Londres Heathrow (LHR) 🇬🇧</option>
        <option value='40.6413,-73.7781,JFK'>New York (JFK) 🇺🇸</option>
        <option value='25.2532,55.3657,DXB'>Dubái (DXB) 🇦🇪</option>
        <option value='35.5494,139.7798,HND'>Tokio Haneda (HND) 🇯🇵</option>
        <option value='49.0097,2.5479,CDG'>París CDG (CDG) 🇫🇷</option>
        <option value='52.3105,4.7683,AMS'>Ámsterdam Schiphol (AMS) 🇳🇱</option>
        <option value='50.0379,8.5622,FRA'>Frankfurt (FRA) 🇩🇪</option>
        <option value='33.6407,-84.4277,ATL'>Atlanta Hartsfield (ATL) 🇺🇸</option>
        <option value='1.3644,103.9915,SIN'>Singapur Changi (SIN) 🇸🇬</option>
      </select>
      
      <input type='hidden' name='airport_id' id='airport_id' value='%AIRPORT_ID%'>
      <label>📍 Latitud (Manual):</label>
      <input type='number' step='any' name='lat' id='lat' value='%LAT%' oninput='document.getElementById("airport_id").value="";'>
      <label>📍 Longitud (Manual):</label>
      <input type='number' step='any' name='lon' id='lon' value='%LON%' oninput='document.getElementById("airport_id").value="";'>
      <button type='button' id='btnGeo' style='background: #2196F3; margin-top: 15px; margin-bottom: 15px; padding: 12px; font-size: 16px; width: 100%; border: none; border-radius: 4px; color: white;'>🧭 Obtener por Red (IP)</button>
      
      <label>📡 Radio del Radar (km):</label>
      <input type='number' step='1' name='rad' value='%RAD%'>
    </details>

    <details>
      <summary>🌤️ El Tiempo (AEMET)</summary>
      <label>🔑 API Key de AEMET:</label>
      <input type='text' name='aemet_key' value='%AEMET_KEY%'>
      <label>🏢 Estaciones AEMET (Auto-relleno):</label>
      <select id='stationSelect' onchange='if(this.value){document.getElementById("aemet_idema").value=this.value;}'>
        <option value=''>-- Selecciona una estación representativa --</option>
        <option value='3195'>Madrid, Retiro (3195)</option>
        <option value='3129'>Madrid, Aeropuerto (3129)</option>
        <option value='0201D'>Barcelona, Aeropuerto (0201D)</option>
        <option value='0076'>Barcelona, Raval (0076)</option>
        <option value='8414A'>Valencia, Aeropuerto (8414A)</option>
        <option value='4642E'>Sevilla, Aeropuerto (4642E)</option>
        <option value='8500A'>Zaragoza, Aeropuerto (8500A)</option>
        <option value='6155A'>Málaga, Aeropuerto (6155A)</option>
        <option value='7228'>Murcia (7228)</option>
        <option value='9771C'>Palma de Mallorca, Aerop. (9771C)</option>
        <option value='C139E'>Gran Canaria, Aeropuerto (C139E)</option>
        <option value='8025'>Alicante/Alacant (8025)</option>
        <option value='5402'>Córdoba, Aeropuerto (5402)</option>
        <option value='2422'>Valladolid (2422)</option>
        <option value='1387'>A Coruña (1387)</option>
        <option value='1014'>San Sebastián, Igueldo (1014)</option>
        <option value='1208H'>Gijón, Musel (1208H)</option>
        <option value='1428'>Santiago de Compostela, Aerop. (1428)</option>
        <option value='C447A'>Tenerife Norte, Aeropuerto (C447A)</option>
        <option value='3469A'>Cáceres (3469A)</option>
        <option value='4452'>Badajoz (4452)</option>
        <option value='4358X'>Don Benito (4358X)</option>
      </select>
      <label>📍 ID de Estación (Manual o auto por GPS):</label>
      <input type='text' name='aemet_idema' id='aemet_idema' value='%AEMET_IDEMA%'>
    </details>

    <details>
      <summary>🕒 Hora y Fecha</summary>
      <label>🕒 Zona Horaria (Horas desde UTC):</label>
      <input type='number' step='1' name='utc_offset' value='%UTC_OFFSET%'>
      <label>☀️ Horario de Verano (+1h):</label>
      <select name='dst'>
        <option value='1' %DST_ON%>Activado</option>
        <option value='0' %DST_OFF%>Desactivado</option>
      </select>
      <label>⌚ Modo de Reloj:</label>
      <select name='clock_mode'>
        <option value='0' %CM_0%>Ciclar todos</option>
        <option value='1' %CM_1%>Solo Digital</option>
        <option value='2' %CM_2%>Solo Analógico 12h</option>
        <option value='3' %CM_3%>Solo Analógico 24h</option>
      </select>
    </details>

    <details>
      <summary>⚙️ Ajustes Visuales</summary>
      <label name='lbl_screens'>📺 Pantallas Activas:</label>
      <div style='text-align: left; margin-left: 20px; color: #ccc; font-size: 16px; margin-bottom: 20px;'>
        <div class="chk-container"><input type='checkbox' name='sh_radar' value='1' %CHK_RADAR%> Radar</div>
        <div class="chk-container"><input type='checkbox' name='sh_time' value='1' %CHK_TIME%> Reloj</div>
        <div class="chk-container"><input type='checkbox' name='sh_wea' value='1' %CHK_WEA%> Tiempo (AEMET)</div>
        <div class="chk-container"><input type='checkbox' name='sh_moon' value='1' %CHK_MOON%> Fase Lunar</div>
        <div class="chk-container"><input type='checkbox' name='sh_horiz' value='1' %CHK_HORIZ%> Horizonte Artificial</div>
        <div class="chk-container"><input type='checkbox' name='sh_target' value='1' %CHK_TARGET%> Target Lock</div>
        <div class="chk-container"><input type='checkbox' name='sh_iss' value='1' %CHK_ISS%> ISS Tracker</div>
        <div class="chk-container"><input type='checkbox' name='sh_sun' value='1' %CHK_SUN%> Arco Solar</div>
      </div>
      <label>⏳ Tiempo de cada pantalla (segundos):</label>
      <input type='number' name='screen_time' value='%SCREEN_TIME%'>
      <label>✈️ Máx. Aviones Visibles:</label>
      <input type='number' name='maxp' value='%MAXP%'>
      <label>🎨 Color de los Aviones:</label>
      <select name='color'>
        <option value='red' %C_RED%>Rojo</option>
        <option value='blue' %C_BLU%>Azul</option>
        <option value='orange' %C_ORA%>Naranja</option>
      </select>
      <label>📏 Sistema de Medida (Altitud):</label>
      <select name='units'>
        <option value='m' %U_M%>Métrico (m)</option>
        <option value='ft' %U_FT%>Imperial (ft)</option>
      </select>
      <label>👻 Avión Fantasma (minutos, 0=Apagado):</label>
      <input type='number' name='ghost' value='%GHOST_MINS%'>
      <label>💨 Velocidad del Avión (px/s):</label>
      <input type='number' name='ghost_speed' value='%GHOST_SPEED%'>
      <label>🌠 Longitud de la estela (puntos):</label>
      <input type='number' name='ghost_trail' value='%GHOST_TRAIL%'>
    </details>

    <details>
      <summary>📊 Estado y Estadísticas</summary>
      <p style='color: #ccc; font-size: 14px;'><b>Temp CPU:</b> <span id='stat_cpu'>%CPU_TEMP%</span> °C</p>
      <p style='color: #ccc; font-size: 14px;'><b>Temp AEMET:</b> <span id='stat_aemet'>%AEMET_TEMP%</span> °C</p>
      <p style='color: #ccc; font-size: 14px;'><b>Aviones Mostrados:</b> <span id='stat_planes'>%PLANES_COUNT%</span></p>
      <p style='color: #ccc; font-size: 14px; margin-bottom: 5px;'><b>Registro de Errores:</b></p>
      <div id='stat_errors' style='background: #333; padding: 10px; border-radius: 5px; font-family: monospace; font-size: 12px; white-space: pre-wrap; color: #ffeb3b;'>%ERROR_LOG%</div>
    </details>

    <button type='submit'>💾 Guardar y Reiniciar</button>
  </form>

  <div style="text-align: center; margin-top: 30px;">
    <a href="/update_page" style="display: block; background: #ff9800; color: white; padding: 12px; text-decoration: none; border-radius: 6px; font-weight: bold; margin-bottom: 15px;">🔄 Actualizar Firmware (OTA)</a>
    <a href="https://globe.airplanes.live/" target="_blank" style="color: #4CAF50; text-decoration: none; font-size: 16px;">🌍 Ver Mapa Global en Airplanes.live</a>
  </div>

  <script>
    const i18nDict = {
      "es": {
        title: "⚙️ Ajustes del Radar", sub: "Configura tu dispositivo",
        s_wifi: "📶 Conexión WiFi", l_ssid: "🌐 Red WiFi (Nombre):", l_pass: "🔑 Contraseña WiFi:",
        s_loc: "🌍 Ubicación y Área", l_geoip: "🌍 Autolocalizar por IP:", o_geo1: "Sí (Ignora manuales)", o_geo0: "No (Usar manuales)",
        l_airports: "🏢 Aeropuertos Famosos (Auto-relleno):", o_asel: "-- Selecciona un aeropuerto --",
        l_lat: "📍 Latitud (Manual):", l_lon: "📍 Longitud (Manual):", btn_geo: "🧭 Obtener por Red (IP)", l_rad: "📡 Radio del Radar (km):",
        s_wea: "🌤️ El Tiempo (AEMET)", l_api: "🔑 API Key de AEMET:", l_sta: "🏢 Estaciones AEMET (Auto-relleno):",
        o_ssel: "-- Selecciona una estación representativa --", l_idema: "📍 ID de Estación (Manual o auto por GPS):",
        s_time: "🕒 Hora y Fecha", l_utc: "🕒 Zona Horaria (Horas desde UTC):", l_dst: "☀️ Horario de Verano (+1h):", o_d1: "Activado", o_d0: "Desactivado",
        l_clock: "⌚ Modo de Reloj:", o_c0: "Ciclar todos", o_c1: "Solo Digital", o_c2: "Solo Analógico 12h", o_c3: "Solo Analógico 24h",
        s_vis: "⚙️ Ajustes Visuales", l_maxp: "✈️ Máx. Aviones Visibles:", l_col: "🎨 Color de los Aviones:",
        o_r: "Rojo", o_b: "Azul", o_o: "Naranja", l_uni: "📏 Sistema de Medida (Altitud):", o_um: "Métrico (m)", o_uft: "Imperial (ft)",
        l_gho: "👻 Avión Fantasma (minutos, 0=Apagado):", l_spd: "💨 Velocidad del Avión (px/s):", l_trl: "🌠 Longitud de la estela (puntos):",
        s_stat: "📊 Estado y Estadísticas", btn_save: "💾 Guardar y Reiniciar", a_ota: "🔄 Actualizar Firmware (OTA)", a_map: "🌍 Ver Mapa Global en Airplanes.live"
      },
      "en": {
        title: "⚙️ Radar Settings", sub: "Configure your device",
        s_wifi: "📶 WiFi Connection", l_ssid: "🌐 WiFi Network (Name):", l_pass: "🔑 WiFi Password:",
        s_loc: "🌍 Location and Area", l_geoip: "🌍 Auto-locate by IP:", o_geo1: "Yes (Ignore manual)", o_geo0: "No (Use manual)",
        l_airports: "🏢 Famous Airports (Auto-fill):", o_asel: "-- Select an airport --",
        l_lat: "📍 Latitude (Manual):", l_lon: "📍 Longitude (Manual):", btn_geo: "🧭 Get by Network (IP)", l_rad: "📡 Radar Radius (km):",
        s_wea: "🌤️ Weather (AEMET)", l_api: "🔑 AEMET API Key:", l_sta: "🏢 AEMET Stations (Auto-fill):",
        o_ssel: "-- Select a representative station --", l_idema: "📍 Station ID (Manual or auto by GPS):",
        s_time: "🕒 Time and Date", l_utc: "🕒 Timezone (Hours from UTC):", l_dst: "☀️ Daylight Saving Time (+1h):", o_d1: "Enabled", o_d0: "Disabled",
        l_clock: "⌚ Clock Mode:", o_c0: "Cycle all", o_c1: "Digital Only", o_c2: "Analog 12h Only", o_c3: "Analog 24h Only",
        s_vis: "⚙️ Visual Settings", l_maxp: "✈️ Max Visible Planes:", l_col: "🎨 Planes Color:",
        o_r: "Red", o_b: "Blue", o_o: "Orange", l_uni: "📏 Measurement System (Altitude):", o_um: "Metric (m)", o_uft: "Imperial (ft)",
        l_gho: "👻 Ghost Plane (minutes, 0=Off):", l_spd: "💨 Plane Speed (px/s):", l_trl: "🌠 Trail Length (points):",
        s_stat: "📊 Status and Statistics", btn_save: "💾 Save and Reboot", a_ota: "🔄 Update Firmware (OTA)", a_map: "🌍 View Global Map on Airplanes.live"
      }
    };
    function changeLang(l) {
      if(!i18nDict[l]) return;
      const d = i18nDict[l];
      document.querySelector("h2").innerText = d.title;
      document.querySelector("p.sub").innerText = d.sub;
      
      const sums = document.querySelectorAll("summary");
      sums[0].innerText = d.s_wifi; sums[1].innerText = d.s_loc; sums[2].innerText = d.s_wea; 
      sums[3].innerText = d.s_time; sums[4].innerText = d.s_vis; sums[5].innerText = d.s_stat;
      
      const txt = (selector, text) => { const el = document.querySelector(selector); if(el) el.innerText = text; };
      const setLbl = (name, text) => txt(`input[name='${name}']`, text); // Not good since label is before input
      
      // Let's select labels by traversing previous element sibling of inputs
      const setLabelByInputName = (name, text) => {
         const inp = document.querySelector(`[name='${name}']`);
         if(inp && inp.previousElementSibling && inp.previousElementSibling.tagName === 'LABEL') {
           inp.previousElementSibling.innerText = text;
         }
      };
      
      setLabelByInputName('ssid', d.l_ssid);
      setLabelByInputName('pass', d.l_pass);
      setLabelByInputName('geoip', d.l_geoip);
      setLabelByInputName('lat', d.l_lat);
      setLabelByInputName('lon', d.l_lon);
      setLabelByInputName('rad', d.l_rad);
      setLabelByInputName('aemet_key', d.l_api);
      setLabelByInputName('aemet_idema', d.l_idema);
      setLabelByInputName('utc_offset', d.l_utc);
      setLabelByInputName('dst', d.l_dst);
      setLabelByInputName('clock_mode', d.l_clock);
      setLabelByInputName('maxp', d.l_maxp);
      setLabelByInputName('color', d.l_col);
      setLabelByInputName('units', d.l_uni);
      setLabelByInputName('ghost', d.l_gho);
      setLabelByInputName('ghost_speed', d.l_spd);
      setLabelByInputName('ghost_trail', d.l_trl);
      
      // Selects that don't follow the pattern
      const airportSel = document.getElementById('airportSelect');
      if (airportSel && airportSel.previousElementSibling) airportSel.previousElementSibling.innerText = d.l_airports;
      const stationSel = document.getElementById('stationSelect');
      if (stationSel && stationSel.previousElementSibling) stationSel.previousElementSibling.innerText = d.l_sta;

      txt("select[name='geoip'] option[value='1']", d.o_geo1);
      txt("select[name='geoip'] option[value='0']", d.o_geo0);
      txt("#airportSelect option[value='']", d.o_asel);
      txt("#stationSelect option[value='']", d.o_ssel);
      txt("select[name='dst'] option[value='1']", d.o_d1);
      txt("select[name='dst'] option[value='0']", d.o_d0);
      txt("select[name='clock_mode'] option[value='0']", d.o_c0);
      txt("select[name='clock_mode'] option[value='1']", d.o_c1);
      txt("select[name='clock_mode'] option[value='2']", d.o_c2);
      txt("select[name='clock_mode'] option[value='3']", d.o_c3);
      txt("select[name='color'] option[value='red']", d.o_r);
      txt("select[name='color'] option[value='blue']", d.o_b);
      txt("select[name='color'] option[value='orange']", d.o_o);
      txt("select[name='units'] option[value='m']", d.o_um);
      txt("select[name='units'] option[value='ft']", d.o_uft);
      
      txt("#btnGeo", d.btn_geo);
      txt("button[type='submit']", d.btn_save);
      
      const links = document.querySelectorAll("div[style*='text-align: center'] a");
      if(links.length > 1) {
        links[0].innerText = d.a_ota; links[1].innerText = d.a_map;
      }
    }
    window.addEventListener("DOMContentLoaded", () => {
      changeLang(document.getElementById("langSelect").value);
    });
    document.getElementById('btnGeo').addEventListener('click', function() {
      var btn = this;
      var originalText = btn.innerText;
      btn.innerText = '⏳ Obteniendo...';
      fetch('http://ip-api.com/json/')
        .then(r => r.json())
        .then(data => {
          if(data.status === "success" && data.lat && data.lon) {
            document.getElementById('lat').value = data.lat;
            document.getElementById('lon').value = data.lon;
            document.getElementById('airport_id').value = '';
            btn.innerText = '✅ ¡Coordenadas Obtenidas!';
            btn.style.background = '#4CAF50';
          } else {
            btn.innerText = '❌ Error en la API';
            btn.style.background = '#f44336';
          }
          setTimeout(() => { btn.innerText = originalText; btn.style.background = '#2196F3'; }, 3000);
        })
        .catch(e => {
          btn.innerText = '❌ Sin conexión a Internet';
          btn.style.background = '#f44336';
          setTimeout(() => { btn.innerText = originalText; btn.style.background = '#2196F3'; }, 3000);
        });
    });

    // Actualizar estado en vivo
    setInterval(() => {
      fetch('/api/status')
        .then(r => r.json())
        .then(data => {
          document.getElementById('stat_cpu').innerText = data.cpu;
          document.getElementById('stat_aemet').innerText = data.aemet;
          document.getElementById('stat_planes').innerText = data.planes;
          document.getElementById('stat_errors').innerText = data.errors;
        }).catch(e => console.log('Error updating status'));
    }, 5000);
  </script>
  <div style="text-align: center; margin-top: 30px; font-size: 12px; color: #555;">Autor: freseco@gmail.com</div>
</body>
</html>
)=====";

const char* otaHtml = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset='utf-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>Actualizar Firmware</title>
  <style>
    body { background-color: #121212; color: #fff; font-family: sans-serif; padding: 20px; max-width: 500px; margin: 0 auto; text-align: center; }
    h2 { color: #4CAF50; margin-bottom: 5px; }
    p.sub { font-size: 14px; color: #888; margin-bottom: 25px; }
    input[type='file'] { margin: 20px 0; font-size: 16px; background: #222; padding: 10px; border-radius: 6px; width: 100%; box-sizing: border-box; color: white; }
    button { width: 100%; padding: 15px; background: #4CAF50; color: white; border: none; border-radius: 6px; font-size: 18px; cursor: pointer; font-weight: bold; }
    button:active { background: #45a049; }
    button:disabled { background: #555; cursor: not-allowed; }
    .back { display: block; margin-top: 20px; color: #2196F3; text-decoration: none; }
    #progress-container { display: none; margin-top: 20px; background: #222; border-radius: 6px; overflow: hidden; border: 1px solid #444; }
    #progress-bar { width: 0%; height: 20px; background: #4CAF50; transition: width 0.2s; }
    #status { margin-top: 15px; font-weight: bold; }
  </style>
</head>
<body>
  <h2>🔄 Actualizar Firmware</h2>
  <p class="sub">Sube el archivo .bin para actualizar tu Radar.</p>
  <form method='POST' action='/update' enctype='multipart/form-data' id='upload_form'>
    <input type='file' name='update' id='file' accept='.bin' required>
    <button type='submit' id='btnUpload'>Subir y Actualizar</button>
  </form>
  <div id="progress-container">
    <div id="progress-bar"></div>
  </div>
  <p id="status"></p>
  <a href='/' class='back'>⬅️ Volver a Ajustes</a>
  
  <script>
    const i18nOta = {
      "es": { title: "🔄 Actualizar Firmware", sub: "Sube el archivo .bin para actualizar tu Radar.", btn: "Subir y Actualizar", back: "⬅️ Volver a Ajustes" },
      "en": { title: "🔄 Update Firmware", sub: "Upload the .bin file to update your Radar.", btn: "Upload and Update", back: "⬅️ Back to Settings" }
    };
    window.addEventListener("DOMContentLoaded", () => {
      const lang = "%LANG%";
      if(i18nOta[lang]) {
        document.querySelector("h2").innerText = i18nOta[lang].title;
        document.querySelector("p.sub").innerText = i18nOta[lang].sub;
        document.getElementById("btnUpload").innerText = i18nOta[lang].btn;
        document.querySelector(".back").innerText = i18nOta[lang].back;
      }
    });

    document.getElementById('upload_form').addEventListener('submit', function(e) {
      e.preventDefault();
      var btn = document.getElementById('btnUpload');
      var fileInput = document.getElementById('file');
      if(fileInput.files.length === 0) return;
      
      btn.innerText = 'Subiendo...';
      btn.disabled = true;
      document.getElementById('progress-container').style.display = 'block';
      var status = document.getElementById('status');
      status.innerText = 'Subiendo archivo...';
      
      var file = fileInput.files[0];
      var formData = new FormData();
      formData.append('update', file);
      
      var xhr = new XMLHttpRequest();
      xhr.open('POST', '/update', true);
      
      xhr.upload.onprogress = function(e) {
        if (e.lengthComputable) {
          var percentComplete = (e.loaded / e.total) * 100;
          document.getElementById('progress-bar').style.width = percentComplete + '%';
          if(percentComplete == 100) {
            status.innerText = 'Instalando firmware, por favor espera...';
          }
        }
      };
      
      xhr.onload = function() {
        if (xhr.status == 200 && xhr.responseText.trim() === 'OK') {
          status.innerHTML = '<span style="color:#4CAF50;">✅ ¡Actualización completada! Reiniciando...</span>';
          setTimeout(function() { window.location.href = '/'; }, 8000);
        } else {
          status.innerHTML = '<span style="color:#f44336;">❌ Error en la actualización.</span>';
          btn.innerText = 'Intentar de nuevo';
          btn.disabled = false;
        }
      };
      
      xhr.onerror = function() {
        status.innerHTML = '<span style="color:#f44336;">❌ Error de red.</span>';
        btn.innerText = 'Intentar de nuevo';
        btn.disabled = false;
      };
      
      xhr.send(formData);
    });
  </script>
  <div style="text-align: center; margin-top: 30px; font-size: 12px; color: #555;">Autor: freseco@gmail.com</div>
</body>
</html>
)=====";

void handleRoot() {
  String html = htmlForm;
  
  int n = WiFi.scanNetworks();
  String wifiOptions = "";
  for (int i = 0; i < n; ++i) {
    wifiOptions += "<option value='" + WiFi.SSID(i) + "'>";
  }
  html.replace("%WIFI_OPTIONS%", wifiOptions);
  
  html.replace("%OPACITY_ES%", pref_lang == "es" ? "opacity: 1.0;" : "opacity: 0.3;");
  html.replace("%OPACITY_EN%", pref_lang == "en" ? "opacity: 1.0;" : "opacity: 0.3;");
  html.replace("%LANG_VAL%", pref_lang);
  html.replace("%SSID%", pref_ssid);
  html.replace("%PASS%", pref_pass);
  html.replace("%LAT%", String(pref_lat, 4));
  html.replace("%LON%", String(pref_lon, 4));
  html.replace("%AIRPORT_ID%", pref_airport_id);
  html.replace("%RAD%", String((int)pref_rad));
  html.replace("%MAXP%", String(pref_max_planes));
  html.replace("%AEMET_KEY%", pref_aemet_key);
  html.replace("%AEMET_IDEMA%", pref_idema);
  html.replace("%UTC_OFFSET%", String(pref_offset / 3600));
  html.replace("%DST_ON%", pref_dst ? "selected" : "");
  html.replace("%DST_OFF%", !pref_dst ? "selected" : "");
  
  html.replace("%CM_0%", pref_clock_mode == 0 ? "selected" : "");
  html.replace("%CM_1%", pref_clock_mode == 1 ? "selected" : "");
  html.replace("%CM_2%", pref_clock_mode == 2 ? "selected" : "");
  html.replace("%CM_3%", pref_clock_mode == 3 ? "selected" : "");

  
  html.replace("%GEO_ON%", pref_geoip ? "selected" : "");
  html.replace("%GEO_OFF%", !pref_geoip ? "selected" : "");
  
  html.replace("%C_RED%", pref_color == "red" ? "selected" : "");
  html.replace("%C_BLU%", pref_color == "blue" ? "selected" : "");
  html.replace("%C_ORA%", pref_color == "orange" ? "selected" : "");
  html.replace("%U_M%", pref_units == "m" ? "selected" : "");
  html.replace("%U_FT%", pref_units == "ft" ? "selected" : "");
  
  html.replace("%GHOST_MINS%", String(pref_ghost_mins));
  html.replace("%GHOST_SPEED%", String(pref_ghost_speed));
  html.replace("%GHOST_TRAIL%", String(pref_ghost_trail));
  
  html.replace("%CHK_RADAR%", pref_show_radar ? "checked" : "");
  html.replace("%CHK_TIME%", pref_show_time ? "checked" : "");
  html.replace("%CHK_WEA%", pref_show_weather ? "checked" : "");
  html.replace("%CHK_MOON%", pref_show_moon ? "checked" : "");
  html.replace("%CHK_HORIZ%", pref_show_horizon ? "checked" : "");
  html.replace("%CHK_TARGET%", pref_show_target ? "checked" : "");
  html.replace("%CHK_ISS%", pref_show_iss ? "checked" : "");
  html.replace("%CHK_SUN%", pref_show_sun ? "checked" : "");
  
  html.replace("%SCREEN_TIME%", String(pref_screen_time_s));
  
  html.replace("%CPU_TEMP%", String((int)temperatureRead()));
  html.replace("%AEMET_TEMP%", currentWeather.valid ? String(currentWeather.ta, 1) : "N/D");
  
  int count = 0;
  if (dataMutex != NULL) {
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    count = planes.size();
    xSemaphoreGive(dataMutex);
  }
  html.replace("%PLANES_COUNT%", String(count));
  
  String errLogHtml = "";
  if (errorLog.empty()) {
    errLogHtml = "Sin errores.";
  } else {
    for (String e : errorLog) {
      errLogHtml += e + "\n";
    }
  }
  html.replace("%ERROR_LOG%", errLogHtml);

  server.send(200, "text/html; charset=utf-8", html);
}

void handleSave() {
  String new_ssid = server.arg("ssid");
  String new_pass = server.arg("pass");
  float new_rad = server.arg("rad").toFloat();
  
  bool wifiChanged = (new_ssid != pref_ssid || new_pass != pref_pass);
  
  if (new_rad < pref_rad) zoomAnimState = 1;
  else if (new_rad > pref_rad) zoomAnimState = 2;

  preferences.putString("ssid", new_ssid);
  preferences.putString("pass", new_pass);
  preferences.putBool("geoip", server.arg("geoip") == "1");
  preferences.putFloat("lat", server.arg("lat").toFloat());
  preferences.putFloat("lon", server.arg("lon").toFloat());
  preferences.putFloat("rad", server.arg("rad").toFloat());
  preferences.putInt("maxp", server.arg("maxp").toInt());
  preferences.putString("color", server.arg("color"));
  if (server.hasArg("ghost")) {
    preferences.putInt("ghost", server.arg("ghost").toInt());
    pref_ghost_mins = server.arg("ghost").toInt();
  }
  if (server.hasArg("ghost_speed")) {
    preferences.putInt("ghost_speed", server.arg("ghost_speed").toInt());
    pref_ghost_speed = server.arg("ghost_speed").toInt();
  }
  if (server.hasArg("ghost_trail")) {
    preferences.putInt("ghost_trail", server.arg("ghost_trail").toInt());
    pref_ghost_trail = server.arg("ghost_trail").toInt();
  }
  if (server.hasArg("clock_mode")) {
    preferences.putInt("clock_mode", server.arg("clock_mode").toInt());
    pref_clock_mode = server.arg("clock_mode").toInt();
  }
  
  pref_show_radar = server.hasArg("sh_radar");
  pref_show_time = server.hasArg("sh_time");
  pref_show_weather = server.hasArg("sh_wea");
  pref_show_moon = server.hasArg("sh_moon");
  pref_show_horizon = server.hasArg("sh_horiz");
  pref_show_target = server.hasArg("sh_target");
  pref_show_iss = server.hasArg("sh_iss");
  pref_show_sun = server.hasArg("sh_sun");
  
  preferences.putBool("sh_radar", pref_show_radar);
  preferences.putBool("sh_time", pref_show_time);
  preferences.putBool("sh_wea", pref_show_weather);
  preferences.putBool("sh_moon", pref_show_moon);
  preferences.putBool("sh_horiz", pref_show_horizon);
  preferences.putBool("sh_target", pref_show_target);
  preferences.putBool("sh_iss", pref_show_iss);
  preferences.putBool("sh_sun", pref_show_sun);
  
  if (server.hasArg("screen_time")) {
    preferences.putInt("screen_time", server.arg("screen_time").toInt());
    pref_screen_time_s = server.arg("screen_time").toInt();
  }
  
  if (server.hasArg("lang")) {
    preferences.putString("lang", server.arg("lang"));
    pref_lang = server.arg("lang");
  }
  if (server.hasArg("units")) {
    preferences.putString("units", server.arg("units"));
    pref_units = server.arg("units");
  }

  preferences.putString("airport_id", server.arg("airport_id"));
  preferences.putString("aemet_key", server.arg("aemet_key"));
  preferences.putString("aemet_idema", server.arg("aemet_idema"));
  
  if (server.hasArg("utc_offset")) {
    preferences.putLong("offset", server.arg("utc_offset").toInt() * 3600);
    preferences.putBool("dst", server.arg("dst") == "1");
  }
  
  // Actualizar variables en memoria
  pref_ssid = new_ssid;
  pref_pass = new_pass;
  pref_geoip = server.arg("geoip") == "1";
  pref_lat = server.arg("lat").toFloat();
  pref_lon = server.arg("lon").toFloat();
  pref_rad = new_rad;
  pref_max_planes = server.arg("maxp").toInt();
  pref_color = server.arg("color");
  pref_airport_id = server.arg("airport_id");
  pref_aemet_key = server.arg("aemet_key");
  pref_idema = server.arg("aemet_idema");
  if (server.hasArg("utc_offset")) {
    pref_offset = server.arg("utc_offset").toInt() * 3600;
    pref_dst = server.arg("dst") == "1";
    configTime(pref_offset + (pref_dst ? 3600 : 0), 0, "pool.ntp.org", "time.nist.gov");
  }
  airportShownInitially = false;

  // Forzar redibujado y búsqueda limpia inmediata
  planes.clear();
  lastFetchTime = 0;
  
  if (wifiChanged) {
    String successHtml = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'><meta charset='utf-8'></head>"
                         "<body style='background-color:#121212; color:white; font-family:sans-serif; text-align:center; padding:50px 20px;'>"
                         "<h2 style='color:#4CAF50; margin-bottom:20px;'>✅ Guardado con éxito.</h2>"
                         "<p style='color:#888; margin-bottom:40px;'>Reiniciando WiFi...</p>"
                         "<a href='/' style='background-color:#4CAF50; color:white; padding:15px 20px; text-decoration:none; border-radius:6px; font-size:18px; font-weight:bold; display:inline-block; width:80%; box-sizing:border-box;'>Volver al Menú</a>"
                         "</body></html>";
    server.send(200, "text/html; charset=utf-8", successHtml);
    delay(1500);
    ESP.restart();
  } else {
    String successHtml = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'><meta charset='utf-8'></head>"
                         "<body style='background-color:#121212; color:white; font-family:sans-serif; text-align:center; padding:50px 20px;'>"
                         "<h2 style='color:#4CAF50; margin-bottom:20px;'>⚡ Ajustes Aplicados en Vivo</h2>"
                         "<p style='color:#888; margin-bottom:40px;'>No ha sido necesario reiniciar. Mira la pantalla de tu radar.</p>"
                         "<a href='/' style='background-color:#4CAF50; color:white; padding:15px 20px; text-decoration:none; border-radius:6px; font-size:18px; font-weight:bold; display:inline-block; width:80%; box-sizing:border-box;'>Volver al Menú</a>"
                         "</body></html>";
  server.send(200, "text/html; charset=utf-8", successHtml);
  }
}

void handleStatus() {
  String json = "{";
  json += "\"cpu\":\"" + String((int)temperatureRead()) + "\",";
  json += "\"aemet\":\"" + (currentWeather.valid ? String(currentWeather.ta, 1) : "N/D") + "\",";
  
  int count = 0;
  if (dataMutex != NULL) {
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    count = planes.size();
    xSemaphoreGive(dataMutex);
  }
  json += "\"planes\":\"" + String(count) + "\",";
  
  String errLogHtml = "";
  if (errorLog.empty()) {
    errLogHtml = "Sin errores.";
  } else {
    for (String e : errorLog) {
      errLogHtml += e + "\\n";
    }
  }
  errLogHtml.replace("\"", "\\\"");
  json += "\"errors\":\"" + errLogHtml + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.on("/api/status", handleStatus);
  
  server.on("/update_page", HTTP_GET, []() {
    String otaStr = otaHtml;
    otaStr.replace("%LANG%", pref_lang);
    server.send(200, "text/html; charset=utf-8", otaStr);
  });
  
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { // true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  // Redirigir cualquier otra petición a la raíz (Captive Portal)
  server.onNotFound([]() { 
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  server.begin();
}
