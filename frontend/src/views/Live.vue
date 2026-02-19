<template>
  <v-container fluid class="bg-surface d-flex justify-center">
    <!-- We wrap content in a max-width box like the lab says -->
    <div style="max-width: 1200px; width: 100%;">
      
      <!-- ROW 1 -->
      <v-row>
        <!-- Column 1 (cols=9): Temperature chart -->
        <v-col :cols="9">
            <!--div class="chart-title">Temperature Analysis (Live)</div-->
            <figure class="highcharts-figure">
                <div id="container"></div>
            </figure>
        </v-col>
            

        <!-- Column 2 (cols=3): Cards -->
        <v-col :cols="3">
          <!-- Card 1: Temperature -->
          <v-card class="mb-5" max-width="500" color="primaryContainer" subtitle="Temperature">
            <v-card-item>
              <span class="text-h3 text-onPrimaryContainer">
                {{ temperature }}
              </span>
            </v-card-item>
          </v-card>

          <!-- Card 2: Heat Index -->
          <v-card class="mb-5" max-width="500" color="tertiaryContainer" subtitle="Heat Index (Feels like)">
            <v-card-item>
              <span class="text-h3 text-onTertiaryContainer">
                {{ heatindex }}
              </span>
            </v-card-item>
          </v-card>

          <!-- Card 3: Humidity -->
          <v-card class="mb-5" max-width="500" color="secondaryContainer" subtitle="Humidity">
            <v-card-item>
              <span class="text-h3 text-onSecondaryContainer">
                {{ humidity }}
              </span>
            </v-card-item>
          </v-card>
        </v-col>
      </v-row>

      <!-- ROW 2 -->
      <v-row justify="start">
        <!-- Column 1 (cols=9): Humidity chart -->
        <v-col :cols="9">
          <figure class="highcharts-figure">
            <div id="container1"></div>
          </figure>
        </v-col>

        <!-- Column 2 (cols=3): empty space (matches layout), removed the following from this column[<v-col :cols="3"></v-col>]-->
        
      </v-row>

    </div>
  </v-container>
</template>


<script setup>
/** JAVASCRIPT HERE */

// IMPORTS
import { useMqttStore } from "@/store/mqttStore";
import {storeToRefs} from "pinia"
import { ref,reactive,watch ,onMounted,onBeforeUnmount,computed } from "vue";  
import { useRoute ,useRouter } from "vue-router";

import{useAppStore} from "@/store/appStore";

import Highcharts from "highcharts";
import more from "highcharts/highcharts-more";
import Exporting from "highcharts/modules/exporting";

Exporting(Highcharts);
more(Highcharts);

const tempHiChart = ref(null); // chart object
const humChart=ref(null);
//const check=ref(null);   DOESN'T WORK

const CreateCharts = async () => {

tempHiChart.value = Highcharts.chart("container", {
    chart: { zoomType: "x", backgroundColor: "transparent", },
    title: { text: "Temperature Analysis (Live)", align: "left" },     // "center" for align works too so "right" also works

    yAxis: {
      title: { text: " ", style: { color: "#000000" } },
      labels: { format: "{value} °C" },
    },

    xAxis: {
      type: "datetime",
      title: { text: "Time", style: { color: "#000000" } },
    },

    tooltip: { shared: true },

    series: [
      {
        name: "Temperature",
        type: "spline",
        data: [],   
        turboThreshold: 0,
        color: Highcharts.getOptions().colors[0],
      },
      {
        name: "Heat Index",
        type: "spline",
        data: [],
        turboThreshold: 0,
        color: Highcharts.getOptions().colors[1],
      },
    ],
  });

  humChart.value= Highcharts.chart("container1", {
    chart: {zoomType: "x", backgroundColor: "transparent",},
    title: {text: "Humidity Analysis (Live)", align:"left"},

    yAxis: {
        title:" ",labels: {format: "{value} %"},
    },

    xAxis: {
        type: "datetime",
        title: {text: "Time", style:{color:"#000000"}}
    },

    tooltip: {shared:true},

    series: [
        {
            name: " ",
            type: "spline",
            data: [],
            turbothreshold: 0,
            color: Highcharts.getOptions().colors[0]
        }
    ]
  })
};
    // TRIED TO SO IF THE BELOW WOULD AFFECT THE SECOND CHART AND NOPE IT ONLY STOPPED THE FIRST CHART, AKA CREATED ISSUES...
  /**check.value = Highcharts.chart("container", {
    chart: { zoomType: "x", backgroundColor: "transparent", },
    title: { text: "Temperature Analysis (Live)", align: "left" },     // "center" for align works too so "right" also works

    yAxis: {
      title: { text: " ", style: { color: "#000000" } },
      labels: { format: "{value} °C" },
    },

    xAxis: {
      type: "datetime",
      title: { text: "Time", style: { color: "#000000" } },
    },

    tooltip: { shared: true },

    series: [
      {
        name: "Temperature",
        type: "spline",
        data: [],
        turboThreshold: 0,
        color: Highcharts.getOptions().colors[0],
      },
      {
        name: "Heat Index",
        type: "spline",
        data: [],
        turboThreshold: 0,
        color: Highcharts.getOptions().colors[1],
      },
    ],
  });*/


 
// VARIABLES
const router      = useRouter();
const route       = useRoute();  

const Mqtt        = useMqttStore();
const {payload, payloadTopic}=storeToRefs(Mqtt);

const points =ref(10); //specify a quantity of points to be shown on the live graph simultaneously
const shift= ref(false); // delete a point from the left side and append a new point to the right side of the graph



//LET THE MADNESS BEGIN BELOW

// Card display values (formatted) constants below for temperature to humidity are essentially the same as the code in lab3, page3.
const temperature = computed(() => {
  const t = payload.value?.temperature;
  return (t !== undefined && t !== null) ? `${Number(t).toFixed(2)} °C` : "--";
});

const heatindex = computed(() => {
  const h = payload.value?.heatindex;
  return (h !== undefined && h !== null) ? `${Number(h).toFixed(2)} °C` : "--";
});

const humidity = computed(() => {
  const rh = payload.value?.humidity;
  return (rh !== undefined && rh !== null) ? `${Number(rh).toFixed(2)} %` : "--";
});

watch(payload, (data)=>{
    // ✅ safety: if charts not ready yet, don't try to add points
    if(!tempHiChart.value || !humChart.value) return;

    if(points.value >0){points.value --;}
    else{shift.value=true;}

    tempHiChart.value.series[0].addPoint({y:parseFloat(data.temperature), x: data.timestamp *1000}, true, shift.value);

    tempHiChart.value.series[1].addPoint({y:parseFloat(data.heatindex), x: data.timestamp*1000 }, true, shift.value);

    humChart.value.series[0].addPoint({y:parseFloat(data.humidity), x: data.timestamp*1000}, true, shift.value);
})

/**watch(payload, (data)=>{

    // ✅ safety: if charts not ready yet, don't try to add points
    if(!tempHiChart.value || !humChart.value) return;

    if(points.value >0){points.value --;}
    else{shift.value=true;}

    tempHiChart.value.series[0].addPoint(
      { y: parseFloat(data.temperature), x: data.timestamp * 1000 },
      true,
      shift.value
    );

    tempHiChart.value.series[1].addPoint(
      { y: parseFloat(data.heatindex), x: data.timestamp * 1000 },
      true,
      shift.value
    );

    // ✅ ADD THIS: humidity chart live update
    humChart.value.series[0].addPoint(
      { y: parseFloat(data.humidity), x: data.timestamp * 1000 },
      true,
      shift.value
    );

})*/



const updateLineCharts = async ()=>{
    if(!!start.value && !!end.value){
        // Convert output from Textfield components to 10 digit timestamps
        let startDate = new Date(start.value).getTime() / 1000;
        let endDate = new Date(end.value).getTime() / 1000;
        // Fetch data from backend
        const data = await AppStore.getAllInRange(startDate,endDate);
        // Create arrays for each plot
        let temperature = [];
        let heatindex = [];
        // Iterate through data variable and transform object to format recognized by highcharts
        data.forEach(row => {
        temperature.push({"x": row.timestamp * 1000, "y": parseFloat(row.temperature.toFixed(2)) });
        heatindex.push({ "x": row.timestamp * 1000,"y": parseFloat(row.heatindex.toFixed(2)) });});

        // Add data to Temperature and Heat Index chart
        tempChart.value.series[0].setData(temperature);
        tempChart.value.series[1].setData(heatindex);}}

// FUNCTIONS
onMounted(()=>{
    CreateCharts();
    Mqtt.connect();

    

    setTimeout( ()=>
    {
    Mqtt.subscribe("620171852")
    Mqtt.subscribe("620171852_sub")
    },3000)
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    
});


onBeforeUnmount(()=>{
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
    Mqtt.unsubcribeAll();
});


</script>


<style scoped>
/** CSS STYLE HERE */
#container, #container1 {
  height: 300px;
  width: 100%;
}

figure {
    border:2px solid black
}

.highcharts-figure {
  background: #101010;
  padding: 12px;
  border-radius: 8px;
}

/**
.chart-title{
  color: white;
  font-weight: 600;
  margin-bottom: 6px;
}*/

</style>