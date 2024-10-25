<template>
  <div class="status mb-4">
    <template v-if="!status.isIdle">
      <span v-if="status.isCharging" class="badge rounded-pill text-bg-primary mx-1">In docking station</span>
      <span v-else class="badge rounded-pill text-bg-dark mx-1">Outside docking station</span>
      <span v-if="status.isEmergency" class="badge rounded-pill text-bg-danger mx-1">Emergency</span>
      <span v-if="status.isLocked" class="badge rounded-pill text-bg-warning mx-1">Locked</span>
    </template>
    <template v-else>
      <span class="badge rounded-pill text-bg-info mx-1">Idle</span>
    </template>
  </div>
</template>

<script>
export default {
  props: {
    status: {
      type: Object,
      required: true
    }
  }
};
</script>

<style scoped>
/* Optional: You can style the component here */
</style>