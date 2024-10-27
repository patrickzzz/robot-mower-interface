<template>
  <div class="accordion mb-3">
    <div class="accordion-item">
      <h2 class="accordion-header">
        <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" aria-expanded="false" aria-controls="accordion-content-wifi-setup" data-bs-target="#accordion-content-wifi-setup" @click="searchWifis">
          Wifi-Setup
        </button>
      </h2>
      <div id="accordion-content-wifi-setup" class="accordion-collapse collapse">
        <div class="accordion-body">
          <p class="text-muted" style="font-size: 0.7em;">If no Wifi entered, or connection can't be established, an Access Point will be created.<br/>
            Current wifi: <i>{{ currentSSID }}</i> with IP <i>{{ currentIP }}</i> and hostname <i>{{ hostname }}</i><br/>
            <!-- if isAccessPoint: show info that it is access point mode -->
            <span v-if="isAccessPoint">(📡 Access Point Mode)</span>
          </p>
          <p class="text-danger" style="font-size: 0.7em;">After saving a Wifi-Connection, the mower interface will reboot and try to connect to the new Wifi. In the new Wifi it might be available <a target="_blank" :href="'http://' + hostname + '.local'">here</a>. If not, you might need to find out the ip address manually!</p>
          <form @submit.prevent="setWifi">
            <div class="row">
              <div class="col-xl-6">
                <div class="form-floating mb-3">
                  <input autocomplete="off" class="form-control" list="wifiOptions" id="ssid" v-model="ssid" placeholder="Wifi SSID" />
                  <label for="ssid">Connect to Wifi SSID..</label>
                  <datalist id="wifiOptions">
                    <option v-for="wifi in wifiOptions" :key="wifi" :value="wifi">{{ wifi }}</option>
                  </datalist>
                </div>
              </div>
              <div class="col-xl-6">
                <div class="form-floating mb-3">
                  <input autocomplete="off" type="password" class="form-control" id="password" v-model="password" placeholder="Wifi password" />
                  <label for="password">Wifi password</label>
                </div>
              </div>
            </div>
            <!-- Search Button & Spinner -->
            <div class="d-flex justify-content-between align-items-center">
              <div>
              <button type="button" class="btn btn-outline-secondary btn-sm" @click="searchWifis" :disabled="isLoading">
                <span v-if="isLoading" class="spinner-border spinner-border-sm" aria-hidden="true"></span>
                Search for Wireless LANs
              </button>
              </div>
              <button type="submit" class="btn btn-outline-primary btn-sm">Save Wifi</button>
            </div>
          </form>

          <Notification :show="showToast" :message="toastMessage" :bgClass="bgClass" @close="showToast = false" />
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import axios from 'axios';
import Notification from './Notification.vue';

export default {
  components: {
    Notification,
  },
  props: {
    currentSSID: String,
    currentIP: String,
    hostname: String,
    isAccessPoint: Boolean
  },
  data() {
    return {
      ssid: '',
      password: '',
      wifiOptions: [],  // To store available WiFi networks
      showToast: false,
      toastMessage: '',
      bgClass: '',
      isLoading: false, // To track loading state
    };
  },
  methods: {
    async setWifi() {
      try {
        const response = await axios.post('/wifi', {
          ssid: this.ssid,
          password: this.password,
        });
        this.toastMessage = 'Wifi settings saved successfully!';
        this.bgClass = 'text-bg-success';
        this.showToast = true;
        setTimeout(() => {
          this.showToast = false;
        }, 8000);
      } catch (error) {
        this.toastMessage = 'Error saving Wifi settings. Please try again.';
        this.bgClass = 'bg-danger';
        this.showToast = true;
        setTimeout(() => {
          this.showToast = false;
        }, 8000);
      }
    },
    async searchWifis() {
      this.isLoading = true;
      try {
        const response = await axios.get('/wifis');
        // Assuming the API returns an array of WiFi SSIDs, with possible duplicates
        // array unique values of response.data
        this.wifiOptions = response.data.filter((value, index, self) => self.indexOf(value) === index);
      } catch (error) {
        this.toastMessage = 'Error fetching Wifi networks.';
        this.bgClass = 'bg-danger';
        this.showToast = true;
        setTimeout(() => {
          this.showToast = false;
        }, 8000);
      } finally {
        this.isLoading = false;
      }
    }
  }
};
</script>

<style scoped>
</style>