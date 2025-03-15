// Comment form functionality
function initializeCommentForms() {
    initializeEditCommentButtons();
    initializeCancelEditButtons();
    initializeCommentSubmission();
}

// Handle edit comment button clicks
function initializeEditCommentButtons() {
    const editButtons = document.querySelectorAll('.btn-edit-comment');

    editButtons.forEach(button => {
        button.addEventListener('click', function() {
            const commentId = this.getAttribute('data-comment-id');
            const commentContent = document.getElementById('comment-' + commentId);
            const editForm = document.getElementById('edit-form-' + commentId);

            // Toggle visibility
            commentContent.classList.add('hidden');
            editForm.classList.remove('hidden');
        });
    });
}

// Handle cancel edit button clicks
function initializeCancelEditButtons() {
    const cancelButtons = document.querySelectorAll('.btn-cancel-edit');

    cancelButtons.forEach(button => {
        button.addEventListener('click', function() {
            const commentId = this.getAttribute('data-comment-id');
            const commentContent = document.getElementById('comment-' + commentId);
            const editForm = document.getElementById('edit-form-' + commentId);

            // Toggle visibility
            editForm.classList.add('hidden');
            commentContent.classList.remove('hidden');
        });
    });
}

// AJAX comment submission
function initializeCommentSubmission() {
    const commentForm = document.querySelector('.comment-form form');

    if (commentForm) {
        commentForm.addEventListener('submit', function(e) {
            e.preventDefault();

            const url = this.action;
            const contentInput = this.querySelector('textarea');
            const content = contentInput.value.trim();

            if (!content) {
                alert('Comment cannot be empty');
                return;
            }

            const formData = new FormData(this);

            fetch(url, {
                method: 'POST',
                headers: {
                    'X-CSRF-Token': getCsrfToken(),
                    'X-Requested-With': 'XMLHttpRequest',
                    'Accept': 'application/json'
                },
                body: formData,
                credentials: 'same-origin'
            })
                .then(response => response.json())
                .then(data => {
                    if (data.errors) {
                        alert('Error: ' + Object.values(data.errors).join(', '));
                        return;
                    }

                    handleNewCommentResponse(data, contentInput);
                })
                .catch(error => {
                    console.error('Error:', error);
                    alert('Something went wrong. Please try again later.');
                });
        });
    }
}

// Handle the response after posting a new comment
function handleNewCommentResponse(data, contentInput) {
    // Add new comment to the list
    const commentsList = document.querySelector('.comments-list');
    if (commentsList) {
        // Create a new comment element
        const newComment = document.createElement('div');
        newComment.innerHTML = data.html;

        // If no comments yet, remove the "no comments" message
        const noComments = document.querySelector('.no-comments');
        if (noComments) {
            noComments.remove();

            // Create comments list if it doesn't exist
            if (!commentsList) {
                const commentsSection = document.querySelector('.comments-section');
                const newCommentsList = document.createElement('div');
                newCommentsList.className = 'comments-list';
                commentsSection.insertBefore(newCommentsList, document.querySelector('.comment-form'));
                commentsSection.appendChild(newComment);
            }
        } else {
            // Add to existing list
            commentsList.appendChild(newComment);
        }

        // Update comment count
        const commentCount = document.querySelector('.comments-section h2');
        const currentCount = parseInt(commentCount.textContent.match(/\d+/)[0]) + 1;
        commentCount.textContent = `Comments (${currentCount})`;

        // Clear form
        contentInput.value = '';

        // Re-initialize event handlers for the new comment
        initializeLikeButtons();
        initializeCommentForms();
    }
}