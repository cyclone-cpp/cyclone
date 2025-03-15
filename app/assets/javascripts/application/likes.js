// Like button AJAX functionality
function initializeLikeButtons() {
    const likeForms = document.querySelectorAll('.like-form');

    likeForms.forEach(form => {
        form.addEventListener('submit', function(e) {
            e.preventDefault();

            const isUnlike = form.querySelector('input[name="_method"]') !== null;
            const url = form.action;
            const button = form.querySelector('.btn-like');
            const countElement = form.parentElement.querySelector('.like-count');

            fetch(url, {
                method: isUnlike ? 'DELETE' : 'POST',
                headers: {
                    'X-CSRF-Token': getCsrfToken(),
                    'Accept': 'application/json',
                    'Content-Type': 'application/json'
                },
                credentials: 'same-origin'
            })
                .then(response => response.json())
                .then(data => {
                    if (data.error) {
                        alert(data.error);
                        return;
                    }

                    // Update like count
                    countElement.textContent = data.like_count + (data.like_count === 1 ? ' like' : ' likes');

                    // Toggle button state
                    if (isUnlike) {
                        button.textContent = 'Like';
                        button.classList.remove('liked');
                        form.innerHTML = button.outerHTML;
                    } else {
                        button.textContent = 'Unlike';
                        button.classList.add('liked');

                        // Add method override for unlike
                        const methodInput = document.createElement('input');
                        methodInput.type = 'hidden';
                        methodInput.name = '_method';
                        methodInput.value = 'DELETE';
                        form.insertBefore(methodInput, button);
                    }

                    // Re-initialize the form
                    initializeLikeButtons();
                })
                .catch(error => {
                    console.error('Error:', error);
                    alert('Something went wrong. Please try again later.');
                });
        });
    });
}