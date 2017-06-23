<template>
    <div v-if="buttons">
        <div>class="replay-control" {{controlInfo}}</div>
        <div class="btn-group btn-group-sm justify-content-center">
            <a v-for="(button,index) in buttons" @click="$root.sendKey(button.key)" class="btn btn-secondary" :style="{'color':button.color}"><icon :name="button.icon" /></a>
        </div>
    </div>
</template>

<script>
var common=require("common");
common.Icon.register({"pause":{"width":1536,"height":1792,"paths":[{"d":"M1536 192v1408q0 26-19 45t-45 19h-512q-26 0-45-19t-19-45v-1408q0-26 19-45t45-19h512q26 0 45 19t19 45zM640 192v1408q0 26-19 45t-45 19h-512q-26 0-45-19t-19-45v-1408q0-26 19-45t45-19h512q26 0 45 19t19 45z"}]}}) ;
common.Icon.register({"play":{"width":1408,"height":1792,"paths":[{"d":"M1384 927l-1328 738q-23 13-39.5 3t-16.5-36v-1472q0-26 16.5-36t39.5 3l1328 738q23 13 23 31t-23 31z"}]}});
common.Icon.register({"stop":{"width":1536,"height":1792,"paths":[{"d":"M1536 192v1408q0 26-19 45t-45 19h-1408q-26 0-45-19t-19-45v-1408q0-26 19-45t45-19h1408q26 0 45 19t19 45z"}]}})
common.Icon.register({"fast-backward":{"width":1792,"height":1792,"paths":[{"d":"M1747 141q19-19 32-13t13 32v1472q0 26-13 32t-32-13l-710-710q-9-9-13-19v710q0 26-13 32t-32-13l-710-710q-9-9-13-19v678q0 26-19 45t-45 19h-128q-26 0-45-19t-19-45v-1408q0-26 19-45t45-19h128q26 0 45 19t19 45v678q4-10 13-19l710-710q19-19 32-13t13 32v710q4-10 13-19z"}]}})
common.Icon.register({"fast-forward":{"width":1792,"height":1792,"paths":[{"d":"M45 1651q-19 19-32 13t-13-32v-1472q0-26 13-32t32 13l710 710q9 9 13 19v-710q0-26 13-32t32 13l710 710q9 9 13 19v-678q0-26 19-45t45-19h128q26 0 45 19t19 45v1408q0 26-19 45t-45 19h-128q-26 0-45-19t-19-45v-678q-4 10-13 19l-710 710q-19 19-32 13t-13-32v-710q-4 10-13 19z"}]}})

const replayButtons= [{
    key:'FastRew',
    icon:'fast-backward'
},{
    key: 'Play',
    icon: 'play'
},{
    key: 'Pause',
    icon: 'pause'
},{
    key:'Stop',
    icon:'stop'
},{
    key:'FastFwd',
    icon:'fast-forward'
}];

export default {
    name: 'o2wReplayControl',
    data() {
        return {
            buttons:{},
            controlInfo:null
        }
    },
    created() {
        this.$root.$on("actual", (data) => {
            this.buttons= null;
            this.controlInfo=null;
        });
        this.$root.$on("replay", (data) => {
            this.controlInfo= data.filename;
        })
        this.$root.$on("replaycontrol", (data) => {
            this.buttons= replayButtons;
            if (data.play == 1)
                this.buttons[1].color= 'red'
            if (data.speed >= 0)
                this.buttons[data.forward == 1 ? 4:0].color= 'red'
        });
    }
}
</script>
