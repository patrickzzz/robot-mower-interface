<template>
  <div id="app">
    <div class="container">
      <div class="row justify-content-center mb-5">
        <div class="col-12 col-sm-10 col-md-8 col-lg-6 text-center">
          <h1 class="my-4">Robot Mower 🤖🚜</h1>
          <div v-if="statusLoaded">
            <MowerActions @button-pressed="fetchStatus" :status="status" />
            <MowerStatus :status="status" />
            <div class="alert alert-warning text-muted" style="font-size: 0.7em;" role="alert" v-if="!status.date || !status.time">
              Date and time are not set on the mower currently. Set it <a href="#accordion-content-time-setup" data-bs-toggle="collapse" @click="scrollTo('#accordion-content-time-setup')">here</a>.<br />
              In Access Point mode, it will get reset on mower shutdown. In Wifi mode with internet connection, it will be set automatically.
            </div>

            <MowingPlan @plan-saved="fetchStatus" :currentDate="status.date" :currentTime="status.time" :currentMowingPlanStatusActive="status.mowingPlanActive" />
            <WifiSetup :currentSSID="status.ssid" :currentIP="status.ip" :isAccessPoint="status.isAccessPoint" :hostname="status.hostname" />
            <SetDateAndTime @time-saved="fetchStatus" />
            <LogMessages />
          </div>
          <div v-else>
            <p>Loading status, please wait...</p>
            <div class="spinner-border" role="status">
              <span class="visually-hidden">Loading...</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import axios from 'axios';
import MowerActions from './components/MowerActions.vue';
import MowerStatus from './components/MowerStatus.vue';
import MowingPlan from "./components/MowingPlan.vue";
import WifiSetup from "./components/WifiSetup.vue";
import SetDateAndTime from "./components/SetDateAndTime.vue";
import LogMessages from './components/LogMessages.vue';

export default {
  name: 'App',
  components: {
    MowerActions,
    MowerStatus,
    MowingPlan,
    WifiSetup,
    SetDateAndTime,
    LogMessages
  },
  data() {
    return {
      status: {
        date: '',
        time: '',
        isCharging: false,
        isLocked: false,
        isEmergency: false,
        isIdle: false,
        isAccessPoint: false,
        ssid: '',
        ip: '',
        hostname: '',
        mowingPlanActive: false
      },
      statusLoaded: false
    };
  },
  methods: {
    enableBody() {
      document.body.classList.add('mounted');
    },
    scrollTo(id) {
      document.querySelector(id).scrollIntoView({ behavior: 'smooth' });
    },
    async fetchStatus() {
      try {
        const response = await axios.get('/status');
        this.status = response.data;
        this.statusLoaded = true;
        console.log('Status fetched:', this.status);
      } catch (error) {
        console.error('Error fetching status:', error);
      }
    }
  },
  mounted() {
    this.enableBody();
    this.fetchStatus();
    setInterval(this.fetchStatus, 15000); // Fetch status every 30 seconds
  }
};
</script>

<style scoped>
#app {
}
</style>