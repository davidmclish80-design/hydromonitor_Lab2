import {defineStore} from 'pinia'
import {ref} from 'vue'

export const useAppStore = defineStore(
  "app",
  () => {
    // Helper: fetch JSON with timeout
    const fetchJSON = async (url) => {
      const controller = new AbortController();
      const signal = controller.signal;
      const timeoutId = setTimeout(() => controller.abort(), 60000);

      try {
        const res = await fetch(url, { method: "GET", signal });
        clearTimeout(timeoutId);

        if (!res.ok) {
          console.log("HTTP error:", res.status, await res.text());
          return null;
        }

        return await res.json();
      } catch (err) {
        clearTimeout(timeoutId);
        console.error("fetchJSON error:", err.message);
        return null;
      }
    };

    // 1) All data in range
    const getAllInRange = async (start, end) => {
      const json = await fetchJSON(`/api/climo/get/${start}/${end}`);
      if (!json) return [];

      // matches backend: {"status":"Success","data":[...]}
      if (json.status === "Success" && Array.isArray(json.data)) return json.data;

      // backend "not found" case
      return [];
    };

    // 2) Temperature MMAR
    const getTemperatureMMAR = async (start, end) => {
      const json = await fetchJSON(`/api/mmar/temperature/${start}/${end}`);
      if (!json) return null;

      // matches backend: {"Status_T":"Success","data": {min,max,avg,range}}
      if (json.Status_T === "Success" && json.data) return json.data;

      return null;
    };

    // 3) Humidity MMAR
    const getHumidityMMAR = async (start, end) => {
      const json = await fetchJSON(`/api/mmar/humidity/${start}/${end}`);
      if (!json) return null;

      // matches backend: {"status_H":"Success","data": {min,max,avg,range}}
      if (json.status_H === "Success" && json.data) return json.data;

      return null;
    };

    // 4) Frequency distribution
    const getFreqDistro = async (variable, start, end) => {
      const json = await fetchJSON(`/api/frequency/${variable}/${start}/${end}`);
      if (!json) return [];

      // matches backend: {"status":"Success","data":[...]}
      if (json.status === "Success" && Array.isArray(json.data)) return json.data;

      return [];
    };

    return {
      getAllInRange,
      getTemperatureMMAR,
      getHumidityMMAR,
      getFreqDistro,
    };
  },
  { persist: true }
);





