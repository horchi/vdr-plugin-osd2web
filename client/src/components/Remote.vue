<template>
<div v-show="menuItem.on" style="min-width:180px; min-height:385px">
    <img src="../assets/remotecontrol.jpg" width="162" height="378" border="0"  style="position:fixed; z-index:999" usemap="#remote" alt="">
    <map name="remote" @click="handleClick($event)">
   	  <area shape="circle" coords="37,36,10" alt="Power" />
   	  <area shape="rect" coords="27,60,59,75" alt="1" />
   	  <area shape="rect" coords="64,59,98,75" alt="2" />
   	  <area shape="rect" coords="103,59,137,75" alt="3" />
   	  <area shape="rect" coords="25,82,60,98" alt="4" />
   	  <area shape="rect" coords="65,82,98,99" alt="5" />
   	  <area shape="rect" coords="103,83,137,99" alt="6" />
   	  <area shape="rect" coords="26,106,60,121" alt="7" />
   	  <area shape="rect" coords="65,105,97,121" alt="8" />
   	  <area shape="rect" coords="104,106,137,122" alt="9" />
   	  <area shape="rect" coords="66,128,98,144" alt="0" />
   	  <area shape="circle" coords="37,157,13" alt="Volume+" />
   	  <area shape="circle" coords="37,190,13" alt="Volume-" />
   	  <area shape="circle" coords="125,157,13" alt="Channel+" />
   	  <area shape="circle" coords="125,190,13" alt="Channel-" />
   	  <area shape="circle" coords="81,174,8" alt="Mute" />
   	  <area shape="circle" coords="81,202,11" alt="Recordings" />
   	  <area shape="poly" coords="39,215,29,227,30,235,39,236,51,222,49,215,43,213" alt="Menu" />
   	  <area shape="poly" coords="119,213,127,219,135,230,132,235,127,236,119,231,114,223,113,215" alt="Back" />
   	  <area shape="poly" coords="33,289,31,298,43,310,51,308,54,300,41,287" alt="Subtitles" />
   	  <area shape="poly" coords="131,289,133,298,122,310,112,308,110,300,122,287" alt="Audio" />
   	  <area shape="poly" coords="58,230,70,223,82,223,91,223,105,231,96,240,87,237,78,236,69,240,66,241,58,232" alt="Up" />
   	  <area shape="poly" coords="111,237,119,249,121,259,119,275,113,285,109,284,101,276,104,270,107,260,106,253,101,245" alt="Right" />
   	  <area shape="poly" coords="66,281,75,285,86,285,95,281,106,291,100,295,87,300,73,300,58,291,57,289,58,289" alt="Down" />
   	  <area shape="poly" coords="51,236,61,245,56,256,57,266,61,277,52,285,47,278,42,264,43,252,49,240" alt="Left" />
   	  <area shape="circle" coords="81,261,17" alt="Ok" />
   	  <area shape="rect" coords="25,324,52,339" alt="Red" />
   	  <area shape="rect" coords="53,324,81,339" alt="Green" />
   	  <area shape="rect" coords="83,324,109,339" alt="Yellow" />
   	  <area shape="rect" coords="110,324,137,339" alt="Blue" />
   </map>
</div>
</template>
<script>
require("common").Icon.register({"remote":{"width":1408,"height":1792,"paths":[{"d":"M384 1344q0 80-56 136t-136 56-136-56-56-136 56-136 136-56 136 56 56 136zM896 1467q2 28-17 48-18 21-47 21h-135q-25 0-43-16.5t-20-41.5q-22-229-184.5-391.5t-391.5-184.5q-25-2-41.5-20t-16.5-43v-135q0-29 21-47 17-17 43-17h5q160 13 306 80.5t259 181.5q114 113 181.5 259t80.5 306zM1408 1469q2 27-18 47-18 20-46 20h-143q-26 0-44.5-17.5t-19.5-42.5q-12-215-101-408.5t-231.5-336-336-231.5-408.5-102q-25-1-42.5-19.5t-17.5-43.5v-143q0-28 20-46 18-18 44-18h3q262 13 501.5 120t425.5 294q187 186 294 425.5t120 501.5z"}]}}) 
export default {
    name: 'o2wRemote',
    data: function() {
        return { menuItem:{
            label: '',
            on:false,
            icon: 'remote',
            func: function(navComp){
               this.on= !this.on;
			   if (!navComp.$root.isActive){
				  navComp.$root.$socket.reopen();
			   	  //navComp.$root.$socket.open();  // Neue Verbindung aufmachen und so focus zurückholen
			   }
            }
        }}
    },
    methods: {
        handleClick(ev) {
            if (ev.target && ev.target.alt)
               this.$root.sendKey(ev.target.alt);
        }
    },
    created() {
        this.$root.$data.menuItems.push(this.menuItem);
    }
}
</script>
