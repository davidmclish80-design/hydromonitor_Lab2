<template>
  <v-container class="pa-6">
    <v-row>
      <!-- LEFT COLUMN: sliders + indicator -->
      <v-col cols="12" md="6">
        <h2 class="mb-4">Control</h2>

        <!-- Brightness -->
        <v-card class="mb-4" variant="outlined">
          <v-card-title>Brightness</v-card-title>
          <v-card-text>
            <v-slider
              v-model="controls.brightness"
              :min="0"
              :max="255"
              :step="1"
              thumb-label="always"
            />
            <div class="text-caption">Value: {{ controls.brightness }}</div>
          </v-card-text>
        </v-card>

        <!-- LED segments -->
        <v-card class="mb-4" variant="outlined">
          <v-card-title>Active LED Segments</v-card-title>
          <v-card-text>
            <v-slider
              v-model="controls.leds"
              :min="0"
              :max="7"
              :step="1"
              thumb-label="always"
            />
            <div class="text-caption">
              Active segments: <b>{{ controls.leds }}</b>
            </div>
          </v-card-text>
        </v-card>

        <!-- Indicator -->
        <v-card variant="outlined">
          <v-card-title>Indicator</v-card-title>
          <v-card-text class="d-flex align-center ga-4">
            <v-progress-circular
              :model-value="controls.brightness"
              :max="255"
              :rotate="0"
              :size="200"
              :width="15"
              :color="indicatorColor"
            >
              <div class="text-center">
                <div class="text-h6">{{ controls.leds }}/7</div>
                <div class="text-caption">LEDs</div>
              </div>
            </v-progress-circular>

            <div class="text-body-2">
              <div><b>Topic:</b> {{ CONTROL_TOPIC }}</div>
              <div><b>Last publish:</b> {{ lastPublishStatus }}</div>
            </div>
          </v-card-text>
        </v-card>
      </v-col>

      <!-- RIGHT COLUMN: color picker -->
      <v-col cols="12" md="6">
        <v-card variant="outlined">
          <v-card-title>Colour</v-card-title>
          <v-card-text>
            <!-- Vuetify color picker works best with a hex string -->
            <v-color-picker v-model="colorHex" mode="rgba" hide-inputs />

            <div class="mt-4">
              <div class="text-caption">
                RGBA:
                {{ controls.color.r }},
                {{ controls.color.g }},
                {{ controls.color.b }},
                {{ controls.color.a }}
              </div>
            </div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<script setup>
import { reactive, computed, watch, onMounted, onBeforeUnmount, ref } from "vue";
import { useMqttStore } from "@/store/mqttStore";

// MQTT store
const Mqtt = useMqttStore();

// ESP32 control topic (the one hardware SUBSCRIBES to)
const CONTROL_TOPIC = "620171852_sub";

// Reactive state required by lab schema
const controls = reactive({
  brightness: 255,
  leds: 7,
  color: { r: 255, g: 255, b: 255, a: 1 },
});

// For status text on page (helps you “prove it works”)
const lastPublishStatus = ref("No messages yet");

// Connect when page loads
onMounted(() => {
  Mqtt.connect();
   setTimeout( ()=>
    {
    Mqtt.subscribe("620171852")
    Mqtt.subscribe("620171852_sub")
    },1000)
});

// Disconnect/unsubscribe when leaving page
onBeforeUnmount(() => {
  if (Mqtt.unsubcribeAll) Mqtt.unsubcribeAll();
  else if (Mqtt.disconnect) Mqtt.disconnect();
});

/* -----------------------------
   Color picker binding (hex <-> rgba)
   Vuetify picker is easiest with a hex string.
-------------------------------- */
const colorHex = ref("#ffffff");

// Convert current controls.color -> hex when page starts
const toHex2 = (n) => n.toString(16).padStart(2, "0");
const rgbaToHex = (c) =>
  `#${toHex2(c.r)}${toHex2(c.g)}${toHex2(c.b)}`.toLowerCase();

colorHex.value = rgbaToHex(controls.color);

// When colorHex changes, update controls.color (RGB)
watch(colorHex, (hex) => {
  // supports #RRGGBB
  const h = hex.replace("#", "");
  if (h.length === 6) {
    controls.color.r = parseInt(h.slice(0, 2), 16);
    controls.color.g = parseInt(h.slice(2, 4), 16);
    controls.color.b = parseInt(h.slice(4, 6), 16);
    // leave alpha as-is (lab keeps a=1)
    controls.color.a = 1;
  }
});

// Used for the circular indicator color
const indicatorColor = computed(() => {
  const c = controls.color;
  return `rgba(${c.r},${c.g},${c.b},${c.a})`;
});

/* -----------------------------
   Implement Logic (Lab Pages 15-16):
   Publish when ANY control changes
-------------------------------- */
let timerId = null;

watch(
  controls,
  (val) => {
    clearTimeout(timerId);

    timerId = setTimeout(() => {
      const msgObj = {
        type: "controls",
        brightness: val.brightness,
        leds: val.leds,
        color: val.color,
      };

      const msg = JSON.stringify(msgObj);
      Mqtt.publish(CONTROL_TOPIC, msg);

      console.log("Published:", CONTROL_TOPIC, msg);
      lastPublishStatus.value = new Date().toLocaleTimeString();
    }, 300); // debounce (prevents spam while you drag sliders)
  },
  { deep: true }
);
</script>

<style scoped>
/* optional: keep it simple */
</style>
