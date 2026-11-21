<template>
<div v-if="menuItem.on" style="min-width:216px; min-height:432px">
    <!-- the remote control page (fb.html) scaled down, 'embedded' -> menu key brings the OSD into the browser -->
    <!-- fb.html is laid out for 360x720, shown at 60% -->
    <iframe src="/fb.html?embedded=1" title="Fernbedienung"
            style="position:fixed; z-index:999; width:360px; height:720px; border:0; border-radius:12px;
                   background:black; transform:scale(0.6); transform-origin:top left;
                   box-shadow:0 4px 16px rgba(0,0,0,0.5)"></iframe>
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
            hint: 'Fernbedienung ein-/ausblenden',
            func: function(navComp){
               this.on= !this.on;
			   if (!navComp.$root.isActive){
				  navComp.$root.$socket.reopen();
			   	  //navComp.$root.$socket.open();  // Neue Verbindung aufmachen und so focus zurückholen
			   }
            }
        }}
    },
    created() {
        this.$root.$data.menuItems.push(this.menuItem);
    }
}
</script>

