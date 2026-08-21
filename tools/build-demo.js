// ============================================================================
//  Erzeugt eine eigenstaendige Demo-Seite (demo.html) aus src/web_page.h.
//
//  Die Demo laeuft OHNE ESP32 und OHNE Server: ein eingebauter Simulator
//  ersetzt die WebSocket-Verbindung. Einfach demo.html im Browser oeffnen.
//
//  Start:  node tools/build-demo.js   ->  erzeugt demo.html im Projektordner
// ============================================================================
const fs = require('fs');
const path = require('path');

const WEB_PAGE_H = path.join(__dirname, '..', 'src', 'web_page.h');
const OUT = path.join(__dirname, '..', 'demo.html');

const src = fs.readFileSync(WEB_PAGE_H, 'utf8');
const m = /const char index_html\[\] PROGMEM = R"rawliteral\(([\s\S]*?)\)rawliteral";/.exec(src);
if (!m) {
    console.error('Fehler: index_html in web_page.h nicht gefunden.');
    process.exit(1);
}
let html = m[1];

// Manifest/Icon verweisen auf den ESP32-Server -> in der lokalen Datei entfernen.
html = html.replace(/<link rel="manifest"[^>]*>/g, '');
html = html.replace(/<link rel="apple-touch-icon"[^>]*>/g, '');
html = html.replace(/<link rel="icon"[^>]*>/g, '');

// Simulator: ersetzt window.WebSocket, damit die Seite ohne ESP32 lebt.
const simulator = `
<script>
(function(){
  const state = {
    mode:3, left:80, right:80, logo:80, global:80, autoBrightness:0,
    rtc:true, ntp:true, ip:'Demo', rssi:-55, wifi:true, ap:false,
    ssid:'Demo (ohne ESP32)', firmware:'Demo',
    sched:{tMorning:6,tDay:8,tEvening:18,tNight:23,bMorning:90,bDay:90,bEveStart:60,bEveEnd:25},
    presets:[]
  };
  let overrideActive=false, overrideWindow=-1;
  const clamp=(v,lo,hi)=>{v=parseInt(v);if(isNaN(v))return lo;return v<lo?lo:(v>hi?hi:v);};
  function win(){const d=new Date();const t=d.getHours()*60+d.getMinutes();const s=state.sched;
    if(t>=s.tNight*60||t<s.tMorning*60)return 0;if(t<s.tDay*60)return 1;if(t<s.tEvening*60)return 2;return 3;}
  function autoB(){const d=new Date();const t=d.getHours()*60+d.getMinutes();const s=state.sched;
    if(t>=s.tNight*60||t<s.tMorning*60)return 0;
    if(t<s.tDay*60){let p=(t-s.tMorning*60)/(s.tDay*60-s.tMorning*60);return Math.round(Math.max(0,Math.min(1,p))*s.bMorning);}
    if(t<s.tEvening*60)return s.bDay;
    let p=(t-s.tEvening*60)/(s.tNight*60-s.tEvening*60);p=Math.max(0,Math.min(1,p));
    return Math.round(s.bEveStart+(s.bEveEnd-s.bEveStart)*p);}
  const two=n=>String(n).padStart(2,'0');
  function status(){const d=new Date();state.autoBrightness=autoB();const s=Object.assign({},state);
    s.time=two(d.getHours())+':'+two(d.getMinutes())+':'+two(d.getSeconds());
    s.date=two(d.getDate())+'.'+two(d.getMonth()+1)+'.'+d.getFullYear();return JSON.stringify(s);}
  function overrideAuto(){if(state.mode===3){state.mode=1;overrideActive=true;overrideWindow=win();}}
  function handle(msg){let doc;try{doc=JSON.parse(msg);}catch(e){return;}
    if(typeof doc.wifiSsid==='string'){if(doc.wifiSsid)state.ssid=doc.wifiSsid;return;}
    if(typeof doc.mode==='number'&&doc.mode>=0&&doc.mode<=12){state.mode=doc.mode;
      if(doc.mode===3){overrideActive=false;}else{overrideActive=true;overrideWindow=win();}}
    if(doc.left!==undefined){state.left=clamp(doc.left,0,100);overrideAuto();}
    if(doc.right!==undefined){state.right=clamp(doc.right,0,100);overrideAuto();}
    if(doc.logo!==undefined){state.logo=clamp(doc.logo,0,100);overrideAuto();}
    if(doc.global!==undefined)state.global=clamp(doc.global,0,100);
    ['tMorning','tDay','tEvening','tNight'].forEach(k=>{if(doc[k]!==undefined)state.sched[k]=clamp(doc[k],0,23);});
    ['bMorning','bDay','bEveStart','bEveEnd'].forEach(k=>{if(doc[k]!==undefined)state.sched[k]=clamp(doc[k],0,100);});
    if(typeof doc.savePreset==='string'&&doc.savePreset){let p=state.presets.find(x=>x.name===doc.savePreset);
      if(!p&&state.presets.length<6){p={slot:state.presets.length,name:doc.savePreset};state.presets.push(p);}
      if(p){p.left=state.left;p.right=state.right;p.logo=state.logo;}}
    if(typeof doc.applyPreset==='number'){const p=state.presets.find(x=>x.slot===doc.applyPreset);
      if(p){state.left=p.left;state.right=p.right;state.logo=p.logo;state.mode=1;overrideActive=true;overrideWindow=win();}}
    if(typeof doc.deletePreset==='number'){state.presets=state.presets.filter(x=>x.slot!==doc.deletePreset);
      state.presets.forEach((x,i)=>x.slot=i);}
  }
  class FakeWebSocket{
    constructor(){this.readyState=0;const self=this;
      setTimeout(()=>{self.readyState=1;if(self.onopen)self.onopen();
        if(self.onmessage)self.onmessage({data:status()});},50);
      this._t=setInterval(()=>{if(self.readyState===1&&self.onmessage){
        if(overrideActive&&state.mode!==3){const w=win();
          if(w>=0&&overrideWindow>=0&&w!==overrideWindow){state.mode=3;overrideActive=false;}}
        self.onmessage({data:status()});}},1000);}
    send(msg){handle(msg);if(this.onmessage)this.onmessage({data:status()});}
    close(){this.readyState=3;clearInterval(this._t);}
  }
  FakeWebSocket.CONNECTING=0;FakeWebSocket.OPEN=1;FakeWebSocket.CLOSING=2;FakeWebSocket.CLOSED=3;
  window.WebSocket=FakeWebSocket;
})();
</script>
`;

html = html.replace('<body>', '<body>\n' + simulator);

fs.writeFileSync(OUT, html);
console.log('demo.html erzeugt:', OUT);
console.log('Einfach im Browser oeffnen - laeuft ohne ESP32 und ohne Server.');
