<template>
  <div>
    <div class="d-grid gap-2 d-md-block mb-4">
      <button class="btn btn-primary mx-1" @click="sendAction('/start')">Start</button>
      <button class="btn btn-secondary mx-1" @click="sendAction('/home')">Home</button>
      <button class="btn btn-danger mx-1" @click="sendAction('/stop')">Stop</button>

      <template v-if="!status.isIdle">
        <template v-if="status.isLocked">
          <button class="btn btn-info mx-1" @click="sendAction('/unlock')">Unlock</button>
        </template>
        <template v-else>
          <button class="btn btn-warning mx-1" @click="sendAction('/lock')">Lock</button>
        </template>
      </template>
    </div>

    <!-- Toast Notification -->
    <Notification :show="showToast" :message="toastMessage" :bgClass="bgClass" @close="showToast = false" />
  </div>
</template>

<script>
import axios from 'axios';
import Notification from './Notification.vue';

export default {
  components: {
    Notification
  },
  props: {
    status: {
      type: Object,
      required: true
    }
  },
  data() {
    return {
      showToast: false,
      toastMessage: '',
      bgClass: ''
    };
  },
  methods: {
    async sendAction(url) {
      try {
        const response = await axios.post(url);
        if (response.status === 200) {
          this.toastMessage = `Action sent to ${url} successfully!`;
          this.bgClass = 'text-bg-success';
          // emit button-pressed event
          setTimeout(() => {
            this.$emit('button-pressed');
          }, 500);
        } else {
          throw new Error('Non-OK response');
        }
      } catch (error) {
        this.toastMessage = `Error sending action to ${url}.`;
        this.bgClass = 'text-bg-danger';
      } finally {
        this.showToast = true;
        setTimeout(() => {
          this.showToast = false;
        }, 8000);
      }
    }
  }
};
</script>

<style scoped>
/* Optional: Styling for buttons if necessary */
</style>