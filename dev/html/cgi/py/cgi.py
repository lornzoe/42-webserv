import os
import json
import sys

from urllib.parse import parse_qs

data_file_path = os.path.join(os.path.dirname(__file__), "data.json")

def save_data_to_json(data):
    with open(data_file_path, "w") as f:
        json.dump(data, f)

def read_data_from_json():
    if not os.path.exists(data_file_path):
        save_data_to_json([])

    with open(data_file_path, "r") as f:
        return json.load(f)

request_method = os.environ.get("REQUEST_METHOD")
json_data = read_data_from_json()

if request_method == "GET":
    sys.stdout.write("Content-Type: application/json\r\n")
    sys.stdout.write("\r\n")
    sys.stdout.write(json.dumps(json_data))

elif request_method == "POST":
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    new_data = sys.stdin.read(content_length)
    if not new_data:
        sys.stdout.write("Status: 400 Bad Request\r\n")
        sys.stdout.write("Content-Type: text/plain\r\n")
        sys.stdout.write("\r\n")
        sys.stdout.write("Name cannot be empty")
    elif new_data in json_data:
        sys.stdout.write("Status: 409 Conflict\r\n")
        sys.stdout.write("Content-Type: text/plain\r\n")
        sys.stdout.write("\r\n")
        sys.stdout.write("Duplicate name")
    else:
        json_data.append(new_data)
        save_data_to_json(json_data)

        sys.stdout.write("Status: 201 Created\r\n")
        sys.stdout.write("Content-Type: application/json\r\n")
        sys.stdout.write("\r\n")
        sys.stdout.write(json.dumps(json_data))

elif request_method == "DELETE":
    query_params = parse_qs(os.environ.get("QUERY_STRING", ""))
    data_to_delete = query_params.get("name", [""])[0]
    if data_to_delete not in json_data:
        sys.stdout.write("Status: 404 Not Found\r\n")
        sys.stdout.write("Content-Type: text/plain\r\n")
        sys.stdout.write("\r\n")
        sys.stdout.write("Name not found")
    else:
        json_data.remove(data_to_delete)
        save_data_to_json(json_data)
        sys.stdout.write("Status: 204 No Content\r\n")
        sys.stdout.write("\r\n")

else:
    sys.stdout.write("Status: 405 Method Not Allowed\r\n") 
    sys.stdout.write("Allow: GET, POST, DELETE\r\n") 
    sys.stdout.write("\r\n")
