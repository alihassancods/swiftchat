const button = document.querySelector('#send');
const input = document.querySelector('#message-box');
const messageBox = document.querySelector('#chatbox');
const messageBoxContainer = document.querySelector('#chatbody');
const translateButtons = document.querySelectorAll("#translate-button")
const username = "Ali";
var token;
const wb = new WebSocket('ws://localhost:18080/ws');
wb.addEventListener('open', () => {
    console.log('WebSocket connection opened');
    wb.send(JSON.stringify({"type":"init"}));
});
wb.addEventListener('message', (event) => {
    const data = JSON.parse(event.data);
    if (data.message != undefined) {
        messageBox.innerHTML += `<div class="message bg-[#f1f1f1] rounded-lg px-4 py-2 max-w-[40%] shadow">
              <p class="text-sm">${data.message}</p>
              <span class="text-xs text-gray-500 block text-right">2:55 AM</span>
            </div>`;
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
    messageBox.innerHTML += `<div class=" message bg-[#dcf8c6] rounded-lg px-4 py-2 max-w-[40%] ml-auto shadow">
              <p class="text-sm">${message}</p>
              <span class="text-xs text-gray-500 block text-right">2:55 AM</span>
            </div>`;
    messageBoxContainer.scrollTop = messageBoxContainer.scrollHeight;
    input.value = "";
  }
   translateButtons.forEach(button => {
    button.addEventListener("click", () => {
      const container = button.closest(".message");
      const messageTextElem = container.querySelector(".text-sm");
      const originalText = messageTextElem.textContent;
      console.log(originalText);
      messageTextElem.textContent = translate(originalText);
    });
});
  
  async function translate(textToTranslate){
      try {
        const response = await fetch(`http://127.0.0.1:18080/translate?text=${textToTranslate}`);

        if (!response.ok) {
          throw new Error(`Server responded with status ${response.status}`);
        }

        return JSON.stringify(data, null, 2);
      } catch (error) {
        return `Error: ${error.message}`;
      }
    }