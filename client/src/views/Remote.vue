<template>
<div v-show="menuItem.on" style="padding:10px 30px 10px 10px">
    <img src="../assets/remotecontrol.jpg" width="162" height="378" border="0" style="max-width: initial; max-height: initial;" usemap="#remote" alt="">
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
export default {
    name: 'o2vRemote',
    data: function() {
        return { menuItem:{
            label: '',
            on:false,
            iconClass: 'uk-icon-calculator',
            func: function(navComp){
               this.on= !this.on;
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
