// Flash message auto-hide
function initializeFlashMessages() {
    const flashMessages = document.querySelectorAll('.flash');

    flashMessages.forEach(flash => {
        // Add close button
        const closeBtn = document.createElement('button');
        closeBtn.className = 'flash-close';
        closeBtn.innerHTML = '&times;';
        closeBtn.addEventListener('click', function() {
            flash.remove();
        });
        flash.appendChild(closeBtn);

        // Auto-hide after 5 seconds
        setTimeout(() => {
            flash.classList.add('fade-out');
            setTimeout(() => {
                flash.remove();
            }, 500); // Match the CSS transition time
        }, 5000);
    });
}