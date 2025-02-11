import socket
import locale

locale.setlocale(locale.LC_ALL, '') 
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 27000))

def _decode_frequence(data):
    s = ""
    data = data[5:10][::-1]
    for n in data:
        s += format(n,"x").rjust(2,"0")
    return f"{int(s):,}"

def _decode_transceiver(data):
    s = ""  
    match hex(data[3]):
        case "0x94": s = "IC-7300"
        case "0xA2": s = "IC-9700"
    return s

while True:
    data, address = sock.recvfrom(32)
    s = _decode_transceiver(data)
    s += " " + _decode_frequence(data)
    print(s)

