<template>
  <!-- Container: fluid + bg-surface (lab) -->
  <v-container fluid class="bg-surface d-flex justify-center">
    <div style="max-width: 1200px; width: 100%;">
      <!-- ROW 1 (pa-1) -->
      <v-row class="pa-1">
        <!-- Col 1 (unchanged => 6 because col2+col3 are 3 each) -->
        <v-col :cols="6">
          <v-sheet class="pa-2" height="250" rounded>
            <p class="text-subtitle-2 mb-2">Enter date range for Analysis</p>
            <v-divider class="mb-3" />

            <v-text-field
              v-model="start"
              label="Start date"
              type="date"
              density="compact"
              variant="solo-inverted"
              flat
              class="mr-5"
              style="max-width: 300px;"
            />

            <v-text-field
              v-model="end"
              label="End date"
              type="date"
              density="compact"
              variant="solo-inverted"
              flat
              style="max-width: 300px;"
            />

            <v-spacer />

            <v-btn
              class="text-caption mt-2"
              @click="updateCards(); updateLineCharts(); updateHumidityChart(); updateHistogramCharts(); updateScatterCharts(); updateHumidityScatterCharts();"
              text="Analyze"
              color="primary"
              variant="tonal"
            />
          </v-sheet>
        </v-col>

        <!-- Col 2 (cols=3, centered) -->
        <v-col :cols="3" class="d-flex justify-center">
          <v-card
            title="Temperature"
            width="250"
            variant="outlined"
            color="primary"
            density="compact"
            rounded="lg"
            align="center"
          >
            <v-card-item class="mb-n5">
            <div style="width: 100%;"> 
              <v-chip-group
                class="d-flex flex-row justify-center" 
                color="primaryContainer"
                variant="flat"
              >
                <v-tooltip text="Min" location="start">
                  <template #activator="{ props }">
                    <v-chip v-bind="props">{{ temperature.min }}</v-chip>                  
                  </template>
                </v-tooltip>

                <v-tooltip text="Range" location="top">
                  <template #activator="{ props }">
                    <v-chip v-bind="props">{{ temperature.range }}</v-chip>
                  </template>
                </v-tooltip>

                <v-tooltip text="Max" location="end">
                  <template #activator="{ props }">
                    <v-chip v-bind="props">{{ temperature.max }}</v-chip>
                  </template>
                </v-tooltip>
              </v-chip-group>
            </div>
            </v-card-item>

            <v-card-item align="center">
              <span class="text-h1 text-primary font-weight-bold">
                {{ temperature.avg }}
              </span>
            </v-card-item>
          </v-card>
        </v-col>

        <!-- Col 3 (cols=3, centered) -->
        <v-col :cols="3" class="d-flex justify-center">
          <v-card
            title="Humidity"
            width="250"
            variant="outlined"
            color="primary"
            density="compact"
            rounded="lg"
            align="center"
          >
            <v-card-item class="mb-n5">
              <v-chip-group
                class="d-flex flex-row justify-center"
                color="primaryContainer"
                variant="flat"
              >
                <v-tooltip text="Min" location="start">
                  <template #activator="{ props }">
                    <v-chip v-bind="props">{{ humidity.min }}</v-chip>
                  </template>
                </v-tooltip>

                <v-tooltip text="Range" location="top">
                  <template #activator="{ props }">
                    <v-chip v-bind="props">{{ humidity.range }}</v-chip>
                  </template>
                </v-tooltip>

                <v-tooltip text="Max" location="end">
                  <template #activator="{ props }">
                    <v-chip v-bind="props">{{ humidity.max }}</v-chip>
                  </template>
                </v-tooltip>
              </v-chip-group>
            </v-card-item>

            <v-card-item align="center">
              <span class="text-h1 text-primary font-weight-bold">
                {{ humidity.avg }}
              </span>
            </v-card-item>
          </v-card>
        </v-col>
      </v-row>

      <!-- ROW 2 (2 columns, both cols=12 => stacked) -->
      <v-row>
        <v-col :cols="12">
          <figure class="highcharts-figure">
            <div id="container"></div>
          </figure>
        </v-col>

        <v-col :cols="12">
          <figure class="highcharts-figure">
            <div id="container0"></div>
          </figure>
        </v-col>
      </v-row>

      <!-- ROW 3 (3 columns, all cols=12 => stacked) -->
      <v-row>
        <v-col :cols="12" style="border: 1px solid rgba(0,0,0,0.12); border-radius: 8px;">
          <figure class="highcharts-figure">
            <div id="container1"></div>
          </figure>
        </v-col>

        <v-col :cols="12">
          <figure class="highcharts-figure">
            <div id="container2"></div>
          </figure>
        </v-col>

        <v-col :cols="12">
          <figure class="highcharts-figure">
            <div id="container3"></div>
          </figure>
        </v-col>
      </v-row>
    </div>
  </v-container>
</template>


<script setup>
/** JAVASCRIPT HERE */

//APPARENTLY V-TOOLTIP DOESNT USE ALIGN="CENTER"
//WRITING WORDS BETWEEN TEMPLATEs LEAD TO THE PAGE DISPLAYING THE WORDS WRITTEN THERE

// IMPORTS
import { ref, reactive, computed, onMounted, onBeforeUnmount } from "vue";
import { useRoute, useRouter } from "vue-router";

import Highcharts from "highcharts";
import more from "highcharts/highcharts-more";
import Exporting from "highcharts/modules/exporting";
Exporting(Highcharts);
more(Highcharts);

import { useMqttStore } from "@/store/mqttStore";
import { storeToRefs } from "pinia";
import { useAppStore } from "@/store/appStore";

// VARIABLES
const router = useRouter();
const route = useRoute();

const Mqtt = useMqttStore();
const { payload,payloadTopic } = storeToRefs(Mqtt);

const AppStore = useAppStore();

// per lab: start/end refs bound via v-model
const start = ref("");
const end = ref("");

// per lab: reactive temperature/humidity objects initialized like this
const temperature = reactive({ min: 0, max: 0, avg: 0, range: 0 });
const humidity = reactive({ min: 0, max: 0, avg: 0, range: 0 });

// chart refs
const tempChart = ref(null);        // container
const humidChart = ref(null);       // container0
const histogramChart = ref(null);   // container1
const scatterTHI = ref(null);       // container2 (Temp vs HI)
const scatterHHI = ref(null);       // container3 (Humid vs HI)

// live computed cards
const liveTemperature = computed(() => {
  const t = payload.value?.temperature;
  return (t !== undefined && t !== null) ? `${Number(t).toFixed(2)} °C` : "--";
});
const liveHeatindex = computed(() => {
  const h = payload.value?.heatindex;
  return (h !== undefined && h !== null) ? `${Number(h).toFixed(2)} °C` : "--";
});
const liveHumidity = computed(() => {
  const rh = payload.value?.humidity;
  return (rh !== undefined && rh !== null) ? `${Number(rh).toFixed(2)} %` : "--";
});

import { watch } from "vue";

watch([start, end], () => {
  console.log("start:", start.value, "end:", end.value);
});

function toUnixRange() {
  if (!start.value || !end.value) return null;

  // start of start-day
  const s = new Date(`${start.value}T00:00:00`);
  // end of end-day
  const e = new Date(`${end.value}T23:59:59`);

  const startDate = Math.floor(s.getTime() / 1000);
  const endDate = Math.floor(e.getTime() / 1000);

  if (!Number.isFinite(startDate) || !Number.isFinite(endDate)) return null;
  if (startDate > endDate) return null;

  return { startDate, endDate };
}

// Create graphs (base shapes)
function createCharts() {
  // 1) Temperature + Heat Index line chart (container)
  tempChart.value = Highcharts.chart("container", {
    chart: { zoomType: "x",backgroundColor: "#101010", },
    title: { text: "Temperature and Heat Index Analysis", align: "left" },
    subtitle: {
      text:
        'The heat index, also known as the "apparent temperature," is a measure that combines air temperature and relative humidity to assess how hot it feels to the human body. The relationship between heat index and air temperature is influenced by humidity levels. As humidity increases, the heat index also rises, making the perceived temperature higher than the actual air temperature.'
    },
    xAxis: { type: "datetime", title: { text: "Time" } },
    yAxis: { title: { text: "Air Temperature & Heat Index" }, labels: { format: "{value} °C" } },
    tooltip: { shared: true },
    series: [
      { name: "Temperature", type: "spline", data: [], turboThreshold: 0 },
      { name: "Heat Index", type: "spline", data: [], turboThreshold: 0 }
    ]
  });

  // 2) Humidity line chart (container0)
  humidChart.value = Highcharts.chart("container0", {
    chart: { zoomType: "x",backgroundColor: "#101010", },
    title: { text: "Humidity Analysis", align: "left" },
    xAxis: { type: "datetime", title: { text: "Time" } },
    yAxis: { title: { text: "" }, labels: { format: "{value} %" } },
    tooltip: { shared: true },
    series: [{ name: "Humidity", type: "spline", data: [], turboThreshold: 0 }]
  });

  // 3) Frequency distribution column chart (container1)
  histogramChart.value = Highcharts.chart("container1", {
    chart: { type: "column", zoomType: "x",backgroundColor: "#101010", },
    title: { text: "Frequency Distribution Analysis", align: "left" },
    xAxis: { title: { text: "Bins" } },
    yAxis: { title: { text: "Count" } },
    tooltip: { shared: true },
    series: [
      { name: "Temperature", data: [], turboThreshold: 0 },
      { name: "Humidity", data: [], turboThreshold: 0 },
      { name: "Heat Index", data: [], turboThreshold: 0 }
    ]
  });

  // 4) Scatter: Temp vs Heat Index (container2)
  scatterTHI.value = Highcharts.chart("container2", {
    chart: { type: "scatter", zoomType: "x",backgroundColor: "#101010", },
    title: { text: "Temperature & Heat Index Correlation Analysis", align: "left" },
    subtitle: { text: "Visualize the relationship between Temperature and Heat Index as well as revealing patterns or trends in the data" },
    xAxis: { title: { text: "Temperature" }, labels: { format: "{value} °C" } },
    yAxis: { title: { text: "Heat Index" }, labels: { format: "{value} °C" } },
    tooltip: { pointFormat: "Temperature: {point.x} °C <br/> Heat Index: {point.y} °C" },
    series: [{ name: "Analysis", data: [], turboThreshold: 0 }]
  });

  // 5) Scatter: Humidity vs Heat Index (container3)
  scatterHHI.value = Highcharts.chart("container3", {
    chart: { type: "scatter", zoomType: "x",backgroundColor: "#101010", },
    title: { text: "Humidity & Heat Index Correlation Analysis", align: "left" },
    subtitle: { text: "Visualize the relationship between Humidity and Heat Index as well as revealing patterns or trends in the data" },
    xAxis: { title: { text: "Humidity" }, labels: { format: "{value} %" } },
    yAxis: { title: { text: "Heat Index" }, labels: { format: "{value} °C" } },
    tooltip: { pointFormat: "Humidity: {point.x} % <br/> Heat Index: {point.y} °C" },
    series: [{ name: "Analysis", data: [], turboThreshold: 0 }]
  });
}

// UPDATE FUNCTIONS (called by Analyze)
const updateCards = async () => {
  const r = toUnixRange();
  if (!r) return;

  const temp = await AppStore.getTemperatureMMAR(r.startDate, r.endDate);
  const humid = await AppStore.getHumidityMMAR(r.startDate, r.endDate);

  // ✅ CHANGED: temp/humid are OBJECTS now (not arrays)
  if (temp) {
    temperature.max = Number(temp.max).toFixed(1);
    temperature.min = Number(temp.min).toFixed(1);
    temperature.avg = Number(temp.avg).toFixed(1);
    temperature.range = Number(temp.range).toFixed(1);
  }

  if (humid) {
    humidity.max = Number(humid.max).toFixed(1);
    humidity.min = Number(humid.min).toFixed(1);
    humidity.avg = Number(humid.avg).toFixed(1);
    humidity.range = Number(humid.range).toFixed(1);
  }
};

const updateLineCharts = async () => {
  const r = toUnixRange();
  if (!r) return;

  const data = await AppStore.getAllInRange(r.startDate, r.endDate);

  console.log("Range:", r);
  console.log("Rows returned:", data?.length, data?.slice?.(0, 3));

  const temperatureSeries = [];
  const heatindexSeries = [];

  data?.forEach((row) => {
    temperatureSeries.push({ x: row.timestamp * 1000, y: parseFloat(Number(row.temperature).toFixed(2)) });
    heatindexSeries.push({ x: row.timestamp * 1000, y: parseFloat(Number(row.heatindex).toFixed(2)) });
  });

  tempChart.value?.series?.[0]?.setData(temperatureSeries);
  tempChart.value?.series?.[1]?.setData(heatindexSeries);
};

const updateHumidityChart = async () => {
  const r = toUnixRange();
  if (!r) return;

  const data = await AppStore.getAllInRange(r.startDate, r.endDate);

  const humiditySeries = [];

  data?.forEach((row) => {
    humiditySeries.push({ x: row.timestamp * 1000, y: parseFloat(Number(row.humidity).toFixed(2)) });
  });

  humidChart.value?.series?.[0]?.setData(humiditySeries);
};

const updateHistogramCharts = async () => {
  const r = toUnixRange();
  if (!r) return;

  const temp = await AppStore.getFreqDistro("temperature", r.startDate, r.endDate);
  const humid = await AppStore.getFreqDistro("humidity", r.startDate, r.endDate);
  const hi = await AppStore.getFreqDistro("heatindex", r.startDate, r.endDate);

  const temperatureBins = [];
  const humidityBins = [];
  const heatindexBins = [];

  temp?.forEach((row) => temperatureBins.push({ x: row["_id"], y: row["count"] }));
  humid?.forEach((row) => humidityBins.push({ x: row["_id"], y: row["count"] }));
  hi?.forEach((row) => heatindexBins.push({ x: row["_id"], y: row["count"] }));

  histogramChart.value?.series?.[0]?.setData(temperatureBins);
  histogramChart.value?.series?.[1]?.setData(humidityBins);
  histogramChart.value?.series?.[2]?.setData(heatindexBins);
};

const updateScatterCharts = async () => {
  const r = toUnixRange();
  if (!r) return;

  const data = await AppStore.getAllInRange(r.startDate, r.endDate);

  const points = [];
  data?.forEach((row) => {
    points.push([Number(row.temperature), Number(row.heatindex)]);
  });

  scatterTHI.value?.series?.[0]?.setData(points);
};

const updateHumidityScatterCharts = async () => {
  const r = toUnixRange();
  if (!r) return;

  const data = await AppStore.getAllInRange(r.startDate, r.endDate);

  const points = [];
  data?.forEach((row) => {
    points.push([Number(row.humidity), Number(row.heatindex)]);
  });

  scatterHHI.value?.series?.[0]?.setData(points);
};

// FUNCTIONS
onMounted(() => {
  createCharts();

  // (Optional) if you want the live cards to work on this page too:
  /**setTimeout( ()=>
  {
    Mqtt.subscribe("620171852")
    Mqtt.subscribe("620171852_sub")
  },3000)*/
});

onBeforeUnmount(() => {
  // if you enabled MQTT here, clean up:
  //Mqtt.unsubcribeAll();
});
</script>

<style scoped>
#container { height: 380px; }
#container0, #container1, #container2, #container3 { height: 300px; }

.highcharts-figure {
  background: #101010;
  padding: 12px;
  border-radius: 8px;
}

</style>

