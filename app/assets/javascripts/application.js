// This is the main JavaScript file for the Cyclone application
// It loads all other JavaScript components

// = require csrf_protection
// = require_tree application

document.addEventListener('DOMContentLoaded', function() {
    // Initialize all components
    initializeDropdowns();
    initializeFlashMessages();
    initializeLikeButtons();
    initializeCommentForms();

    // Set up CSRF token for AJAX requests
    setupCSRFToken();
});