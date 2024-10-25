<template>
  <div class="position-fixed top-0 end-0 p-3" style="z-index: 11">
    <div v-if="show" class="toast fade show" :class="bgClass" role="alert" aria-live="assertive" aria-atomic="true">
      <div class="toast-body">
        {{ message }}
      </div>
    </div>
  </div>
</template>

<script>
export default {
  props: {
    show: {
      type: Boolean,
      required: true,
    },
    message: {
      type: String,
      required: true,
    },
    bgClass: {
      type: String,
      default: 'text-bg-primary',
    },
  },
  methods: {
    closeToast() {
      this.$emit('close');
    },
  },
};
</script>

<style scoped>
</style>