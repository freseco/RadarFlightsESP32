#include "web_server.h"
#include <Update.h>
#include "math_utils.h"

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
        <option value='-23.4356,-46.4731,GRU'>São Paulo Guarulhos (GRU) 🇧🇷</option>
        <option value='4.7016,-74.1469,BOG'>Bogotá El Dorado (BOG) 🇨🇴</option>
        <option value='-34.8222,-58.5358,EZE'>Buenos Aires Ezeiza (EZE) 🇦🇷</option>
        <option value='-33.3930,-70.7858,SCL'>Santiago (SCL) 🇨🇱</option>
        <option value='-12.0219,-77.1143,LIM'>Lima Jorge Chávez (LIM) 🇵🇪</option>
        <option value='18.5674,-68.3634,PUJ'>Punta Cana (PUJ) 🇩🇴</option>
        <option value='18.4394,-66.0018,SJU'>San Juan (SJU) 🇵🇷</option>
        <option value='22.9892,-82.4091,HAV'>La Habana (HAV) 🇨🇺</option>
        <option value='-26.1367,28.2411,JNB'>Johannesburgo (JNB) 🇿🇦</option>
        <option value='30.1219,31.4056,CAI'>El Cairo (CAI) 🇪🇬</option>
        <option value='-33.9715,18.6021,CPT'>Ciudad del Cabo (CPT) 🇿🇦</option>
        <option value='8.9778,38.7993,ADD'>Adís Abeba (ADD) 🇪🇹</option>
        <option value='-1.3192,36.9278,NBO'>Nairobi (NBO) 🇰🇪</option>
        <option value='33.3675,-7.5899,CMN'>Casablanca (CMN) 🇲🇦</option>
      </select>
      
      <input type='hidden' name='airport_id' id='airport_id' value='%AIRPORT_ID%'>
      <label>📍 Latitud (Manual):</label>
      <input type='number' step='any' name='lat' id='lat' value='%LAT%' oninput='document.getElementById("airport_id").value="";'>
      <label>📍 Longitud (Manual):</label>
      <input type='number' step='any' name='lon' id='lon' value='%LON%' oninput='document.getElementById("airport_id").value="";'>
      <button type='button' id='btnGeo' style='background: #2196F3; margin-top: 15px; margin-bottom: 15px; padding: 12px; font-size: 16px; width: 100%; border: none; border-radius: 4px; color: white;'>🧭 Obtener por Red (IP)</button>
      
      <label>📡 Radio del Radar (km):</label>
      <input type='number' step='1' name='rad' value='%RAD%'>

      <label style="margin-top: 25px; color: #4CAF50; border-top: 1px solid #333; padding-top: 15px;">🌐 OpenSky Network (Opcional - Más límite API):</label>
      <label>👤 Client ID (OpenSky):</label>
      <input type='text' name='os_user' value='%OS_USER%' placeholder='Tu Client ID'>
      <label>🔑 Client Secret (OpenSky):</label>
      <input type='password' name='os_pass' value='%OS_PASS%' placeholder='Tu Client Secret'>
      <small style='color:#aaa'>Si tienes errores '429', ve a tu perfil de OpenSky y crea un API Client. Pon aquí el Client ID y Secret. (Autenticación OAuth2 Bearer token)</small>
    </details>



    <details>
      <summary>🛸 ISS Tracker (n2yo.com)</summary>
      <label>🔑 API Key de n2yo.com (gratis en n2yo.com/login):</label>
      <input type='text' name='n2yo_key' value='%N2YO_KEY%' placeholder='Tu API key de n2yo.com'>
      <small style='color:#aaa'>Registrate gratis en <a href='https://www.n2yo.com/login/' target='_blank' style='color:#6af'>n2yo.com</a> para obtener la key y ver cuándo será visible la ISS.</small>
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

        <div class="chk-container"><input type='checkbox' name='sh_moon' value='1' %CHK_MOON%> Fase Lunar</div>
        <div class="chk-container"><input type='checkbox' name='sh_horiz' value='1' %CHK_HORIZ%> Horizonte Artificial</div>
        <div class="chk-container"><input type='checkbox' name='sh_iss' value='1' %CHK_ISS%> ISS Tracker</div>
        <div class="chk-container"><input type='checkbox' name='sh_sun' value='1' %CHK_SUN%> Arco Solar</div>
        <div class="chk-container"><input type='checkbox' name='sh_zodiac' value='1' %CHK_ZODIAC%> Zodíaco</div>
        <div class="chk-container"><input type='checkbox' name='sh_elec' value='1' %CHK_ELEC%> Precio Luz (€/kWh)</div>
        <div class="chk-container"><input type='checkbox' name='sh_eclock' value='1' %CHK_ECLOCK%> Reloj Luz (Esfera)</div>
        <div class="chk-container"><input type='checkbox' name='sh_aqi' value='1' %CHK_AQI%> 🌬️ Calidad del Aire (AQI)</div>
        <div class="chk-container"><input type='checkbox' name='sh_crypto' value='1' %CHK_CRYPTO%> 📈 Gráfica Criptomonedas (Coinbase)</div>
      </div>
      <label>💰 Criptomoneda (Coinbase EUR):</label>
      <select name='crypto_coin'>
        <option value='BTC' %CRYPTO_BTC%>Bitcoin (BTC)</option>
        <option value='ETH' %CRYPTO_ETH%>Ethereum (ETH)</option>
        <option value='USDT' %CRYPTO_USDT%>Tether (USDT)</option>
        <option value='SOL' %CRYPTO_SOL%>Solana (SOL)</option>
        <option value='BNB' %CRYPTO_BNB%>BNB (BNB)</option>
        <option value='XRP' %CRYPTO_XRP%>XRP (XRP)</option>
        <option value='USDC' %CRYPTO_USDC%>USDC (USDC)</option>
        <option value='ADA' %CRYPTO_ADA%>Cardano (ADA)</option>
        <option value='DOGE' %CRYPTO_DOGE%>Dogecoin (DOGE)</option>
        <option value='AVAX' %CRYPTO_AVAX%>Avalanche (AVAX)</option>
        <option value='TRX' %CRYPTO_TRX%>TRON (TRX)</option>
        <option value='LINK' %CRYPTO_LINK%>Chainlink (LINK)</option>
        <option value='DOT' %CRYPTO_DOT%>Polkadot (DOT)</option>
        <option value='MATIC' %CRYPTO_MATIC%>Polygon (MATIC)</option>
        <option value='SHIB' %CRYPTO_SHIB%>Shiba Inu (SHIB)</option>
        <option value='LTC' %CRYPTO_LTC%>Litecoin (LTC)</option>
        <option value='BCH' %CRYPTO_BCH%>Bitcoin Cash (BCH)</option>
        <option value='XLM' %CRYPTO_XLM%>Stellar (XLM)</option>
        <option value='UNI' %CRYPTO_UNI%>Uniswap (UNI)</option>
        <option value='ATOM' %CRYPTO_ATOM%>Cosmos (ATOM)</option>
        <option value='ETC' %CRYPTO_ETC%>Ethereum Classic (ETC)</option>
        <option value='ALGO' %CRYPTO_ALGO%>Algorand (ALGO)</option>
        <option value='FIL' %CRYPTO_FIL%>Filecoin (FIL)</option>
        <option value='NEAR' %CRYPTO_NEAR%>NEAR Protocol (NEAR)</option>
        <option value='APE' %CRYPTO_APE%>ApeCoin (APE)</option>
        <option value='QNT' %CRYPTO_QNT%>Quant (QNT)</option>
        <option value='HBAR' %CRYPTO_HBAR%>Hedera (HBAR)</option>
        <option value='ICP' %CRYPTO_ICP%>Internet Computer (ICP)</option>
        <option value='SAND' %CRYPTO_SAND%>The Sandbox (SAND)</option>
        <option value='EOS' %CRYPTO_EOS%>EOS (EOS)</option>
        <option value='MANA' %CRYPTO_MANA%>Decentraland (MANA)</option>
        <option value='THETA' %CRYPTO_THETA%>Theta Network (THETA)</option>
        <option value='AAVE' %CRYPTO_AAVE%>Aave (AAVE)</option>
        <option value='XTZ' %CRYPTO_XTZ%>Tezos (XTZ)</option>
        <option value='AXS' %CRYPTO_AXS%>Axie Infinity (AXS)</option>
        <option value='CHZ' %CRYPTO_CHZ%>Chiliz (CHZ)</option>
        <option value='ENJ' %CRYPTO_ENJ%>Enjin Coin (ENJ)</option>
        <option value='DASH' %CRYPTO_DASH%>Dash (DASH)</option>
        <option value='MKR' %CRYPTO_MKR%>Maker (MKR)</option>
        <option value='GRT' %CRYPTO_GRT%>The Graph (GRT)</option>
        <option value='ZEC' %CRYPTO_ZEC%>Zcash (ZEC)</option>
      </select>
      <label>⏳ Período Gráfica Cripto:</label>
      <select name='crypto_period'>
        <option value='1d' %CPERIOD_1D%>1 Día</option>
        <option value='1w' %CPERIOD_1W%>1 Semana</option>
        <option value='1m' %CPERIOD_1M%>1 Mes</option>
        <option value='1y' %CPERIOD_1Y%>1 Año</option>
      </select>
      <label>⏳ Tiempo de cada pantalla (segundos):</label>
      <input type='number' name='screen_time' value='%SCREEN_TIME%'>
      <label>⏳ Tiempo en radar (segundos):</label>
      <input type='number' name='radar_time' value='%RADAR_TIME%'>
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
      <p style='color: #ccc; font-size: 14px;'><b>Temp Ext:</b> <span id='stat_aemet'>%AEMET_TEMP%</span> °C</p>
      <p style='color: #ccc; font-size: 14px;'><b>Aviones Mostrados:</b> <span id='stat_planes'>%PLANES_COUNT%</span></p>
      <p style='color: #ccc; font-size: 14px; margin-bottom: 5px;'><b>Registro de Errores:</b></p>
      <div id='stat_errors' style='background: #333; padding: 10px; border-radius: 5px; font-family: monospace; font-size: 12px; white-space: pre-wrap; color: #ffeb3b;'>%ERROR_LOG%</div>
    </details>

    <button type='submit'>💾 Guardar y Reiniciar</button>
  </form>

  <div style="text-align: center; margin-top: 30px;">
    <a href="/update_page" style="display: block; background: #ff9800; color: white; padding: 12px; text-decoration: none; border-radius: 6px; font-weight: bold; margin-bottom: 15px;">🔄 Actualizar Firmware (OTA)</a>
    <a href="https://opensky-network.org/network/explorer" target="_blank" style="color: #4CAF50; text-decoration: none; font-size: 16px;">🌍 Ver Mapa Global en OpenSky Network</a>
  </div>

  <script>
    const i18nDict = {
      "es": {
        title: "⚙️ Ajustes del Radar", sub: "Configura tu dispositivo",
        s_wifi: "📶 Conexión WiFi", l_ssid: "🌐 Red WiFi (Nombre):", l_pass: "🔑 Contraseña WiFi:",
        s_loc: "🌍 Ubicación y Área", l_geoip: "🌍 Autolocalizar por IP:", o_geo1: "Sí (Ignora manuales)", o_geo0: "No (Usar manuales)",
        l_airports: "🏢 Aeropuertos Famosos (Auto-relleno):", o_asel: "-- Selecciona un aeropuerto --",
        l_lat: "📍 Latitud (Manual):", l_lon: "📍 Longitud (Manual):", btn_geo: "🧭 Obtener por Red (IP)", l_rad: "📡 Radio del Radar (km):",
        s_wea: "🌤️ El Tiempo (Open-Meteo)",
        s_time: "🕒 Hora y Fecha", l_utc: "🕒 Zona Horaria (Horas desde UTC):", l_dst: "☀️ Horario de Verano (+1h):", o_d1: "Activado", o_d0: "Desactivado",
        l_clock: "⌚ Modo de Reloj:", o_c0: "Ciclar todos", o_c1: "Solo Digital", o_c2: "Solo Analógico 12h", o_c3: "Solo Analógico 24h",
        s_vis: "⚙️ Ajustes Visuales", l_scr_time: "⏳ Tiempo de cada pantalla (segundos):", l_rad_time: "⏳ Tiempo en radar (segundos):", l_maxp: "✈️ Máx. Aviones Visibles:", l_col: "🎨 Color de los Aviones:",
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
        s_wea: "🌤️ Weather (Open-Meteo)",
        s_time: "🕒 Time and Date", l_utc: "🕒 Timezone (Hours from UTC):", l_dst: "☀️ Daylight Saving Time (+1h):", o_d1: "Enabled", o_d0: "Disabled",
        l_clock: "⌚ Clock Mode:", o_c0: "Cycle all", o_c1: "Digital Only", o_c2: "Analog 12h Only", o_c3: "Analog 24h Only",
        s_vis: "⚙️ Visual Settings", l_scr_time: "⏳ Screen Time (seconds):", l_rad_time: "⏳ Radar Time (seconds):", l_maxp: "✈️ Max Visible Planes:", l_col: "🎨 Planes Color:",
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
      sums[0].innerText = d.s_wifi; sums[1].innerText = d.s_loc; 
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
      setLabelByInputName('os_user', "👤 Client ID (OpenSky):");
      setLabelByInputName('os_pass', "🔑 Client Secret (OpenSky):");

      setLabelByInputName('utc_offset', d.l_utc);
      setLabelByInputName('dst', d.l_dst);
      setLabelByInputName('clock_mode', d.l_clock);
      setLabelByInputName('screen_time', d.l_scr_time);
      setLabelByInputName('radar_time', d.l_rad_time);
      setLabelByInputName('maxp', d.l_maxp);
      setLabelByInputName('color', d.l_col);
      setLabelByInputName('units', d.l_uni);
      setLabelByInputName('ghost', d.l_gho);
      setLabelByInputName('ghost_speed', d.l_spd);
      setLabelByInputName('ghost_trail', d.l_trl);
      setLabelByInputName('crypto_coin', "💰 Criptomoneda:");
      setLabelByInputName('crypto_period', "⏳ Período Gráfica Cripto:");
      
      // Selects that don't follow the pattern
      const airportSel = document.getElementById('airportSelect');
      if (airportSel && airportSel.previousElementSibling) airportSel.previousElementSibling.innerText = d.l_airports;


      txt("select[name='geoip'] option[value='1']", d.o_geo1);
      txt("select[name='geoip'] option[value='0']", d.o_geo0);
      txt("#airportSelect option[value='']", d.o_asel);

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
  <div style="text-align: center; margin-top: 30px; font-size: 12px; color: #555;">Autor: freseco@gmail.com | v%FW_VER%<br><a href="https://github.com/freseco/RadarFlightsESP32" target="_blank" style="color: #888; text-decoration: none; display: inline-block; margin-top: 5px;" title="View on GitHub"><svg height="24" viewBox="0 0 16 16" version="1.1" width="24" aria-hidden="true" fill="currentColor"><path fill-rule="evenodd" d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0016 8c0-4.42-3.58-8-8-8z"></path></svg></a></div>
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
  <div style="text-align: center; margin-top: 30px; font-size: 12px; color: #555;">Autor: freseco@gmail.com<br><a href="https://github.com/freseco/RadarFlightsESP32" target="_blank" style="color: #888; text-decoration: none; display: inline-block; margin-top: 5px;" title="View on GitHub"><svg height="24" viewBox="0 0 16 16" version="1.1" width="24" aria-hidden="true" fill="currentColor"><path fill-rule="evenodd" d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0016 8c0-4.42-3.58-8-8-8z"></path></svg></a></div>
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
  html.replace("%OS_USER%", pref_os_user);
  html.replace("%OS_PASS%", pref_os_pass);

  html.replace("%N2YO_KEY%", pref_n2yo_key);
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
  html.replace("%CHK_ISS%", pref_show_iss ? "checked" : "");
  html.replace("%CHK_SUN%", pref_show_sun ? "checked" : "");
  html.replace("%CHK_ZODIAC%", pref_show_zodiac ? "checked" : "");
  html.replace("%CHK_ELEC%", pref_show_electricity ? "checked" : "");
  html.replace("%CHK_ECLOCK%", pref_show_elec_clock ? "checked" : "");
  html.replace("%CHK_AQI%", pref_show_aqi ? "checked" : "");
  html.replace("%CHK_CRYPTO%", pref_show_crypto ? "checked" : "");
  
  // Crypto setup
  html.replace("%CRYPTO_" + pref_crypto_coin + "%", "selected");
  html.replace("%CRYPTO_BTC%", ""); html.replace("%CRYPTO_ETH%", ""); html.replace("%CRYPTO_USDT%", "");
  html.replace("%CRYPTO_SOL%", ""); html.replace("%CRYPTO_BNB%", ""); html.replace("%CRYPTO_XRP%", "");
  html.replace("%CRYPTO_USDC%", ""); html.replace("%CRYPTO_ADA%", ""); html.replace("%CRYPTO_DOGE%", "");
  html.replace("%CRYPTO_AVAX%", ""); html.replace("%CRYPTO_TRX%", ""); html.replace("%CRYPTO_LINK%", "");
  html.replace("%CRYPTO_DOT%", ""); html.replace("%CRYPTO_MATIC%", ""); html.replace("%CRYPTO_SHIB%", "");
  html.replace("%CRYPTO_LTC%", ""); html.replace("%CRYPTO_BCH%", ""); html.replace("%CRYPTO_XLM%", "");
  html.replace("%CRYPTO_UNI%", ""); html.replace("%CRYPTO_ATOM%", ""); html.replace("%CRYPTO_ETC%", "");
  html.replace("%CRYPTO_ALGO%", ""); html.replace("%CRYPTO_FIL%", ""); html.replace("%CRYPTO_NEAR%", "");
  html.replace("%CRYPTO_APE%", ""); html.replace("%CRYPTO_QNT%", ""); html.replace("%CRYPTO_HBAR%", "");
  html.replace("%CRYPTO_ICP%", ""); html.replace("%CRYPTO_SAND%", ""); html.replace("%CRYPTO_EOS%", "");
  html.replace("%CRYPTO_MANA%", ""); html.replace("%CRYPTO_THETA%", ""); html.replace("%CRYPTO_AAVE%", "");
  html.replace("%CRYPTO_XTZ%", ""); html.replace("%CRYPTO_AXS%", ""); html.replace("%CRYPTO_CHZ%", "");
  html.replace("%CRYPTO_ENJ%", ""); html.replace("%CRYPTO_DASH%", ""); html.replace("%CRYPTO_MKR%", "");
  html.replace("%CRYPTO_ZEC%", ""); html.replace("%CRYPTO_GRT%", "");
  
  html.replace("%CPERIOD_1D%", pref_crypto_period == "1d" ? "selected" : "");
  html.replace("%CPERIOD_1W%", pref_crypto_period == "1w" ? "selected" : "");
  html.replace("%CPERIOD_1M%", pref_crypto_period == "1m" ? "selected" : "");
  html.replace("%CPERIOD_1Y%", pref_crypto_period == "1y" ? "selected" : "");
  
  html.replace("%FW_VER%", FIRMWARE_VERSION);
  
  html.replace("%SCREEN_TIME%", String(pref_screen_time_s));
  html.replace("%RADAR_TIME%", String(pref_radar_time_s));
  
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
  pref_show_iss = server.hasArg("sh_iss");
  pref_show_sun = server.hasArg("sh_sun");
  pref_show_zodiac = server.hasArg("sh_zodiac");
  pref_show_electricity = server.hasArg("sh_elec");
  pref_show_elec_clock = server.hasArg("sh_eclock");
  pref_show_aqi = server.hasArg("sh_aqi");
  pref_show_crypto = server.hasArg("sh_crypto");

  preferences.putBool("sh_radar", pref_show_radar);
  preferences.putBool("sh_time", pref_show_time);
  preferences.putBool("sh_wea", pref_show_weather);
  preferences.putBool("sh_moon", pref_show_moon);
  preferences.putBool("sh_horiz", pref_show_horizon);
  preferences.putBool("sh_iss", pref_show_iss);
  preferences.putBool("sh_sun", pref_show_sun);
  preferences.putBool("sh_zodiac", pref_show_zodiac);
  preferences.putBool("sh_elec", pref_show_electricity);
  
  preferences.putBool("sh_eclock", pref_show_elec_clock);
  preferences.putBool("sh_aqi", pref_show_aqi);
  preferences.putBool("sh_crypto", pref_show_crypto);
  
  if (server.hasArg("crypto_coin")) {
    preferences.putString("crypto_coin", server.arg("crypto_coin"));
    pref_crypto_coin = server.arg("crypto_coin");
  }
  if (server.hasArg("crypto_period")) {
    preferences.putString("crypto_period", server.arg("crypto_period"));
    pref_crypto_period = server.arg("crypto_period");
  }
  lastCryptoFetch = 0; // Force update
  
  if (server.hasArg("screen_time")) {
    preferences.putInt("screen_time", server.arg("screen_time").toInt());
    pref_screen_time_s = server.arg("screen_time").toInt();
  }
  if (server.hasArg("radar_time")) {
    preferences.putInt("radar_time", server.arg("radar_time").toInt());
    pref_radar_time_s = server.arg("radar_time").toInt();
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

  if (server.hasArg("os_user")) {
    preferences.putString("os_user", server.arg("os_user"));
    pref_os_user = server.arg("os_user");
  }
  if (server.hasArg("os_pass")) {
    preferences.putString("os_pass", server.arg("os_pass"));
    pref_os_pass = server.arg("os_pass");
  }

  if (server.hasArg("n2yo_key")) {
    preferences.putString("n2yo_key", server.arg("n2yo_key"));
    pref_n2yo_key = server.arg("n2yo_key");
    lastIssPassFetch = 0; // Forzar reconsulta inmediata
  }
  
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

  if (server.hasArg("utc_offset")) {
    pref_offset = server.arg("utc_offset").toInt() * 3600;
    pref_dst = server.arg("dst") == "1";
    configTime(pref_offset + (pref_dst ? 3600 : 0), 0, "pool.ntp.org", "time.nist.gov");
  }
  airportShownInitially = false;

  bool currentEnabled = false;
  if (currentState == STATE_RADAR && pref_show_radar) currentEnabled = true;
  else if (currentState == STATE_TIME && pref_show_time) currentEnabled = true;
  else if (currentState == STATE_WEATHER && pref_show_weather) currentEnabled = true;
  else if (currentState == STATE_MOON && pref_show_moon) {
    currentEnabled = true;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      float sun_angle = 180.0 + (sun_progress * 360.0);
      if (sun_angle > 360.0) sun_angle -= 360.0;
      float moon_fraction = getMoonPhaseFraction(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
      float moon_angle = sun_angle - (moon_fraction * 360.0);
      if (moon_angle < 0.0) moon_angle += 360.0;
      if (moon_angle < 180.0 || moon_angle > 360.0) {
        currentEnabled = false;
      }
    }
  }
  else if (currentState == STATE_HORIZON && pref_show_horizon) currentEnabled = true;
  else if (currentState == STATE_ISS && pref_show_iss) currentEnabled = true;
  else if (currentState == STATE_SUN && pref_show_sun) currentEnabled = true;
  else if (currentState == STATE_ZODIAC && pref_show_zodiac) currentEnabled = true;
  else if (currentState == STATE_ELECTRICITY && pref_show_electricity) currentEnabled = true;
  else if (currentState == STATE_ELEC_CLOCK && pref_show_elec_clock) currentEnabled = true;
  else if (currentState == STATE_AIR_QUALITY && pref_show_aqi) currentEnabled = true;
  
  if (!currentEnabled) {
    int prev = (int)currentState - 1;
    if (prev < 0) prev = STATE_MAX - 1;
    currentState = (DisplayState)prev;
    nextState();
    stateStartTime = millis();
    spr.fillSprite(TFT_BLACK);
  }

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
