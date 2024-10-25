<template>
  <div class="accordion mb-3">
    <div class="accordion-item">
      <h2 class="accordion-header">
        <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" aria-expanded="false" aria-controls="accordion-content-mowing-plan" data-bs-target="#accordion-content-mowing-plan">
          Mowing Plan
        </button>
      </h2>
      <div id="accordion-content-mowing-plan" class="accordion-collapse collapse">
        <div class="accordion-body">
          <form @submit.prevent="saveMowingPlan" autocomplete="off">
            <p class="text-muted" style="font-size: 0.7em;">
              For manual starts only, activate the following field, without selecting days below.<br />
              If deactivated, the mower will mow according to it's original schedule.
            </p>

            <div class="form-check form-switch mb-3" style="display: inline-block;">
              <input
                  class="form-check-input"
                  type="checkbox"
                  role="switch"
                  id="custom-mowing-plan-active"
                  v-model="isMowingPlanActive"
                  @change="toggleMowingPlan"
                  value="1"
              />
              <label class="form-check-label" for="custom-mowing-plan-active">
                Activate custom mowing plan
              </label>
            </div>

            <div v-if="isMowingPlanActive" class="custom-mowing-plan">
              <div class="custom-mowing-plan-days mb-3">
                <template v-for="(day, index) in days" :key="index">
                  <input
                      type="checkbox"
                      class="btn-check"
                      :id="'btn-check-day-' + day.toLowerCase()"
                      v-model="selectedDays[index]"
                      autocomplete="off"
                      :value="1"
                  />
                  <label class="btn btn-sm" :for="'btn-check-day-' + day.toLowerCase()">{{ day }}</label>
                </template>
              </div>
              <div class="custom-mowing-plan-time">
                <div class="form-floating mb-3 mx-1" style="display:inline-block;">
                  <input
                      type="time"
                      class="form-control form-control-sm"
                      id="custom-mowing-plan-time-start"
                      v-model="planTimeStart"
                  />
                  <label for="custom-mowing-plan-time-start">Start</label>
                </div>
                <div class="form-floating mb-3 mx-1" style="display:inline-block;">
                  <input
                      type="time"
                      class="form-control form-control-sm"
                      id="custom-mowing-plan-time-end"
                      v-model="planTimeEnd"
                  />
                  <label for="custom-mowing-plan-time-end">End</label>
                </div>
                <div class="mb-3" style="font-size:0.7em">
                  <!-- show little warning, if {{ currentDate }} {{ currentTime }} are unset, otherwise show date/time -->
                  <span v-if="!currentDate || !currentTime" class="text-danger">Date and time are not set on the mower currently. Mowing Plan will not be active.</span>
                  <span v-else class="text-muted">Date: {{ currentDate }}, Time: {{ currentTime }}</span>
                </div>
              </div>
              <div class="text-end">
                <button type="submit" class="btn btn-outline-primary btn-sm">Save Mowing Plan</button>
              </div>
            </div>
            <div v-if="!isMowingPlanActive && currentMowingPlanStatusActive" class="text-end">
              <button type="button" class="btn btn-outline-warning btn-sm" @click="saveMowingPlan">
                Deactivate Mowing Plan on Mower
              </button>
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
  props: {
    currentDate: String,
    currentTime: String,
    currentMowingPlanStatusActive: Boolean
  },
  components: {
    Notification,
  },
  data() {
    return {
      isMowingPlanActive: false,
      selectedDays: [false, false, false, false, false, false, false],
      days: ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'],
      planTimeStart: '',
      planTimeEnd: '',
      showToast: false,
      toastMessage: '',
      bgClass: '',
    };
  },
  methods: {
    toggleMowingPlan() {
      if (!this.isMowingPlanActive) {
        //this.selectedDays.fill(false);
        // we can leave days, for later activation
      }
    },
    async saveMowingPlan() {
      const payload = {
        customMowingPlanActive: this.isMowingPlanActive,
        days: this.selectedDays,
        planTimeStart: this.planTimeStart,
        planTimeEnd: this.planTimeEnd
      };

      try {
        const response = await axios.post('/mowing-plan', payload, {
          headers: {
            'Content-Type': 'application/json'
          }
        });
        this.toastMessage = 'Mowing plan saved successfully!';
        this.bgClass = 'text-bg-success';
        this.showToast = true;
        this.$emit('plan-saved');
        setTimeout(() => {
          this.showToast = false;
        }, 8000);
      } catch (error) {
        this.toastMessage = 'Error saving mowing plan!';
        this.bgClass = 'text-bg-error';
        this.showToast = true;
        setTimeout(() => {
          this.showToast = false;
        }, 8000);
      }
    },
    async fetchMowingPlan() {
      try {
        const response = await axios.get('/mowing-plan');

        // handle empty response
        if (!response.data) {
          console.log('No mowing plan found');
          return;
        }

        const { customMowingPlanActive, days, startTime, endTime } = response.data;

        this.isMowingPlanActive = customMowingPlanActive;
        this.selectedDays = days;
        this.planTimeStart = startTime;
        this.planTimeEnd = endTime;

        console.log('Fetched Mowing Plan:', response.data);
      } catch (error) {
        this.toastMessage = 'Error fetching mowing plan';
        this.bgClass = 'text-bg-error';
        this.showToast = true;
        setTimeout(() => {
          this.showToast = false;
        }, 8000);
      }
    },
    showNotification(message, type) {
      this.notification = { message, type };
      setTimeout(() => {
        this.notification = null; // Clear notification after a few seconds
      }, 3000);
    },
  },
  mounted() {
    this.fetchMowingPlan(); // Fetch the existing mowing plan when the component is mounted
  },
};
</script>

<style scoped>
</style>