import urllib.request
import urllib.error

def make_request():
    url = "http://127.0.0.1:3030"
    print(f"Connecting to {url}...")
    
    try:
        with urllib.request.urlopen(url) as response:
            html_bytes = response.read()
            
            html_text = html_bytes.decode("utf-8")
            
            print("\n--- Response Received ---")
            print(html_text)
            
    except urllib.error.URLError as e:
        print(f"\nError: Could not connect to the server. Is it running?")
        print(f"Reason: {e.reason}")

if __name__ == "__main__":
    make_request()
