import socket

# 192.168.1.1
DOMAINS = {
    b"example.local": "10.5.0.100",
    b"mycoolsite.com": "10.0.0.5",
    # b"test.dev": "10.5.0.50"
    b"test.dev": "127.0.0.1"
}

def parse_domain_name(data, start_pos):
    """Parses the domain name from the DNS request query."""
    parts = []
    current = start_pos
    while True:
        length = data[current]
        if length == 0:
            current += 1
            break
        current += 1
        parts.append(data[current:current+length])
        current += length
    return b".".join(parts), current

def build_dns_response(data):
    """Constructs a valid DNS response packet matching the query."""
    # Transaction ID (First 2 bytes)
    transaction_id = data[:2]
    
    flags = b"\x81\x80"
    
    qdcount = data[4:6]
    ancount = b"\x00\x01"
    nscount = b"\x00\x00"
    arcount = b"\x00\x00"
    
    domain_name, query_end = parse_domain_name(data, 12)
    
    question_section = data[12:query_end + 4]
    
    if domain_name in DOMAINS:
        ip_address = DOMAINS[domain_name]
        
        answer_name = b"\xc0\x0c"
        
        answer_type_class = b"\x00\x01\x00\x01"
        
        ttl = b"\x00\x00\x00\x3c"
        
        data_len = b"\x00\x04"
        
        ip_bytes = bytes(map(int, ip_address.split('.')))
        
        answer_section = answer_name + answer_type_class + ttl + data_len + ip_bytes
    else:
        flags = b"\x81\x83" 
        ancount = b"\x00\x00"
        answer_section = b""

    return transaction_id + flags + qdcount + ancount + nscount + arcount + question_section + answer_section

def start_dns_server():
    """Binds to UDP port 53 and listens for incoming requests."""
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        # Port 53 is the standard DNS port
        server_socket.bind(("0.0.0.0", 53))
        print("DNS Server is running on port 53... Press Ctrl+C to stop.")
        
        while True:
            data, addr = server_socket.recvfrom(512)
            response = build_dns_response(data)
            server_socket.sendto(response, addr)
            
    except PermissionError:
        print("Error: Root/Administrator privileges are required to run on port 53.")
    except KeyboardInterrupt:
        print("\nShutting down DNS server.")
    finally:
        server_socket.close()

if __name__ == "__main__":
    start_dns_server()
