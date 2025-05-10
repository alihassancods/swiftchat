const button = document.querySelector('#send');
const input = document.querySelector('#message-box');
const messageBox = document.querySelector('#chatbox');
const messageBoxContainer = document.querySelector('#chatbody');
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
        messageBox.innerHTML += `<div class="bg-[#f1f1f1] rounded-lg px-4 py-2 max-w-[40%] shadow">
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
    messageBox.innerHTML += `<div class="bg-[#dcf8c6] rounded-lg px-4 py-2 max-w-[40%] ml-auto shadow">
              <p class="text-sm">${message}</p>
              <span class="text-xs text-gray-500 block text-right">2:55 AM</span>
            </div>`;
    messageBoxContainer.scrollTop = messageBoxContainer.scrollHeight;
    input.value = "";
  }
