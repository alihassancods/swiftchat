const button = document.querySelector('#send');
const input = document.querySelector('#message-box');
const messageBox = document.querySelector('#chatbox');
const messageBoxContainer = document.querySelector('#chatbody');
const translateButtons = document.querySelectorAll("#translate-button")
const username = "Ali";
const baseUrl = window.location.href.substring(0, window.location.href.lastIndexOf("/"));

var token;
const wb = new WebSocket('ws://localhost:18080/ws');
wb.addEventListener('open', () => {
    console.log('WebSocket connection opened');
    wb.send(JSON.stringify({"type":"init"}));
});
wb.addEventListener('message', (event) => {
    const data = JSON.parse(event.data);
    if (data.message != undefined) {
        messageBox.innerHTML += `
            <div class="message bg-[#f1f1f1] rounded-lg px-4 py-2 max-w-[40%] shadow">

              <p class="text-sm">${data.message}</p>
              <span class="text-xs text-gray-500 block text-right relative float-right">2:55 AM</span>
              <div class="text-xs text-blue-600 text-left">
              <a class="hover:underline translate-button">Translate</a></div>
            </div>
              `;
        messageBoxContainer.scrollTop = messageBoxContainer.scrollHeight;
    } else if (data.type == "token") {
        console.log('Tokens received:', data.token);
        token = data.token;
    }
});
input.addEventListener("keydown", (e) => {
    if (e.key == "Enter") {
        e.preventDefault();
        sendMessage(input.value,token);
        console.log('Message sent to server');
    }
  });

  function sendMessage(message,tokenValue){
    if(message == ""){
        return;
    }
    jsonSend = {"message": input.value,"token": tokenValue};
    wb.send(JSON.stringify(jsonSend));
    messageBox.innerHTML += `
            <div class=" message bg-[#dcf8c6] rounded-lg px-4 py-2 max-w-[40%] ml-auto shadow">
              <p class="text-sm">${message}</p>
              <span class="text-xs text-gray-500 block text-right relative float-right">2:55 AM</span>
              <div class="text-xs text-blue-600 text-left">
              <a class="hover:underline translate-button">Translate</a></div>
            </div>
              `;
    messageBoxContainer.scrollTop = messageBoxContainer.scrollHeight;
    input.value = "";
  }

  
document.addEventListener("click", function(event) {
    console.log("Clicked:", event.target);
  if (event.target.classList.contains("translate-button")) {
    const button = event.target;
    const messageContainer = button.closest(".message");
    const messageTextElem = messageContainer.querySelector(".text-sm");
    const originalText = messageTextElem.textContent;

    fetch(`${baseUrl}/translate?text=${originalText}`)
      .then(res => res.json())
      .then(data => {
        messageTextElem.textContent = data.text || "(Translation failed)";
      })
      .catch(err => {
        console.error("Fetch error:", err);
        messageTextElem.textContent = "(Translation error)";
      });
  }
});
