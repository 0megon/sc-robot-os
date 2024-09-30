import asyncio
import websockets
from http.server import BaseHTTPRequestHandler, HTTPServer
import os
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
    
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length).decode('utf-8')
        command = post_data.split('=')[0]
        if command == 'fwd' or command == 'bwd' or command == 'rot':
            with open('cs\\program-calls.txt', 'a') as file:
                file.write(post_data)
        else:
            print('Error: unexpected data!')
   
        self.send_response(200)
        self.end_headers()


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
            loop.call_soon_threadsafe(asyncio.create_task, notify_clients())

clients = set()

async def notify_clients():
    if clients:
        message = "reload"
        await asyncio.gather(*[client.send(message) for client in clients])

async def websocket_handler(websocket, path):
    clients.add(websocket)
    try:
        async for message in websocket:
            pass
    finally:
        clients.remove(websocket)

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
    
    # Start the WebSocket server
    start_server = websockets.serve(websocket_handler, "localhost", 8765)
    loop = asyncio.get_event_loop()
    loop.run_until_complete(start_server)
    
    try:
        loop.run_forever()
    except KeyboardInterrupt:
        observer.stop()
    observer.join()
