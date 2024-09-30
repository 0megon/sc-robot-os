from http.server import BaseHTTPRequestHandler, HTTPServer
import os
import time
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

html_file_path = "index.html"

class ServerHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/":
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            with open(html_file_path, 'r', encoding='utf-8') as file:
                html = file.read()
            self.wfile.write(html.encode('utf-8'))
        else:
            self.send_error(404, "Page Not Found {}".format(self.path))

def server_thread(port):
    server_address = ('', port)
    httpd = HTTPServer(server_address, ServerHandler)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()

class FileChangeHandler(FileSystemEventHandler):
    def on_modified(self, event):
        if event.src_path == os.path.abspath(html_file_path):
            print(f"{html_file_path} has been modified")

if __name__ == '__main__':
    port = 8000
    print("Starting server at port %d" % port)
    
    # Start the server in a separate thread
    import threading
    server_thread = threading.Thread(target=server_thread, args=(port,))
    server_thread.start()
    
    # Set up the watchdog observer
    event_handler = FileChangeHandler()
    observer = Observer()
    observer.schedule(event_handler, path=os.path.dirname(os.path.abspath(html_file_path)), recursive=False)
    observer.start()
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()
