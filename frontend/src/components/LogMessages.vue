<template>
  <div class="accordion mb-3">
    <div class="accordion-item">
      <h2 class="accordion-header">
        <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" aria-expanded="false" aria-controls="accordion-content-logs" data-bs-target="#accordion-content-logs">
          Logs &amp; Debugging
        </button>
      </h2>
      <div id="accordion-content-logs" class="accordion-collapse collapse">
        <div class="accordion-body">
          <form @submit.prevent>
            <textarea
                v-model="logMessages"
                class="w-100"
                rows="14"
                readonly
                style="white-space: pre; overflow: auto;font-size: 0.75em;font-family: Courier New, monospace;"
                wrap="off"
            ></textarea>
          </form>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import axios from 'axios';

export default {
  data() {
    return {
      logMessages: '',
      autoRefresh: true,
      refreshLogsTimer: null,
    };
  },
  methods: {
    refreshLogMessages() {
      axios.get('/log-messages')
          .then(response => {
            this.logMessages = response.data;
            this.$nextTick(() => {
              const textarea = this.$el.querySelector('textarea');
              textarea.scrollTop = textarea.scrollHeight; // Automatically scroll to the bottom
            });
          })
          .catch(error => {
            console.error('Error fetching log messages:', error);
          });
    },
    startAutoRefresh() {
      console.log('Starting refresh...');
      if (this.autoRefresh) {
        this.refreshLogMessages(); // Load logs immediately when opened
        this.refreshLogsTimer = setInterval(this.refreshLogMessages, 15000); // Set interval for refreshing every 15 seconds
      }
    },
    stopAutoRefresh() {
      console.log('Stopping refresh...');
      clearInterval(this.refreshLogsTimer); // Stop the interval when accordion is closed
    }
  },
  mounted() {
    // Initial load of logs if the accordion is already open on page load
    const accordion = document.getElementById('accordion-content-logs');

    if (accordion) {
      // Manually bind the Bootstrap collapse events
      accordion.addEventListener('shown.bs.collapse', this.startAutoRefresh);
      accordion.addEventListener('hidden.bs.collapse', this.stopAutoRefresh);
    }

    // Check if accordion is already open on load
    if (accordion && accordion.classList.contains('show')) {
      this.startAutoRefresh();
    }
  },
  beforeDestroy() {
    // Clean up the event listeners when component is destroyed
    const accordion = document.getElementById('accordion-content-logs');
    if (accordion) {
      accordion.removeEventListener('shown.bs.collapse', this.startAutoRefresh);
      accordion.removeEventListener('hidden.bs.collapse', this.stopAutoRefresh);
    }

    this.stopAutoRefresh(); // Clear the interval when the component is destroyed
  }
};
</script>

<style scoped>
</style>