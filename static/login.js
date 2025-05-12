async function login(username, password) {
    const url = '/login'; // 🔁 Replace with your actual login endpoint

    const payload = {
        username: username,
        password: password
    };

    try {
        const response = await fetch(url, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(payload)
        });

        if (!response.ok) {
            // Handle HTTP errors
            throw new Error(`Login failed: ${response.status}`);
        }

        const data = await response.json();
        console.log('Login successful:', data);
        // Optionally store token: localStorage.setItem('token', data.token);
        return data;

    } catch (error) {
        console.error('Error during login:', error);
        return null;
    }
}