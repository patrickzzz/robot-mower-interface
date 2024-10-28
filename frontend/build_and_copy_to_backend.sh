#!/bin/bash

# Build the frontend
npm run build

# Gzip the frontend files
gzip -k -f dist/index.html
gzip -k -f dist/js/bundle.js

# Copy the gzipped files to the backend
mv dist/index.html.gz ../backend/data/frontend/index.html.gz
mv dist/js/bundle.js.gz ../backend/data/frontend/js/bundle.js.gz

cp version.json ../backend/data/frontend/version.json