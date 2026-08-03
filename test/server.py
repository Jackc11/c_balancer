import threading
from http.server import HTTPServer, BaseHTTPRequestHandler

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
    def _send_headers(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html; charset=utf-8")
        if "Connection" in self.headers and self.headers["Connection"] == "close":
            self.send_header("Connection", "close")
        self.end_headers()

    def do_GET(self):
        self._send_headers()
        
        response_text = f"<h1>Hello from Python Server on Port {self.server.server_port}!</h1>"
        self.wfile.write(response_text.encode("utf-8"))

    def do_HEAD(self):
        xf_host = self.headers.get("X-Forwarded-Host", "None")
        print(f"[{self.server.server_port}] HEAD-Request erhalten! X-Forwarded-Host: {xf_host}")
        
        self._send_headers()

def run_server(port: int) -> None:
    server_address = ("127.0.0.1", port)
    httpd = HTTPServer(server_address, SimpleHTTPRequestHandler)
    print(f"Server is listening on http://127.0.0.1:{port}")
    try:
        httpd.serve_forever()
    except Exception:
        httpd.server_close()

if __name__ == "__main__":
    ports = [8080, 8081, 8082]
    threads = []

    # Starte jeden HTTP-Server in seinem eigenen Thread
    for port in ports:
        t = threading.Thread(target=run_server, args=(port,), daemon=True)
        t.start()
        threads.append(t)

    print("Alle 3 Backend-Server wurden parallel gestartet.")
    
    # Halte den Haupt-Thread am Leben, bis STRG+C gedrückt wird
    try:
        for t in threads:
            t.join()
    except KeyboardInterrupt:
        print("\nStoppe alle Server...")
