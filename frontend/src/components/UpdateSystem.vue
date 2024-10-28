<template>
  <div class="accordion mb-3">
    <div class="accordion-item">
      <h2 class="accordion-header">
        <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" aria-expanded="false" aria-controls="accordion-content-update" data-bs-target="#accordion-content-update">
          Update System
        </button>
      </h2>
      <div id="accordion-content-update" class="accordion-collapse collapse">
        <div class="accordion-body">
          <form @submit.prevent="uploadFiles">
            <p class="text-muted" style="font-size: 0.7em;">
              You can either upload a firmware.bin update, a filesystem.bin update, or both. The system will automatically restart after the update.
            </p>
            <div class="mb-3 text-start">
              <label for="firmwareUpload" class="form-label"><b>Firmware Update</b></label>
              <input type="file" class="form-control" id="firmwareUpload" @change="onFirmwareFileChange" />
            </div>
            <div class="mb-3 text-start">
              <label for="filesystemUpload" class="form-label"><b>Filesystem Update</b></label>
              <input type="file" class="form-control" id="filesystemUpload" @change="onFilesystemFileChange" />
            </div>
            <div class="text-end">
              <button type="submit" class="btn btn-outline-primary btn-sm" :disabled="isLoading">
                <span v-if="isLoading" class="spinner-border spinner-border-sm" aria-hidden="true"></span>
                Upload and update
              </button>
            </div>
          </form>

          <div v-if="uploadStatus" class="alert alert-info mt-3" role="alert">
            {{ uploadStatus }}
          </div>
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
      firmwareFile: null,
      filesystemFile: null,
      uploadStatus: '',
      isLoading: false,
    };
  },
  methods: {
    onFirmwareFileChange(event) {
      this.firmwareFile = event.target.files[0];
    },
    onFilesystemFileChange(event) {
      this.filesystemFile = event.target.files[0];
    },
    async uploadFiles() {
      if (!this.firmwareFile && !this.filesystemFile) {
        this.uploadStatus = 'Please choose at least one file.';
        return;
      }

      const formData = new FormData();
      if (this.firmwareFile) {
        formData.append("firmware", this.firmwareFile, "firmware.bin");
      }
      if (this.filesystemFile) {
        formData.append("filesystem", this.filesystemFile, "filesystem.bin");
      }

      this.isLoading = true;
      try {
        const response = await axios.post('/update', formData, {
          headers: {
            'Content-Type': 'multipart/form-data',
          },
        });

        this.uploadStatus = response.status === 200
            ? 'Upload successful! Rebooting...'
            : 'Upload failed.';
      } catch (error) {
        this.uploadStatus = "Error on update.";
        console.error("Upload-error:", error);
      } finally {
        this.isLoading = false;
        // reset form
        this.firmwareFile = null;
        this.filesystemFile = null;
        // reset form fields
        document.getElementById('firmwareUpload').value = '';
        document.getElementById('filesystemUpload').value = '';
      }
    }
  },
  mounted() {
  },
  beforeDestroy() {
  }
};
</script>

<style scoped>
</style>