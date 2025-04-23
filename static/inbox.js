const button = document.querySelector('#send');
const input = document.querySelector('#message');
const wb = new WebSocket('ws://localhost:18080/ws');
wb.addEventListener('open', () => {
    console.log('WebSocket connection opened');
});
button.addEventListener('click', () => {
    jsonSend = {"message": input.value,"type":"message"};
    wb.send(JSON.stringify(jsonSend));
    // wb.send(input.value);
    console.log('Message sent to server');
})