// CSRF token setup for AJAX requests
function setupCSRFToken() {
    // Set up CSRF token for all AJAX requests
    const token = getCsrfToken();

    if (token) {
        document.addEventListener('ajax:beforeSend', function(e) {
            const xhr = e.detail[0];
            xhr.setRequestHeader('X-CSRF-Token', token);
        });
    }
}

// Get CSRF token from meta tag
function getCsrfToken() {
    const metaTag = document.querySelector('meta[name="csrf-token"]');
    return metaTag ? metaTag.getAttribute('content') : '';
}