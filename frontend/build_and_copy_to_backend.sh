#!/bin/bash

# Build the frontend
npm run build
cp dist/index.html ../backend/data/frontend/index.html
cp dist/js/bundle.js ../backend/data/frontend/js/bundle.js