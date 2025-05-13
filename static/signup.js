const emailField = document.querySelector("#email");
const passwordField = document.querySelector("#password");
const signupButton = document.querySelector(".login-btn");
const warningSection = document.querySelector("#warning");
const errorSection = document.querySelector("#error");
const emailRegex = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$/;

function isValidEmail(email) {
  return emailRegex.test(email);
}
signupButton.addEventListener("click",()=>{
    if(!isValidEmail(emailField.value)){
        warningSection.textContent = "Invalid Email! Please enter correct one";
    }
    else{
        warningSection.textContent = "";
        data = JSON.dump(signup(emailField.textContent,passwordField.textContent));
        console.log(data);
        if(!data.status){
            errorSection.textContent = "Signup failed! Please Try again";
        }
        else{
            errorSection.textContent = "Signup completed!";

        }
    }
})
async function signup(email, password) {
    const url = 'http://127.0.0.1:18080/signup'; // 🔁 Replace with your actual login endpoint

    const payload = {
        email: email,
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
            throw new Error(`Signup failed: ${response.status}`);
        }

        const data = await response.json();
        console.log('Signups successful:', data);
        // Optionally store token:
        return data;

    } catch (error) {
        console.error('Error during login:', error);
        return null;
    }
}