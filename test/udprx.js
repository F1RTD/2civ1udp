var udp = require('dgram');
var server = udp.createSocket('udp4');

function decode_frequency(data) {
    data = data.slice(5, 10).reverse();
    let s = "";
    for (let i = 0; i < data.length; i++) {
        s += data[i].toString(16).padStart(2, '0');
    }
    return parseInt(s).toLocaleString('de-DE');;
}   

function decode_transceiver(data) {
    let s = ""
    switch (data[3]) {
        case 0x94: s = "IC-7300"; break;
        case 0xA2: s = "IC-9700"; break;
    }
    return s;
}

server.on('message', function (data) {
    let s = decode_transceiver(data);
    switch (data[4]) {
        case 0:
            s += " " + decode_frequency(data);
            break;
    }
    console.log(s);
});

server.bind({ address: '0.0.0.0', port: 27000 });