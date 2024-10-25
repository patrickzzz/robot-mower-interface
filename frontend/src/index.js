import { createApp } from 'vue';
import App from './App.vue';
import 'bootstrap/dist/css/bootstrap.min.css';
import 'bootstrap'; // not needed, because we only need the CSS
import '@popperjs/core'; // not needed, because we only need the CSS

import './assets/app.css';

if (process.env.NODE_ENV === 'development') {
    import('./mockService')
        .then(() => {
            console.log('Mock service loaded');
        })
        .catch(err => {
            console.error('Error loading mock service:', err);
        });

    // add a small wait here..
    setTimeout(() => {
        console.log('Mock service loaded');
        createApp(App).mount('#app');
    }, 250);
}else{
    createApp(App).mount('#app');
}

