<template>
  <div class="accordion mb-3">
    <div class="accordion-item">
      <h2 class="accordion-header">
        <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" aria-expanded="false" aria-controls="accordion-content-time-setup" data-bs-target="#accordion-content-time-setup">
          Time &amp; Date-Setup
        </button>
      </h2>
      <div id="accordion-content-time-setup" class="accordion-collapse collapse">
        <div class="accordion-body">
          <p class="text-muted" style="font-size: 0.7em;">
            With internet connection, UTC is set automatically. Here you can adjust to your timezone or set Date &amp; Time in Access Point Mode. After Mower complete shutdown, time will be reset.
          </p>
          <form @submit.prevent="saveTime">
            <div class="row justify-content-center">
              <div class="col-6 col-xl-4">
                <div class="form-floating mb-3">
                  <input type="date" v-model="date" class="form-control" />
                  <label for="date" class="form-label">Set current date</label>
                </div>
              </div>
              <div class="col-6 col-xl-4">
                <div class="form-floating mb-3">
                  <input type="time" v-model="time" class="form-control" />
                  <label for="time" class="form-label">Set current time</label>
                </div>
              </div>
            </div>
            <div class="text-end">
              <button type="submit" class="btn btn-sm btn-outline-primary">Save date and time</button>
            </div>
          </form>

          <!-- Toast Notification -->
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
  data() {
    return {
      date: '',
      time: '',
      timer: null,
      showToast: false,
      toastMessage: '',
      bgClass: 'text-bg-primary',
    };
  },
  mounted() {
    this.setCurrentDateTime();
    this.startTimer();
  },
  beforeDestroy() {
    clearInterval(this.timer);
  },
  methods: {
    setCurrentDateTime() {
      const now = new Date();
      this.date = now.toISOString().split('T')[0];
      this.time = now.toTimeString().split(':').slice(0, 2).join(':');
    },
    startTimer() {
      this.timer = setInterval(() => {
        this.setCurrentDateTime();
      }, 60000); // 60000 ms = 1 minute
    },
    saveTime() {
      axios.post('/date-time', { date: this.date, time: this.time })
          .then(response => {
            this.toastMessage = 'Time saved successfully!';
            this.showToast = true;
            this.bgClass = 'text-bg-success';
            setTimeout(() => {
              this.showToast = false;
            }, 8000);
            this.$emit('time-saved');
          })
          .catch(error => {
            this.toastMessage = 'Error saving time. Please try again.';
            this.showToast = true;
            this.bgClass = 'text-bg-danger';
            setTimeout(() => {
              this.showToast = false;
            }, 8000);
            console.error('Error saving time:', error);
          });
    }
  }
};
</script>

<style scoped>
</style>